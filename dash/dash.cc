// Copyright 2021, Beeri 15.  All rights reserved.
// Author: Roman Gershman (romange@gmail.com)
//

#include "dash.h"


#include <cstring>


#include "aquahash.h"

#define DCHECK_EQ(x, y)
#define DCHECK(x)
#define  DCHECK_LT(x, y)
#define  DCHECK_LE(x, y)
#define  DCHECK_GT(x, y)

namespace base {

namespace {

uint64_t HashFun(DashTable::Key_t k) {
  auto hash = AquaHash::SmallKeyAlgorithm(reinterpret_cast<const uint8_t*>(&k), sizeof(k));
  return _mm_cvtsi128_si64x(hash);
}

}  // namespace

namespace detail {

BucketBase::BucketBase() {
}

bool BucketBase::ClearStash(uint8_t fp, unsigned stash_pos, bool is_own) {
  auto cb = [stash_pos, this](unsigned i, unsigned pos) -> SlotId {
    if (pos == stash_pos) {
      overflow_bitmap_ &= (~(1u << i));
      overflow_member_ &= (~(1u << i));
      overflow_pos_ &= (~(3u << (i * 2)));

      DCHECK_EQ(0u, ((overflow_pos_ >> (i * 2)) & Segment::kStashMask));
      return 0;
    }
    return kNanSlot;
  };

  auto res = IterateStash(fp, is_own, std::move(cb));
  return res.second != kNanSlot;
}

Segment::~Segment() {
}

void Segment::SetStashPtr(unsigned stash_pos, uint8_t meta_hash, BucketBase* target,
                          BucketBase* next) {
  DCHECK_LT(stash_pos, kNumStashBucket);

  unsigned index = target->FindOverflowSlot();  // index finds the rightmost free slot.

  // we use only kOverflowLen fp slots for handling stash buckets,
  // therefore if all those slots are used we try neighbor (probing bucket) as a fallback to point
  // to stash buckets. otherwise we increment overflow. if overflow is incremented we need to check
  // all the stash buckets. otherwise we can use overflow_index_ to find the the stash bucket
  // effectively.
  if (index < BucketBase::kOverflowLen) {
    target->SetOverflow(meta_hash, index, stash_pos);
  } else {
    index = next->FindOverflowSlot();
    if (index < BucketBase::kOverflowLen) {
      next->SetOverflow(meta_hash, index, stash_pos);
      // overflow_member_ specifies which records relate to other bucket.
      next->overflow_member_ |= (1 << index);
    } else { /*overflow, increase count*/
      target->overflow_count_++;
    }
  }
  target->overflow_bitmap_ |= BucketBase::kOverflowBit;
}

// stash_pos is index of the stash bucket, it is in the range of [0, kNumStashBucket].
void Segment::UnsetStashPtr(unsigned stash_pos, uint8_t meta_hash, BucketBase* target,
                            BucketBase* next) {
  /*also needs to ensure that this meta_hash must belongs to other bucket*/
  bool clear_success = target->ClearStash(meta_hash, stash_pos, true);

  if (!clear_success) {
    clear_success = next->ClearStash(meta_hash, stash_pos, false);
  }

  if (!clear_success) {
    DCHECK_GT(target->overflow_count_, 0);
    target->overflow_count_--;
  }

  // kOverflowBit helps with summarizing all the overflow states into a single binary flag.
  // we need it because of the next, though if we make sure to move stash pointers upon split/delete
  // towards the owner we should not reach the state where mask1 == 0 but mask2 &
  // next->overflow_member_ != 0.
  unsigned mask1 = target->overflow_bitmap_ & BucketBase::kOverflowMask;
  unsigned mask2 = next->overflow_bitmap_ & BucketBase::kOverflowMask;
  if (((mask1 & (~target->overflow_member_)) == 0) && (target->overflow_count_ == 0) &&
      ((mask2 & next->overflow_member_) == 0)) {
    target->overflow_bitmap_ &= ~BucketBase::kOverflowBit;
  }
}

int Segment::Insert(Key_t key, Value_t value, size_t key_hash, comp_fun cmp_fun) {
  /*unique check, needs to check 2 hash table*/
  Iterator it = FindIt(key, key_hash, cmp_fun);
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

auto Segment::FindIt(Key_t key, size_t key_hash, comp_fun cf) const -> Iterator {
  unsigned bidx = BucketIndex(key_hash);
  const BucketBase& target = bucket_[bidx].b;

  uint8_t fp_hash = key_hash & kFpMask;
  unsigned mask = target.Find(fp_hash, false);
  BucketBase::SlotId sid = BucketBase::kNanSlot;
  if (mask) {
    if (cf) {
      sid = FindByMask(bidx, key, mask, cf);
    } else {
      sid = FindByMask(bidx, key, mask, [](auto k1, auto k2) { return k1 == k2; });
    }

    if (sid != BucketBase::kNanSlot) {
      return Iterator{bidx, sid};
    }
  }

  uint8_t nid = (bidx + 1) & kBucketMask;
  const BucketBase& probe = bucket_[nid].b;
  mask = probe.Find(fp_hash, true);
  if (mask) {
    sid = cf ? FindByMask(nid, key, mask, cf)
             : FindByMask(nid, key, mask, [](auto k1, auto k2) { return k1 == k2; });
    if (sid != BucketBase::kNanSlot) {
      return Iterator{nid, sid};
    }
  }

  if (!target.HasStash()) {
    return Iterator{};
  }

  auto stash_cb = [&](unsigned overflow_index, unsigned pos) -> BucketBase::SlotId {
    DCHECK_LT(pos, kNumStashBucket);

    pos += kNumBucket;
    const BucketBase& bucket = bucket_[pos].b;
    unsigned mask = bucket.Find(fp_hash, false);
    if (!mask)
      return BucketBase::kNanSlot;

    return cf ? FindByMask(pos, key, mask, cf)
              : FindByMask(pos, key, mask, [](auto k1, auto k2) { return k1 == k2; });
  };

  if (target.HasStashOverflow()) {
    for (unsigned i = 0; i < kNumStashBucket; ++i) {
      auto sid = stash_cb(0, i);
      if (sid != BucketBase::kNanSlot) {
        return Iterator{kNumBucket + i, sid};
      }
    }

    return Iterator{};
  }

  auto stash_res = target.IterateStash(fp_hash, true, stash_cb);
  if (stash_res.second != BucketBase::kNanSlot) {
    return Iterator{kNumBucket + stash_res.first, stash_res.second};
  }

  stash_res = probe.IterateStash(fp_hash, false, stash_cb);
  if (stash_res.second != BucketBase::kNanSlot) {
    return Iterator{kNumBucket + stash_res.first, stash_res.second};
  }
  return Iterator{};
}

void Segment::DeleteKeys(DtorFn dfun) {
  for (unsigned i = 0; i < kNumBucket + kNumStashBucket; ++i) {
    ForEach(i, [&](unsigned, Key_t k, Value_t, bool) { dfun(k); });
  }
}

bool Segment::Delete(Key_t k, size_t key_hash, comp_fun cf, DtorFn dfun) {
  auto it = FindIt(k, key_hash, cf);

  if (!it.found())
    return false;

  if (it.index >= kNumBucket) {
    unsigned y = BucketIndex(key_hash);
    auto* owner = &bucket_[y].b;
    auto* probe = &bucket_[(y + 1) & kBucketMask].b;
    UnsetStashPtr(it.index - kNumBucket, key_hash & kFpMask, owner, probe);
  }

  if (dfun) {
    dfun(key(it.index, it.slot));
  }
  bucket_[it.index].b.ClearHash(it.slot);

  return true;
}

bool Segment::Find(Key_t key, size_t key_hash, comp_fun cf, Value_t* res) const {
  auto it = FindIt(key, key_hash, cf);
  if (!it.found())
    return false;

  *res = value(it.index, it.slot);

  return true;
}

void Segment::Split(Segment::HashFn hfn, Segment* dest) {
  ++local_depth_;
  dest->local_depth_ = local_depth_;
  unsigned removed = 0;

  if (!hfn)
    hfn = &HashFun;

  for (unsigned i = 0; i < kNumBucket; ++i) {
    uint32_t invalid_mask = 0;

    auto cb = [&](unsigned slot, Key_t k, Value_t v, bool probe) {
      uint64_t hash = hfn(k);

      // we extract local_depth bits from the left part of the hash. Since we extended local_depth,
      // we added an additional bit to the right, therefore we need to look at lsb of the extract.
      if ((hash >> (64 - local_depth_) & 1) == 0)
        return;  // keep this key in the source

      // Bucket* dest_bucket = dest->bucket_ + i;
      invalid_mask |= (1u << slot);
      ++removed;

      // I copy items directly to the equivalent bucket in next_table.
      // Note: This approach may destory the balanced segment and make future insertions
      // slower.
      // We know that we insert new items and that the new bucket must have place for them because
      // the old one has.
      // DCHECK(!dest_bucket->IsFull());
      dest->InsertToBucket(Iterator{i, slot}, k, v, hash & kFpMask, probe);

      // dest_bucket->InsertUniq(k, v, hash & kFpMask, slot, probe);
      // CHECK(dest->InsertUniq(k, v, hash));

      dest->size_++;
    };

    // Bucket& bucket = bucket_[i];
    ForEach(i, std::move(cb));
    bucket_[i].b.ClearSlots(invalid_mask);
  }

  for (unsigned i = 0; i < kNumStashBucket; ++i) {
    BucketBase& bucket = bucket_[kNumBucket + i].b;
    uint32_t invalid_mask = 0;

    auto cb = [&](unsigned slot, Key_t key, Value_t v, bool probe) {
      uint64_t hash = hfn(key);
      if ((hash >> (64 - local_depth_) & 1) == 0)
        return;

      invalid_mask |= (1u << slot);

      dest->InsertUniq(key, v, hash);

      dest->size_++;
      ++removed;

      // Remove stash pointer referencing to this bucket.
      auto bucket_ix = BucketIndex(hash);
      auto* owner = &bucket_[bucket_ix].b;
      auto* neighbor_bucket = &bucket_[(bucket_ix + 1) & kBucketMask].b;
      UnsetStashPtr(i, hash & kFpMask, owner, neighbor_bucket);
    };
    ForEach(kNumBucket + i, std::move(cb));
    bucket.ClearSlots(invalid_mask);
  }

  DCHECK_LE(removed, size_);
  size_ -= removed;
}

bool Segment::InsertUniq(Key_t key, Value_t value, size_t key_hash) {
  unsigned y = BucketIndex(key_hash);
  unsigned neighbor_idx = (y + 1) & kBucketMask;
  BucketBase* target = &bucket_[y].b;
  BucketBase* neighbor = &bucket_[neighbor_idx].b;
  BucketBase* insert_target = target;
  unsigned target_idx = y;

  uint8_t meta_hash = key_hash & kFpMask;
  bool probe = false;

  if (target->Size() > neighbor->Size()) {
    insert_target = neighbor;
    target_idx = neighbor_idx;
    probe = true;
  }

  if (!insert_target->IsFull()) {
    unsigned slot = insert_target->GetEmptySlot();
    InsertToBucket(Iterator{target_idx, slot}, key, value, meta_hash, probe);

    return true;
  }

  int displace_index = MoveToOther(true, neighbor_idx, (y + 2) & kBucketMask);
  if (displace_index >= 0) {
    InsertToBucket(Iterator{neighbor_idx, unsigned(displace_index)}, key, value, meta_hash, true);
    return true;
  }

  unsigned prev_idx = (y == 0) ? kNumBucket - 1 : y - 1;
  displace_index = MoveToOther(false, y, prev_idx);
  if (displace_index >= 0) {
    InsertToBucket(Iterator{y, unsigned(displace_index)}, key, value, meta_hash, false);
    return true;
  }

  return StashInsertUniq(target, neighbor, key, value, meta_hash, y & kStashMask);
}

}  // namespace detail

DashTable::DashTable(size_t capacity_log) : global_depth_(capacity_log) {
  segment_.resize(1 << global_depth_);
  for (auto& ptr : segment_) {
    ptr = new detail::Segment(global_depth_);
  }
  unique_segments_ = segment_.size();
}

DashTable::~DashTable() {
  size_t i = 0;
  while (i < segment_.size()) {
    auto* seg = segment_[i];
    size_t delta = (1u << (global_depth_ - seg->local_depth()));

    if (key_dtor_fun_) {
      seg->DeleteKeys(key_dtor_fun_);
    }
    delete seg;
    i += delta;
  }
}

void DashTable::Reserve(size_t size) {
  size_t sg_cnt = 1 + size / detail::Segment::capacity();
  if (sg_cnt < segment_.size()) {
    return;
  }
  unsigned new_depth = 1 + (63 ^ __builtin_clzll(sg_cnt - 1));

  IncreaseDepth(new_depth);
  // TBD: to create and split segments.
}

bool DashTable::Insert(Key_t key, Value_t value) {
  uint64_t key_hash = Hash(key);

  while (true) {
    // Keep last global_depth_ msb bits of the hash.
    size_t x = SegmentId(key_hash);
    DCHECK_LT(x, segment_.size());
    auto* target = segment_[x];

    int ret = target->Insert(key, value, key_hash, cmp_fun_);

    if (ret == -3) { /*duplicate insert, insertion failure*/
      return false;
    }

    if (ret == 0) {
      ++size_;
      break;
    }

    DCHECK_EQ(-1, ret);  // Full

    if (target->local_depth() == global_depth_) {
      IncreaseDepth(global_depth_ + 1);

      x = SegmentId(key_hash);
      DCHECK(x < segment_.size() && segment_[x] == target);
    }

    size_t chunk_size = 1u << (global_depth_ - target->local_depth());
    size_t start_idx = x & (~(chunk_size - 1));
    DCHECK(segment_[start_idx] == target && segment_[start_idx + chunk_size - 1] == target);
    auto* s = new detail::Segment(target->local_depth() + 1);
    target->Split(hash_fn_, s);  // increases the depth.
    ++unique_segments_;

    for (size_t i = start_idx + chunk_size / 2; i < start_idx + chunk_size; ++i) {
      segment_[i] = s;
    }
  }

  return true;
}

void DashTable::IncreaseDepth(unsigned new_depth) {
  DCHECK(!segment_.empty());
  DCHECK_GT(new_depth, global_depth_);
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
