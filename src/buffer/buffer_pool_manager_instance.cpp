//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// buffer_pool_manager_instance.cpp
//
// Identification: src/buffer/buffer_pool_manager.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/buffer_pool_manager_instance.h"

#include "common/macros.h"
#include "common/logger.h"

namespace bustub {

BufferPoolManagerInstance::BufferPoolManagerInstance(size_t pool_size, DiskManager *disk_manager,
                                                     LogManager *log_manager)
    : BufferPoolManagerInstance(pool_size, 1, 0, disk_manager, log_manager) {}

BufferPoolManagerInstance::BufferPoolManagerInstance(size_t pool_size, uint32_t num_instances, uint32_t instance_index,
                                                     DiskManager *disk_manager, LogManager *log_manager)
    : pool_size_(pool_size),
      num_instances_(num_instances),
      instance_index_(instance_index),
      next_page_id_(instance_index),
      disk_manager_(disk_manager),
      log_manager_(log_manager) {
  BUSTUB_ASSERT(num_instances > 0, "If BPI is not part of a pool, then the pool size should just be 1");
  BUSTUB_ASSERT(
      instance_index < num_instances,
      "BPI index cannot be greater than the number of BPIs in the pool. In non-parallel case, index should just be 1.");
  // We allocate a consecutive memory space for the buffer pool.
  pages_ = new Page[pool_size_];
  replacer_ = new LRUReplacer(pool_size);

  // Initially, every page is in the free list.
  for (size_t i = 0; i < pool_size_; ++i) {
    free_list_.emplace_back(static_cast<int>(i));
  }
}

BufferPoolManagerInstance::~BufferPoolManagerInstance() {
  delete[] pages_;
  delete replacer_;
}
/**
 * find the unused page from free list and lrureplacer
 * always find in free_list first
 * @return -1 if there is no page we can use now
 */
frame_id_t BufferPoolManagerInstance::FindUnusedPage() {
  auto frame_id=-1;
  if(!free_list_.empty()){
    frame_id= free_list_.front();
    free_list_.pop_front();
    return frame_id;
  }
  if(replacer_->Victim(&frame_id)){
    auto page_id=page_table_[frame_id];
    if(pages_[frame_id].is_dirty_){
      disk_manager_->WritePage(page_id,pages_[frame_id].GetData());
    }
    page_table_.erase(page_id);
    pages_[frame_id].is_dirty_= false;
    return frame_id;
  }
  return -1;
}
frame_id_t BufferPoolManagerInstance::FindFrame(page_id_t page_id){
  auto iter = page_table_.find(page_id);
  if (iter != page_table_.cend()) {
    return iter->second;
  }
  return -1;
}


bool BufferPoolManagerInstance::FlushPgImp(page_id_t page_id) {
  // Make sure you call DiskManager::WritePage!
  std::lock_guard<std::mutex> lg(latch_);
  frame_id_t getted_frameid= FindFrame(page_id);
  if(getted_frameid==-1){
    return false;
  }
  if(pages_[getted_frameid].is_dirty_){
    disk_manager_->WritePage(page_id,pages_[getted_frameid].GetData());
  }
  pages_[getted_frameid].is_dirty_= false;
  return true;
}

void BufferPoolManagerInstance::FlushAllPgsImp() {
  // You can do it!
    for(auto it=page_table_.begin();it!=page_table_.end();it++){
      FlushPgImp(it->first);
    }
}

Page *BufferPoolManagerInstance::NewPgImp(page_id_t *page_id) {
  // 0.   Make sure you call AllocatePage!
  // 1.   If all the pages in the buffer pool are pinned, return nullptr.
  // 2.   Pick a victim page P from either the free list or the replacer. Always pick from the free list first.
  // 3.   Update P's metadata, zero out memory and add P to the page table.
  // 4.   Set the page ID output parameter. Return a pointer to P.
  //1.Find fresh page in lru replacer
  //先在free_list里面找空闲的page，如果找不到就到replacer里面找可以替换的page
  std::lock_guard<std::mutex> lg(latch_);
  frame_id_t getted_frame_id=-1;
  if(!free_list_.empty()){
    getted_frame_id=free_list_.front();
    LOG_INFO("%d",static_cast<int>(getted_frame_id));
    free_list_.pop_front();
  }
  if(getted_frame_id==-1&&replacer_->Victim(&getted_frame_id)){
    auto getted_page_id = pages_[getted_frame_id].page_id_;
    if(pages_[getted_frame_id].is_dirty_){
      disk_manager_->WritePage(getted_page_id,pages_[getted_frame_id].GetData());
    }
    page_table_.erase(getted_page_id);
    pages_[getted_frame_id].is_dirty_= false;
  }
  if(getted_frame_id==-1||getted_frame_id==INVALID_PAGE_ID){
    return nullptr;
  }
  auto new_page_id = AllocatePage();
  *page_id=new_page_id;
  pages_[getted_frame_id].pin_count_++;
  pages_[getted_frame_id].page_id_=new_page_id;
  page_table_[new_page_id]=getted_frame_id;
  memset(pages_[getted_frame_id].GetData(),0,PAGE_SIZE);
  return &pages_[getted_frame_id];
}

Page *BufferPoolManagerInstance::FetchPgImp(page_id_t page_id) {
  // 1.     Search the page table for the requested page (P).
  // 1.1    If P exists, pin it and return it immediately.
  // 1.2    If P does not exist, find a replacement page (R) from either the free list or the replacer.
  //        Note that pages are always found from the free list first.
  // 2.     If R is dirty, write it back to the disk.
  // 3.     Delete R from the page table and insert P.
  // 4.     Update P's metadata, read in the page content from disk, and then return a pointer to P.
  std::lock_guard<std::mutex> lg(latch_);
  auto frame_id= FindFrame(page_id);
  //1.1 P exists
  if(frame_id!=-1){
    replacer_->Pin(frame_id);
    pages_[frame_id].pin_count_++;
    pages_[frame_id].is_dirty_= true;
    return &pages_[frame_id];
  }
  //1.2 P not exists search in free list
  frame_id=FindUnusedPage();
  if(frame_id==-1){
    return nullptr;
  }
  page_table_[page_id]=frame_id;
  replacer_->Pin(frame_id);
  pages_[frame_id].pin_count_++;
  pages_[frame_id].is_dirty_= false;
  pages_[frame_id].page_id_=page_id;
  disk_manager_->ReadPage(page_id,pages_[frame_id].GetData());
  return &pages_[frame_id];
}

bool BufferPoolManagerInstance::DeletePgImp(page_id_t page_id) {
  // 0.   Make sure you call DeallocatePage!
  // 1.   Search the page table for the requested page (P).
  // 1.   If P does not exist, return true.
  // 2.   If P exists, but has a non-zero pin-count, return false. Someone is using the page.
  // 3.   Otherwise, P can be deleted. Remove P from the page table, reset its metadata and return it to the free list.
  std::lock_guard<std::mutex> lg(latch_);
  DeallocatePage(page_id);
  if(page_table_.count(page_id)){
    auto frame_id = FindFrame(page_id);
    if(pages_[frame_id].GetPinCount()!=0){
      return false;
    }
    page_table_.erase(page_id);
    pages_[frame_id].is_dirty_= false;
    pages_[frame_id].pin_count_=0;
    pages_[frame_id].page_id_=INVALID_PAGE_ID;
    memset(pages_[frame_id].GetData(),0,PAGE_SIZE);
    free_list_.push_back(frame_id);
    return true;
  }
  return false;
}

bool BufferPoolManagerInstance::UnpinPgImp(page_id_t page_id, bool is_dirty) {
  std::lock_guard<std::mutex> lg(latch_);
  auto frame_id = FindFrame(page_id);
  LOG_INFO("frame id is %d",(int) frame_id);
  if (frame_id == -1 || pages_[frame_id].pin_count_ == 0) {
    return false;
  }
  pages_[frame_id].pin_count_--;
  pages_[frame_id].is_dirty_=is_dirty;
  if(pages_[frame_id].pin_count_==0){
    replacer_->Unpin(frame_id);
    if(pages_[frame_id].is_dirty_){
      disk_manager_->WritePage(page_id,pages_[frame_id].GetData());
    }
  }
  return true;
}

page_id_t BufferPoolManagerInstance::AllocatePage() {
  const page_id_t next_page_id = next_page_id_;
  next_page_id_ += num_instances_;
  ValidatePageId(next_page_id);
  return next_page_id;
}

void BufferPoolManagerInstance::ValidatePageId(const page_id_t page_id) const {
  assert(page_id % num_instances_ == instance_index_);  // allocated pages mod back to this BPI
}

}  // namespace bustub
