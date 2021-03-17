// Copyright 2021, Beeri 15.  All rights reserved.
// Author: Roman Gershman (romange@gmail.com)
//
#pragma once

#include <cassert>
#include <cstdint>
#include <functional>
#include <vector>

namespace base {

class Segment;
class DashTable;
class BucketBase {
  friend class Segment;

  static constexpr unsigned kOverflowLen = 4;
  static constexpr unsigned kOverflowBit = 1 << kOverflowLen;
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

  static constexpr unsigned kNumSlots = 14; /* it is determined by the usage of the fingerprint*/

 protected:
  static constexpr unsigned kAllocMask = (1u << kNumSlots) - 1;

  unsigned FindOverflowSlot() const {
    unsigned mask = overflow_bitmap_ & kOverflowMask;
    return __builtin_ctz(~mask);
  }

  void SetOverflow(uint8_t fp, unsigned index, unsigned stash_pos) {
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

  uint32_t version_unsused_ = 0;  // could be used for CAS and MVC.

  // bitmap is [14 bit- busy][14bit-probing, whether the key does not belong to this
  // bucket][4bit-count]
  uint32_t bitmap_ = 0;  // allocation bitmap + pointer bitmap + counter

  /*only use the first 14 bytes, can be accelerated by
    SSE instruction,0-13 for finger, 14-17 for overflowed*/
  uint8_t finger_array_[kNumSlots + kOverflowLen];
  uint8_t overflow_bitmap_ = 0;
  uint8_t overflow_pos_ = 0;
  uint8_t overflow_member_ = 0; /*overflow_member_ indicates membership of the overflow
                             fingerprint*/
  uint8_t overflow_count_ = 0;
  uint8_t unused[2];
};

class Bucket : public BucketBase {
 public:
  using Key_t = uint64_t;
  using Value_t = uint64_t;

  Bucket() {
  }

  using comp_fun = bool (*)(uint64_t, uint64_t);

  // Returns true if insertion is succesful, false if no free slots are found.
  bool TryInsertUniq(Key_t key, Value_t value, uint8_t meta_hash, bool probe) {
    auto slot = FindEmptySlot();
    assert(slot < int(kNumSlots));
    if (slot == -1) {
      return false;
    }

    InsertUniq(key, value, meta_hash, slot, probe);

    return true;
  }

  void InsertUniq(Key_t key, Value_t value, uint8_t meta_hash, unsigned slot, bool probe) {
    key_[slot] = key;
    value_[slot] = value;
    SetHash(slot, meta_hash, probe);
  }

  // Returns the slot id if key was found, or kNanSlot if not.
  SlotId Find(Key_t key, comp_fun cf, uint8_t meta_hash, bool probe) const;

  // if own_items is true it means we try to move owned item to probing bucket.
  // if own_items false it means we try to move non-owned item from probing bucket back to its host.
  int MoveToOther(bool own_items, Bucket* other) {
    uint32_t mask = GetProbe() ^ (unsigned(own_items) * kAllocMask);
    if (mask == 0) {
      return -1;
    }
    int displace_index = __builtin_ctz(mask);

    if (!other->TryInsertUniq(key_[displace_index], value_[displace_index],
                              finger_array_[displace_index], own_items))
      return -1;

    ClearHash(displace_index);

    return displace_index;
  }

  template <typename F> void ForEach(F&& f) {
    uint32_t mask = GetBusy();
    uint32_t probe_mask = GetProbe();

    for (unsigned j = 0; j < kNumSlots; ++j) {
      if (mask & 1) {
        f(j, key_[j], value_[j], probe_mask & 1);
      }
      mask >>= 1;
      probe_mask >>= 1;
    }
  }

  const Value_t& value(unsigned id) const {
    return value_[id];
  }

 private:
  struct Pair {
    Key_t key;
    Value_t value;
  };

  Key_t key_[kNumSlots];
  Value_t value_[kNumSlots];
};

class Segment {
  friend class DashTable;

  static constexpr size_t kFingerBits = 8;

  /* the number of stash buckets in one segment, must be power of 2*/
  static constexpr unsigned kNumStashBucket = 2;

 public:
  static constexpr unsigned kStashMask = kNumStashBucket - 1;
  static constexpr unsigned kNumBucket = 64; /* the number of normal buckets in one segment*/
  static constexpr size_t kFpMask = (1 << kFingerBits) - 1;

  using Value_t = Bucket::Value_t;
  using Key_t = Bucket::Key_t;
  using DtorFn = void (*)(uint64_t);
  using HashFn = size_t (*)(Key_t);

  explicit Segment(size_t depth) : local_depth_(depth) {
  }

  ~Segment();

  // Returns 0 if insert succeeds, -1 if it's full, -3 for duplicate,
  int Insert(Key_t key, Value_t value, size_t key_hash, Bucket::comp_fun cmp_fun);

  void Split(HashFn hfn, Segment* dest);

  bool Find(Key_t key, size_t key_hash, Bucket::comp_fun cf, Value_t* res) const;
  bool Delete(Key_t key, size_t key_hash, Bucket::comp_fun cf, DtorFn dfun);

  void DeleteKeys(DtorFn dfun);

  const Bucket& operator[](size_t i) const {
    return bucket_[i];
  }

  size_t size() const {
    return size_;
  }

  static constexpr size_t capacity() {
    return kMaxSize;
  }

 private:
  static constexpr uint8_t kNanBuckedId = 255;
  struct Iterator {
    uint8_t index;  // bucket index
    uint8_t slot;

    Iterator() : index(kNanBuckedId), slot(Bucket::kNanSlot) {
    }
    Iterator(unsigned bi, unsigned sid) : index(bi), slot(sid) {
    }

    bool found() const {
      return index != kNanBuckedId;
    }
  };

  static_assert(sizeof(Iterator) == 2, "");

  static constexpr size_t kBucketMask = kNumBucket - 1;
  static constexpr size_t kMaxSize = (kNumBucket + kNumStashBucket) * Bucket::kNumSlots;

  static unsigned BucketIndex(uint64_t hash) {
    return (hash >> kFingerBits) & kBucketMask;
  }

  // Used is cases where we know that the key is not exist in the table.
  // Returns true if insertion succeeds, false if segment is full.
  bool InsertUniq(Key_t key, Value_t value, size_t key_hash);

  // stash_pos is an initial offset that probably serves to balance stash filling.
  // Returns true if insertion succeeds or false if
  bool StashInsertUniq(Bucket* target, Bucket* neighbor, Key_t key, Value_t value,
                       uint8_t meta_hash, unsigned stash_pos) {
    for (unsigned i = 0; i < kNumStashBucket; ++i) {
      Bucket* curr_bucket = bucket_ + kNumBucket + ((stash_pos + i) & kStashMask);
      bool res = curr_bucket->TryInsertUniq(key, value, meta_hash, false);
      if (res) {
        SetStashPtr((stash_pos + i) & kStashMask, meta_hash, target, neighbor);
        return true;
      }
    }
    return false;
  }

  Iterator Find(Key_t key, size_t key_hash, Bucket::comp_fun cf) const;

  void SetStashPtr(unsigned stash_pos, uint8_t meta_hash, Bucket* target, Bucket* next);

  /*both clear this bucket and its neighbor bucket*/
  void UnsetStashPtr(unsigned stash_pos, uint8_t meta_hash, Bucket* target, Bucket* neighbor);

  Bucket bucket_[kNumBucket + kNumStashBucket];
  size_t local_depth_;
  uint16_t size_ = 0;
};

class DashTable {
  // TBD
  DashTable(const DashTable&) = delete;
  DashTable& operator=(const DashTable&) = delete;

 public:
  using Key_t = Bucket::Key_t;
  using Value_t = Bucket::Value_t;

  explicit DashTable(size_t capacity_log = 1);
  ~DashTable();

  void Reserve(size_t size);

  // -1 for duplicate, 0 if inserted.
  int Insert(Key_t key, Value_t value);
  bool Find(Key_t key, Value_t* res) const;

  bool Delete(Key_t);

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
    return segment_.capacity() * sizeof(Segment*) + sizeof(Segment) * unique_segments_;
  }

  double load_factor() const { return double(size()) / (Segment::capacity() * unique_segments()) ;}

  void set_cmp(Bucket::comp_fun cmp_fun) {
    cmp_fun_ = cmp_fun;
  }

  void set_key_dtor(Segment::DtorFn dfun) {
    key_dtor_fun_ = dfun;
  }

  void set_hash_fn(Segment::HashFn fn) {
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
  Bucket::comp_fun cmp_fun_ = nullptr;
  Segment::DtorFn key_dtor_fun_ = nullptr;
  Segment::HashFn hash_fn_ = nullptr;

  std::vector<Segment*> segment_;
};

inline bool DashTable::Find(Key_t key, Value_t* res) const {
  uint64_t key_hash = Hash(key);
  size_t x = SegmentId(key_hash);

  const Segment* target = segment_[x];

  return target->Find(key, key_hash, cmp_fun_, res);
}

inline bool DashTable::Delete(Key_t key) {
  uint64_t key_hash = Hash(key);
  size_t x = SegmentId(key_hash);
  Segment* target = segment_[x];

  return target->Delete(key, key_hash, cmp_fun_, key_dtor_fun_);
}

}  // namespace base
