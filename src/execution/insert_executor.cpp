//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// insert_executor.cpp
//
// Identification: src/execution/insert_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <memory>

#include "execution/executors/insert_executor.h"

namespace bustub {

InsertExecutor::InsertExecutor(ExecutorContext *exec_ctx, const InsertPlanNode *plan,
                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx),plan_(plan),child_executor_(child_executor) {}

void InsertExecutor::Init() {
  tableinfo_ = exec_ctx_->GetCatalog()->GetTable(plan_->TableOid());
  indexs_ = exec_ctx_->GetCatalog()->GetTableIndexes(tableinfo_->name_);
  if(child_executor_== nullptr){
    raw_iter_=plan_->RawValues().begin();
  }else{
    child_executor_->Init();
  }
}
void InsertExecutor::InsertRawTupleWithIndex(bustub::Tuple *cur_tuple) {
  RID cur_rid;
  auto txn = exec_ctx_->GetTransaction();
  if(!tableinfo_->table_->InsertTuple(*cur_tuple,&cur_rid,txn)){
    throw Exception(ExceptionType::OUT_OF_MEMORY, "InsertExecutor: no enough space for this tuple");
  }
  auto lock_mgr = exec_ctx_->GetLockManager();
  //写操作所以需要加排他锁
  if(lock_mgr!= nullptr){
    if(txn->IsSharedLocked(cur_rid)){
      lock_mgr->LockUpgrade(txn,cur_rid);
    }else if(txn->IsExclusiveLocked(txn,cur_rid)){
      lock_mgr->LockExclusive(txn,cur_rid);
    }
  }
  //修改索引
  for(const auto &index : indexs_){
    index->index_->InsertEntry(cur_tuple->KeyFromTuple(tableinfo_->schema_,*index->index_->GetKeySchema(),index->index_->GetKeyAttrs()),cur_rid,txn);
    IndexWriteRecord write_record(cur_rid,tableinfo_->oid_,WType::INSERT,*cur_tuple,index->index_oid_,exec_ctx_->GetCatalog());
    write_record.tuple_ = *cur_tuple;
    txn->GetIndexWriteSet()->emplace_back(write_record);
  }
  //解锁 Read_Committed 才需要解锁
  if(txn->GetIsolationLevel()==IsolationLevel::READ_COMMITTED && lock_mgr!= nullptr){
    lock_mgr->Unlock(txn,cur_rid);
  }

}

bool InsertExecutor::Next([[maybe_unused]] Tuple *tuple, RID *rid) {
  //RawInsert
  if(plan_->IsRawInsert()){
    for(auto const &rowvalues : plan_->RawValues()){
      Tuple temp_tuple(rowvalues,tableinfo_->schema_);
      InsertRawTupleWithIndex(&temp_tuple);
    }
  }
  // Child query insert
  std::vector<Tuple> child_tuples;
  child_executor_->Init();
  try{
    RID temp_rid;
    Tuple temp_tuple;
    while (child_executor_->Next(&temp_tuple,&temp_rid)){
      child_tuples.push_back(temp_tuple);
    }
  }catch {
    throw Exception(ExceptionType::UNKNOWN_TYPE,"Can not execute child executor");
    return false;
  }
  for(auto &t_:child_tuples){
    InsertRawTupleWithIndex(&t_);
  }
  return false;
}

}  // namespace bustub
