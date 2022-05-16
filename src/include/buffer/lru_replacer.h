//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_replacer.h
//
// Identification: src/include/buffer/lru_replacer.h
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <list>
#include <mutex>  // NOLINT
#include <vector>
#include <map>

#include "buffer/replacer.h"
#include "common/config.h"

namespace bustub {

/**
 * LRUReplacer implements the Least Recently Used replacement policy.
 */
class LRUReplacer : public Replacer {
 public:
  /**
   * Create a new LRUReplacer.
   * @param num_pages the maximum number of pages the LRUReplacer will be required to store
   */
  explicit LRUReplacer(size_t num_pages);

  /**
   * Destroys the LRUReplacer.
   */
  ~LRUReplacer() override;

  bool Victim(frame_id_t *frame_id) override;

  void Pin(frame_id_t frame_id) override;

  void Unpin(frame_id_t frame_id) override;

  size_t Size() override;
 private:
  // TODO(student): implement me!
  /**
   * Check if the frame is already in LRUReplacer
   * @param frame_id
   * @return true if the frame is already in LRUReplacer
   */
  bool IsInReplacer(frame_id_t frame_id);
  /** Double list to implment LRU  */
  std::list<frame_id_t> l;
  /** Hash table used to find item in list*/
  std::vector<std::list<frame_id_t>::iterator> listiter;

  std::vector<frame_id_t> framevec;
  /** maxsize of the LRU*/
  int cap;
  /** for thread safe*/
  std::mutex latch_{};  //thread safe lock
};

}  // namespace bustub
