// Copyright 2021, Beeri 15.  All rights reserved.
// Author: Roman Gershman (romange@gmail.com)
//
#pragma once

#include <cassert>
#include <cstdint>
#include <functional>
#include <vector>
#include <immintrin.h>

namespace base {

namespace detail {

class Segment;

class BucketBase {
  friend class Segment;

  static constexpr unsigned kOverflowLen = 4;
  static constexpr unsigned kOverflowBit = 1 << 4;
  static constexpr unsigned kOverflowMask = kOverflowBit - 1;

  // 4 bits reserved for counting number of busy slots.
  static constexpr unsigned kBitmapLenMask = (1 << 4) - 1;

 public:
  using SlotId = uint8_t;
  enum { kNanSlot = 255 };

  BucketBase();

  bool IsFull() const {
    return Size() == kNumSlots;
  }

  unsigned Size() const {
    return bitmap_ & kBitmapLenMask;
  }

  // Returns -1 if no free slots found or smallest slot index
  int FindEmptySlot() const {
    if (IsFull()) {
      return -1;
    }

    return GetEmptySlot();
  }

  // Must have empty slot, returns result in [0, kNumSlots) range.
  int GetEmptySlot() const {
    uint32_t mask = ~(GetBusy());

    // returns the index for first set bit (FindLSBSetNonZero). mask must be non-zero.
    return __builtin_ctz(mask);
  }

  void Delete(SlotId sid) {
    ClearHash(sid);
  }

  static constexpr unsigned kNumSlots = 12; /* it is determined by the usage of the fingerprint*/

  unsigned Find(uint8_t fp_hash, bool probe) const {
    unsigned mask = CompareFP(fp_hash) & GetBusy();
    return mask & (GetProbe() ^ ((1u - probe) * kAllocMask));
  }

  uint8_t Fp(unsigned i) const {
    return finger_array_[i];
  }

 protected:
  static constexpr unsigned kAllocMask = (1u << kNumSlots) - 1;

  unsigned FindOverflowSlot() const {
    unsigned mask = overflow_bitmap_ & kOverflowMask;
    return __builtin_ctz(~mask);
  }

  void SetOverflow(uint8_t fp, unsigned index, unsigned stash_pos) {
    assert(index < kOverflowLen);

    finger_array_[kNumSlots + index] = fp;
    overflow_bitmap_ |= (1u << index);  // set the overflow slot

    // 2 bits denote the bucket index.
    index *= 2;
    overflow_pos_ &= (~(3 << index));       // clear (can be removed?)
    overflow_pos_ |= (stash_pos << index);  // and set
  }

  bool HasStash() const {
    return overflow_bitmap_ & kOverflowBit;
  }

  bool HasStashOverflow() const {
    return overflow_count_ > 0;
  }

  template <typename F>
  std::pair<unsigned, SlotId> IterateStash(uint8_t fp, bool is_own, F&& func) const {
    unsigned om = is_own ? ~overflow_member_ : overflow_member_;
    unsigned ob = overflow_bitmap_;

    for (unsigned i = 0; i < kOverflowLen; ++i) {
      if ((ob & 1) && (finger_array_[kNumSlots + i] == fp) && (om & 1)) {
        unsigned pos = (overflow_pos_ >> (i * 2)) & 3;
        auto sid = func(i, pos);
        if (sid != BucketBase::kNanSlot) {
          return std::pair<unsigned, SlotId>(pos, sid);
        }
      }
      ob >>= 1;
      om >>= 1;
    }
    return std::pair<unsigned, SlotId>(0, BucketBase::kNanSlot);
  }

  bool ClearStash(uint8_t fp, unsigned stash_pos, bool is_own);

  void SetHash(unsigned index, uint8_t meta_hash, bool probe) {
    assert(index < kNumSlots + kOverflowLen);
    finger_array_[index] = meta_hash;
    bitmap_ |= (1 << (index + 18));
    bitmap_ |= (unsigned(probe) << (index + 4));

    assert((bitmap_ & 15) < kNumSlots);
    bitmap_ += 1;
  }

  void ClearHash(unsigned index) {
    uint32_t new_bitmap = bitmap_ & (~(1u << (index + 18))) & (~(1u << (index + 4)));
    assert(Size() <= kNumSlots);
    assert(Size() > 0);
    new_bitmap -= 1;
    bitmap_ = new_bitmap;
  }

  // mask is 14 bits saying which slots needs to be freed (1 - should clear).
  void ClearSlots(uint32_t mask) {
    uint32_t count = __builtin_popcount(mask);
    mask = (mask << 4) | (mask << 18);
    bitmap_ &= ~mask;
    bitmap_ -= count;
  }

  // GetBusy returns the busy mask.
  uint32_t GetBusy() const {
    return bitmap_ >> 18;
  }

  // probe - true means the entry is probing, i.e. not owning.
  // GetProbe returns index of probing entries, i.e. hosted by not owned by this bucket.
  uint32_t GetProbe() const {
    return (bitmap_ >> 4) & kAllocMask;
  }

  unsigned CompareFP(uint8_t fp) const;

  uint8_t overflow_bitmap_ = 0;
  uint8_t overflow_pos_ = 0;
  uint8_t overflow_member_ = 0; /*overflow_member_ indicates membership of the overflow
                             fingerprint*/
  uint8_t overflow_count_ = 0;

  // bitmap is [14 bit- busy][14bit-probing, whether the key does not belong to this
  // bucket][4bit-count]
  uint32_t bitmap_ = 0;  // allocation bitmap + pointer bitmap + counter

  /*only use the first 14 bytes, can be accelerated by
    SSE instruction,0-13 for finger, 14-17 for overflowed*/
  uint8_t finger_array_[kNumSlots + kOverflowLen];
};

class Segment {
  static constexpr size_t kBucketSz = sizeof(BucketBase);

  static constexpr size_t kFingerBits = 8;

  /* the number of stash buckets in one segment, must be power of 2*/
  static constexpr unsigned kNumStashBucket = 2;

 public:
  static constexpr unsigned kStashMask = kNumStashBucket - 1;
  static constexpr unsigned kNumBucket = 64; /* the number of normal buckets in one segment*/
  static constexpr size_t kFpMask = (1 << kFingerBits) - 1;
  static constexpr size_t kNumSlots = BucketBase::kNumSlots;

  using Value_t = uint64_t;
  using Key_t = uint64_t;
  using comp_fun = bool (*)(uint64_t, uint64_t);

  // using Value_t = Bucket::Value_t;
  // using Key_t = Bucket::Key_t;
  using DtorFn = void (*)(uint64_t);
  using HashFn = size_t (*)(Key_t);

  explicit Segment(size_t depth) : local_depth_(depth) {
  }

  ~Segment();

  // Returns 0 if insert succeeds, -1 if it's full, -3 for duplicate,
  int Insert(Key_t key, Value_t value, size_t key_hash, comp_fun cmp_fun);

  void Split(HashFn hfn, Segment* dest);

  bool Find(Key_t key, size_t key_hash, comp_fun cf, Value_t* res) const;

  bool Find(Key_t key, size_t key_hash, comp_fun cf) const {
    auto it = FindIt(key, key_hash, cf);
    return it.found();
  }

  bool Delete(Key_t key, size_t key_hash, comp_fun cf, DtorFn dfun);

  void DeleteKeys(DtorFn dfun);

  const BucketBase& operator[](size_t i) const {
    return bucket_[i].b;
  }

  size_t size() const {
    return size_;
  }

  static constexpr size_t capacity() {
    return kMaxSize;
  }

  size_t local_depth() const { return local_depth_; }

 private:
  static constexpr uint8_t kNanBuckedId = 255;
  struct Iterator {
    uint8_t index;  // bucket index
    uint8_t slot;

    Iterator() : index(kNanBuckedId), slot(BucketBase::kNanSlot) {
    }
    Iterator(unsigned bi, unsigned sid) : index(bi), slot(sid) {
    }

    bool found() const {
      return index != kNanBuckedId;
    }
  };

  static_assert(sizeof(Iterator) == 2, "");

  static constexpr size_t kBucketMask = kNumBucket - 1;
  static constexpr size_t kMaxSize = (kNumBucket + kNumStashBucket) * BucketBase::kNumSlots;

  static unsigned BucketIndex(uint64_t hash) {
    return (hash >> kFingerBits) & kBucketMask;
  }

#define COLOC_KV 1

#ifdef COLOC_KV
  Key_t& key(unsigned id, unsigned slot) {
    return bucket_[id].key[slot];
  }

  const Key_t& key(unsigned id, unsigned slot) const {
    return bucket_[id].key[slot];
  }

  Value_t& value(unsigned id, unsigned slot) {
    return bucket_[id].value[slot];
  }

  const Value_t& value(unsigned id, unsigned slot) const {
    return bucket_[id].value[slot];
  }

#else
  Key_t& key(unsigned id, unsigned slot) {
    return key_[id * kNumSlots + slot];
  }

  const Key_t& key(unsigned id, unsigned slot) const {
    return key_[id * kNumSlots + slot];
  }

  Value_t& value(unsigned id, unsigned slot) {
    return value_[id * kNumSlots + slot];
  }
  const Value_t& value(unsigned id, unsigned slot) const {
    return value_[id * kNumSlots + slot];
  }
#endif

  template <typename F>
  BucketBase::SlotId FindByMask(unsigned bidx, Key_t k, unsigned mask, F&& f) const {
    unsigned delta = __builtin_ctz(mask);
    mask >>= delta;
    const Key_t* start = &key(bidx, 0);
    for (unsigned i = delta; i < kNumSlots; ++i) {
      if ((mask & 1) && f(start[i], k)) {
        return i;
      }
      mask >>= 1;
    };

    return BucketBase::kNanSlot;
  }

  template <typename F> void ForEach(unsigned bidx, F&& f) {
    uint32_t mask = bucket_[bidx].b.GetBusy();
    uint32_t probe_mask = bucket_[bidx].b.GetProbe();
    const Key_t* k = &key(bidx, 0);
    const Value_t* v = &value(bidx, 0);
    for (unsigned j = 0; j < kNumSlots; ++j) {
      if (mask & 1) {
        f(j, k[j], v[j], probe_mask & 1);
      }
      mask >>= 1;
      probe_mask >>= 1;
    }
  }

  // if own_items is true it means we try to move owned item to probing bucket.
  // if own_items false it means we try to move non-owned item from probing bucket back to its host.
  int MoveToOther(bool own_items, unsigned from, unsigned to) {
    auto& src = bucket_[from].b;
    uint32_t mask = src.GetProbe() ^ (unsigned(own_items) * BucketBase::kAllocMask);
    if (mask == 0) {
      return -1;
    }
    int displace_index = __builtin_ctz(mask);

    if (!TryInsertUniq(to, key(from, displace_index), value(from, displace_index),
                       src.Fp(displace_index), own_items))
      return -1;

    src.ClearHash(displace_index);

    return displace_index;
  }

  // Returns true if insertion is succesful, false if no free slots are found.
  bool TryInsertUniq(unsigned bidx, Key_t key, Value_t value, uint8_t meta_hash, bool probe) {
    auto slot = bucket_[bidx].b.FindEmptySlot();
    assert(slot < int(kNumSlots));
    if (slot == -1) {
      return false;
    }

    InsertToBucket(Iterator{bidx, unsigned(slot)}, key, value, meta_hash, probe);

    return true;
  }

  bool InsertUniq(Key_t key, Value_t value, size_t key_hash);

  // Used is cases where we know that the key is not exist in the table.
  // Returns true if insertion succeeds, false if segment is full.
  void InsertToBucket(Iterator it, Key_t k, Value_t v, uint8_t meta_hash, bool probe) {
    auto& b = bucket_[it.index];
    key(it.index, it.slot) = k;
    value(it.index, it.slot) = v;
    b.b.SetHash(it.slot, meta_hash, probe);
  }

  // stash_pos is an initial offset that probably serves to balance stash filling.
  // Returns true if insertion succeeds or false if
  bool StashInsertUniq(BucketBase* target, BucketBase* neighbor, Key_t key, Value_t value,
                       uint8_t meta_hash, unsigned stash_pos) {
    for (unsigned i = 0; i < kNumStashBucket; ++i) {
      unsigned stash_id = (stash_pos + i) & kStashMask;
      bool res = TryInsertUniq(kNumBucket + stash_id, key, value, meta_hash, false);
      if (res) {
        SetStashPtr(stash_id, meta_hash, target, neighbor);
        return true;
      }
    }
    return false;
  }

  Iterator FindIt(Key_t key, size_t key_hash, comp_fun cf) const;

  void SetStashPtr(unsigned stash_pos, uint8_t meta_hash, BucketBase* target, BucketBase* next);

  /*both clear this bucket and its neighbor bucket*/
  void UnsetStashPtr(unsigned stash_pos, uint8_t meta_hash, BucketBase* target,
                     BucketBase* neighbor);

  enum { kTotalBuckets = kNumBucket + kNumStashBucket };

  struct Bucket {
    BucketBase b;
#ifdef COLOC_KV
    Key_t key[kNumSlots];
    Value_t value[kNumSlots];
#endif
  };

  static_assert(sizeof(Bucket) <= 256, "");

  Bucket bucket_[kTotalBuckets];

#ifndef COLOC_KV
  Key_t key_[kTotalBuckets * kNumSlots];
  Value_t value_[kTotalBuckets * kNumSlots];
#endif

  size_t local_depth_;
  uint16_t size_ = 0;
};

constexpr size_t kSegSize = sizeof(Segment);

}  // namespace detail

class DashTable {
  // TBD
  DashTable(const DashTable&) = delete;
  DashTable& operator=(const DashTable&) = delete;

 public:
  using Key_t = detail::Segment::Key_t;
  using Value_t = detail::Segment::Value_t;
  using CompFn = detail::Segment::comp_fun;
  using HashFn = detail::Segment::HashFn;
  using DtorFn = detail::Segment::DtorFn;

  explicit DashTable(size_t capacity_log = 1);
  ~DashTable();

  void Reserve(size_t size);

  // false for duplicate, true if inserted.
  bool Insert(Key_t key, Value_t value);
  bool Find(Key_t key, Value_t* res) const;

  bool Delete(Key_t);
  bool Find(Key_t key) const;

  uint32_t unique_segments() const {
    return unique_segments_;
  }
  uint32_t depth() const {
    return global_depth_;
  }

  size_t size() const {
    return size_;
  }

  uint64_t Hash(Key_t k) const {
    if (hash_fn_)
      return hash_fn_(k);
    return DefaultHash(k);
  }

  size_t mem_usage() const {
    return segment_.capacity() * sizeof(void*) + sizeof(detail::Segment) * unique_segments_;
  }

  double load_factor() const {
    return double(size()) / (detail::Segment::capacity() * unique_segments());
  }

  void set_cmp(CompFn cmp_fun) {
    cmp_fun_ = cmp_fun;
  }

  void set_key_dtor(DtorFn dfun) {
    key_dtor_fun_ = dfun;
  }

  void set_hash_fn(HashFn fn) {
    hash_fn_ = fn;
  }

 private:
  size_t SegmentId(size_t hash) const {
    return hash >> (64 - global_depth_);
  }

  void IncreaseDepth(unsigned new_depth);
  static size_t DefaultHash(Key_t k);

  uint32_t global_depth_;
  uint32_t unique_segments_;
  size_t size_ = 0;
  CompFn cmp_fun_ = nullptr;
  DtorFn key_dtor_fun_ = nullptr;
  HashFn hash_fn_ = nullptr;

  std::vector<detail::Segment*> segment_;
};

inline unsigned detail::BucketBase::CompareFP(uint8_t fp) const {
  // Replicate 16 times uint8_t key to key_data.
  const __m128i key_data = _mm_set1_epi8(fp);

  // Loads 16 bytes of src into seg_data.
  __m128i seg_data = _mm_loadu_si128(reinterpret_cast<const __m128i*>(finger_array_));

  // compare 16-byte vectors seg_data and key_data, dst[i] := ( a[i] == b[i] ) ? 0xFF : 0.
  __m128i rv_mask = _mm_cmpeq_epi8(seg_data, key_data);

  // collapses 16 msb bits from each byte in rv_mask into mask.
  int mask = _mm_movemask_epi8(rv_mask);

  // Note: Last 2 operations can be combined in skylake with _mm_cmpeq_epi8_mask.
  return mask;
}

inline bool DashTable::Find(Key_t key, Value_t* res) const {
  uint64_t key_hash = Hash(key);
  size_t x = SegmentId(key_hash);
  const auto* target = segment_[x];

  return target->Find(key, key_hash, cmp_fun_, res);
}

inline bool DashTable::Find(Key_t key) const {
  uint64_t key_hash = Hash(key);
  size_t x = SegmentId(key_hash);
  const auto* target = segment_[x];

  return target->Find(key, key_hash, cmp_fun_);
}

inline bool DashTable::Delete(Key_t key) {
  uint64_t key_hash = Hash(key);
  size_t x = SegmentId(key_hash);
  auto* target = segment_[x];

  return target->Delete(key, key_hash, cmp_fun_, key_dtor_fun_);
}

}  // namespace base
