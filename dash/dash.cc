// Copyright 2021, Beeri 15.  All rights reserved.
// Author: Roman Gershman (romange@gmail.com)
//

#include "dash.h"

#include <immintrin.h>

#include <cstring>
#include "aquahash.h"

namespace base {

namespace {

inline unsigned SSE_CMP8(const void* src, uint8_t key) {
  // Replicate 16 times uint8_t key to key_data.
  const __m128i key_data = _mm_set1_epi8(key);

  // Loads 16 bytes of src into seg_data.
  __m128i seg_data = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src));

  // compare 16-byte vectors seg_data and key_data, dst[i] := ( a[i] == b[i] ) ? 0xFF : 0.
  __m128i rv_mask = _mm_cmpeq_epi8(seg_data, key_data);

  // collapses 16 msb bits from each byte in rv_mask into mask.
  int mask = _mm_movemask_epi8(rv_mask);

  // Note: Last 2 operations can be combined in skylake with _mm_cmpeq_epi8_mask.
  return mask;
}

uint64_t HashFun(Segment::Key_t k) {
  // return XXH3_64bits_withSeed(&k, sizeof(k), 0);
  auto hash = AquaHash::SmallKeyAlgorithm(reinterpret_cast<const uint8_t*>(&k), sizeof(k));
  return _mm_cvtsi128_si64x(hash);
}

}  // namespace

BucketBase::BucketBase() {
}

bool BucketBase::ClearStash(uint8_t fp, unsigned stash_pos, bool is_own) {
  auto cb = [stash_pos, this](unsigned i, unsigned pos) -> SlotId {
    if (pos == stash_pos) {
      overflow_bitmap_ &= (~(1u << i));
      overflow_member_ &= (~(1u << i));
      overflow_pos_ &= (~(3u << (i * 2)));

      return 0;
    }
    return kNanSlot;
  };

  auto res = IterateStash(fp, is_own, std::move(cb));
  return res.second != kNanSlot;
}


auto Bucket::Find(Key_t key, comp_fun cf, uint8_t meta_hash, bool probe) const -> SlotId {
  unsigned mask = SSE_CMP8(finger_array_, meta_hash) & GetBusy();
  mask &= (GetProbe() ^ ((1u - probe) * kAllocMask));

  unsigned delta = 0;
  bool eq;
  while (mask) {
    int index = __builtin_ctz(mask);
    if (cf) {
      eq = cf(key_[delta + index], key);
    } else {
      eq = (key_[delta + index] == key);
    }

    if (eq) {
      return delta + index;
    }
    mask >>= (index + 1);
    delta += (index + 1);
  }

  return kNanSlot;
}

Segment::~Segment() {
}

void Segment::SetStashPtr(unsigned stash_pos, uint8_t meta_hash, Bucket* target, Bucket* next) {

  unsigned index = target->FindOverflowSlot();  // index finds the rightmost free slot.

  // we use only 4 fp slots for handling stash buckets,
  // therefore if all those slots are used we try neighbor (probing bucket) as a fallback to point
  // to stash buckets. otherwise we increment overflow. if overflow is incremented we need to check
  // all the stash buckets. otherwise we can use overflow_index_ to find the the stash bucket
  // effectively.
  if (index < 4) {
    target->SetOverflow(meta_hash, index, stash_pos);
  } else {
    index = next->FindOverflowSlot();
    if (index < 4) {
      next->SetOverflow(meta_hash, index, stash_pos);
      // overflow_member_ specifies which records relate to other bucket.
      next->overflow_member_ |= (1 << index);
    } else { /*overflow, increase count*/
      target->overflow_count_++;
    }
  }
  target->overflow_bitmap_ |= Bucket::kOverflowBit;
}

// stash_pos is index of the stash bucket, it is in the range of [0, kNumStashBucket].
void Segment::UnsetStashPtr(unsigned stash_pos, uint8_t meta_hash, Bucket* target, Bucket* next) {
  /*also needs to ensure that this meta_hash must belongs to other bucket*/
  bool clear_success = target->ClearStash(meta_hash, stash_pos, true);

  if (!clear_success) {
    clear_success = next->ClearStash(meta_hash, stash_pos, false);
  }

  if (!clear_success) {

    target->overflow_count_--;
  }

  // kOverflowBit helps with summarizing all the overflow states into a single binary flag.
  // we need it because of the next, though if we make sure to move stash pointers upon split/delete
  // towards the owner we should not reach the state where mask1 == 0 but mask2 &
  // next->overflow_member_ != 0.
  unsigned mask1 = target->overflow_bitmap_ & Bucket::kOverflowMask;
  unsigned mask2 = next->overflow_bitmap_ & Bucket::kOverflowMask;
  if (((mask1 & (~target->overflow_member_)) == 0) && (target->overflow_count_ == 0) &&
      ((mask2 & next->overflow_member_) == 0)) {
    target->overflow_bitmap_ &= ~Bucket::kOverflowBit;
  }
}

int Segment::Insert(Key_t key, Value_t value, size_t key_hash, Bucket::comp_fun cmp_fun) {
  /*unique check, needs to check 2 hash table*/
  Iterator it = Find(key, key_hash, cmp_fun);
  if (it.found()) {
    return -3; /* duplicate insert*/
  }

  bool res = InsertUniq(key, value, key_hash);

  if (res) {
    ++size_;
    return 0;
  }

  return -1;
}

auto Segment::Find(Key_t key, size_t key_hash, Bucket::comp_fun cf) const -> Iterator {
  unsigned bidx = BucketIndex(key_hash);
  const Bucket* target = bucket_ + bidx;
  uint8_t fp_hash = key_hash & kFpMask;

  Bucket::SlotId sid = target->Find(key, cf, fp_hash, false);
  if (sid != Bucket::kNanSlot) {
    return Iterator{bidx, sid};
  }
  uint8_t nid = (bidx + 1) & kBucketMask;
  const Bucket* probe = bucket_ + nid;

  sid = probe->Find(key, cf, fp_hash, true);
  if (sid != Bucket::kNanSlot) {
    return Iterator{nid, sid};
  }

  if (!target->HasStash()) {
    return Iterator{};
  }

  auto stash_cb = [&](unsigned overflow_index, unsigned pos) -> Bucket::SlotId {


    const Bucket* bucket = bucket_ + kNumBucket + pos;
    return bucket->Find(key, cf, fp_hash, false);
  };

  if (target->HasStashOverflow()) {
    for (unsigned i = 0; i < kNumStashBucket; ++i) {
      auto sid = stash_cb(0, i);
      if (sid != Bucket::kNanSlot) {
        return Iterator{kNumBucket + i, sid};
      }
    }

    return Iterator{};
  }

  auto stash_res = target->IterateStash(fp_hash, true, stash_cb);
  if (stash_res.second != Bucket::kNanSlot) {
    return Iterator{kNumBucket + stash_res.first, stash_res.second};
  }

  stash_res = probe->IterateStash(fp_hash, false, stash_cb);
  if (stash_res.second != Bucket::kNanSlot) {
    return Iterator{kNumBucket + stash_res.first, stash_res.second};
  }
  return Iterator{};
}

void Segment::DeleteKeys(DtorFn dfun) {
  for (unsigned i = 0; i < kNumBucket + kNumStashBucket; ++i) {
    bucket_[i].ForEach([&](unsigned, Key_t k, Value_t, bool) { dfun(k); });
  }
}

bool Segment::Delete(Key_t key, size_t key_hash, Bucket::comp_fun cf, DtorFn dfun) {
  auto it = Find(key, key_hash, cf);

  if (!it.found())
    return false;

  if (it.index >= kNumBucket) {
    unsigned y = BucketIndex(key_hash);
    auto* owner = bucket_ + y;
    auto* probe = bucket_ + ((y + 1) & kBucketMask);
    UnsetStashPtr(it.index - kNumBucket, key_hash & kFpMask, owner, probe);
  }
  if (dfun) {
    dfun(bucket_[it.index].value(it.slot));
  }
  bucket_[it.index].ClearHash(it.slot);

  return true;
}

bool Segment::Find(Key_t key, size_t key_hash, Bucket::comp_fun cf, Value_t* res) const {
  auto it = Find(key, key_hash, cf);
  if (!it.found())
    return false;

  const Bucket& b = bucket_[it.index];
  *res = b.value(it.slot);
  return true;
}

void Segment::Split(Segment* dest) {
  ++local_depth_;
  dest->local_depth_ = local_depth_;
  unsigned removed = 0;

  for (unsigned i = 0; i < kNumBucket; ++i) {
    uint32_t invalid_mask = 0;

    auto cb = [&](unsigned slot, Key_t k, Value_t v, bool probe) {
      uint64_t hash = HashFun(k);

      // we extract local_depth bits from the left part of the hash. Since we extended local_depth,
      // we added an additional bit to the right, therefore we need to look at lsb of the extract.
      if ((hash >> (64 - local_depth_) & 1) == 0)
        return;  // keep this key in the source

      Bucket* dest_bucket = dest->bucket_ + i;
      invalid_mask |= (1u << slot);
      ++removed;

      // I copy items directly to the equivalent bucket in next_table.
      // Note: This approach may destory the balanced segment and make future insertions
      // slower.
      // We know that we insert new items and that the new bucket must have place for them because
      // the old one has.

      dest_bucket->InsertUniq(k, v, hash & kFpMask, slot, probe);
      dest->size_++;
    };

    Bucket& bucket = bucket_[i];
    bucket.ForEach(std::move(cb));
    bucket.ClearSlots(invalid_mask);
  }

  for (unsigned i = 0; i < kNumStashBucket; ++i) {
    Bucket& bucket = bucket_[kNumBucket + i];
    uint32_t invalid_mask = 0;

    auto cb = [&](unsigned slot, Key_t key, Value_t v, bool probe) {
      uint64_t hash = HashFun(key);
      if ((hash >> (64 - local_depth_) & 1) == 0)
        return;

      invalid_mask |= (1u << slot);

      dest->InsertUniq(key, v, hash);
      dest->size_++;
      ++removed;

      // Remove stash pointer referencing to this bucket.
      auto bucket_ix = BucketIndex(hash);
      auto* owner = bucket_ + bucket_ix;
      auto* neighbor_bucket = bucket_ + ((bucket_ix + 1) & kBucketMask);
      UnsetStashPtr(i, hash & kFpMask, owner, neighbor_bucket);
    };
    bucket.ForEach(std::move(cb));
    bucket.ClearSlots(invalid_mask);
  }


  size_ -= removed;
}

bool Segment::InsertUniq(Key_t key, Value_t value, size_t key_hash) {
  auto y = BucketIndex(key_hash);
  Bucket* target = bucket_ + y;
  Bucket* neighbor = bucket_ + ((y + 1) & kBucketMask);
  Bucket* insert_target = target;
  uint8_t meta_hash = key_hash & kFpMask;
  bool probe = false;

  if (target->Size() > neighbor->Size()) {
    insert_target = neighbor;
    probe = true;
  }

  if (!insert_target->IsFull()) {
    unsigned slot = insert_target->GetEmptySlot();
    insert_target->InsertUniq(key, value, meta_hash, slot, probe);

    return true;
  }

  Bucket* next_neighbor = bucket_ + ((y + 2) & kBucketMask);
  int displace_index = neighbor->MoveToOther(true, next_neighbor);
  if (displace_index >= 0) {
    neighbor->InsertUniq(key, value, meta_hash, displace_index, true);
    return true;
  }

  Bucket* prev_neighbor = (y == 0) ? bucket_ + kNumBucket - 1 : bucket_ + y - 1;
  displace_index = target->MoveToOther(false, prev_neighbor);
  if (displace_index >= 0) {
    target->InsertUniq(key, value, meta_hash, displace_index, false);
    return true;
  }

  return StashInsertUniq(target, neighbor, key, value, meta_hash, y & kStashMask);
}

DashTable::DashTable(size_t capacity_log) : global_depth_(capacity_log) {
  segment_.resize(1 << global_depth_);
  for (auto& ptr : segment_) {
    ptr = new Segment(global_depth_);
  }
  unique_segments_ = segment_.size();
}

DashTable::~DashTable() {
  size_t i = 0;
  while (i < segment_.size()) {
    Segment* s = segment_[i];
    size_t delta = (1u << (global_depth_ - s->local_depth_));

    if (key_dtor_fun_) {
      s->DeleteKeys(key_dtor_fun_);
    }
    delete s;
    i += delta;
  }
}

void DashTable::Reserve(size_t size) {
  size_t sg_cnt = 1 + size / Segment::capacity();
  if (sg_cnt < segment_.size()) {
    return;
  }
  unsigned new_depth = 1 + (63 ^ __builtin_clzll(sg_cnt - 1));

  IncreaseDepth(new_depth);
  // TBD: to create and split segments.
}

int DashTable::Insert(Key_t key, Value_t value) {
  uint64_t key_hash = HashFun(key);

  while (true) {
    // Keep last global_depth_ msb bits of the hash.
    size_t x = SegmentId(key_hash);

    Segment* target = segment_[x];

    int ret = target->Insert(key, value, key_hash, cmp_fun_);

    if (ret == -3) { /*duplicate insert, insertion failure*/
      return -1;
    }

    if (ret == 0) {
      ++size_;
      return 0;
    }

    if (target->local_depth_ == global_depth_) {
      IncreaseDepth(global_depth_ + 1);

      x = SegmentId(key_hash);

    }

    size_t chunk_size = 1u << (global_depth_ - target->local_depth_);
    size_t start_idx = x & (~(chunk_size - 1));

    Segment* s = new Segment(target->local_depth_ + 1);
    target->Split(s);  // increases the depth.
    ++unique_segments_;

    for (size_t i = start_idx + chunk_size / 2; i < start_idx + chunk_size; ++i) {
      segment_[i] = s;
    }
  }

  return 0;
}

void DashTable::IncreaseDepth(unsigned new_depth) {

  size_t prev_sz = segment_.size();
  size_t repl_cnt = 1ul << (new_depth - global_depth_);
  segment_.resize(1ul << new_depth);

  for (int i = prev_sz - 1; i >= 0; --i) {
    size_t offs = i * repl_cnt;
    std::fill(segment_.begin() + offs, segment_.begin() + offs + repl_cnt, segment_[i]);
  }
  global_depth_ = new_depth;
}

size_t DashTable::DefaultHash(Key_t k) {
  return HashFun(k);
}

}  // namespace base
