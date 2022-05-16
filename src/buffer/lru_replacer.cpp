//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_replacer.cpp
//
// Identification: src/buffer/lru_replacer.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/lru_replacer.h"

namespace bustub {

LRUReplacer::LRUReplacer(size_t num_pages){
  cap=num_pages;
  listiter=std::vector<std::list<frame_id_t>::iterator>(num_pages);
}

LRUReplacer::~LRUReplacer() = default;

bool LRUReplacer::IsInReplacer(frame_id_t frame_id) {
  //empty
  return listiter[frame_id] != std::list<frame_id_t>::iterator{};
}

bool LRUReplacer::Victim(frame_id_t *frame_id) {
  //add latch
  std::lock_guard<std::mutex> lg(latch_);
  if(l.empty()){
    *frame_id=INVALID_PAGE_ID;
    return false;
  }
  *frame_id=l.back();
  l.pop_back();
  listiter[*frame_id]=std::list<frame_id_t>::iterator();
  return true;
}

void LRUReplacer::Pin(frame_id_t frame_id) {
  std::lock_guard<std::mutex> lg(latch_);
  if(!IsInReplacer(frame_id)){
    return;
  }
  l.erase(listiter[frame_id]);
  listiter[frame_id]=std::list<frame_id_t>::iterator();
}

void LRUReplacer::Unpin(frame_id_t frame_id) {
  std::lock_guard<std::mutex> lg(latch_);
  if(IsInReplacer(frame_id)){
    return ;
  }
  l.push_front(frame_id);
  listiter[frame_id]=l.begin();
}

size_t LRUReplacer::Size() {
  std::lock_guard<std::mutex> lg(latch_);
  return l.size();
}

}  // namespace bustub
