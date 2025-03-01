//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// seq_scan_executor.cpp
//
// Identification: src/execution/seq_scan_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "execution/executors/seq_scan_executor.h"

namespace bustub {

SeqScanExecutor::SeqScanExecutor(ExecutorContext *exec_ctx, const SeqScanPlanNode *plan) : AbstractExecutor(exec_ctx),plan_(plan),table_iterator_(nullptr,RID{}, nullptr) {}

void SeqScanExecutor::Init() {
  //get the table which to scan
  table_heap_ = exec_ctx_->GetCatalog()->GetTable(plan_->GetTableOid())->table_.get();
  table_iterator_ = table_heap_->Begin(exec_ctx_->GetTransaction());
}

bool SeqScanExecutor::Next(Tuple *tuple, RID *rid) {
  if(table_iterator_== table_heap_->End()){
    return false;
  }
  auto rid_ = table_iterator_->tuple_->GetRid();

  const Schema* output_schema = plan_->OutputSchema();
  auto lock_mgr = exec_ctx_->GetLockManager();
  auto txn = exec_ctx_->GetTransaction();
  //add lock
  if(lock_mgr!= nullptr){
    if(txn->GetIsolationLevel() != IsolationLevel::READ_UNCOMMITTED){
      if(!txn->IsSharedLocked(rid_) && !txn->IsExclusiveLocked(rid_)){
        lock_mgr->LockShared(txn,rid_);
      }
    }
  }

  std::vector<Value> values;
  values.reserve(output_schema->GetColumnCount());
  for(size_t i=0;i<values.capacity();++i){
    values.push_back(output_schema->GetColumn(i).GetExpr()->Evaluate(&(*table_iterator_), &(exec_ctx_->GetCatalog()->GetTable(plan_->GetTableOid())->schema_)));
  }
  if(txn->GetIsolationLevel()==IsolationLevel::READ_COMMITTED && lock_mgr != nullptr){// READ_COMMITTED need to unlock after read values.
    lock_mgr->Unlock(txn,rid_);
  }
  ++table_iterator_;

  Tuple temp_tuple(values,output_schema);

  const auto predicate = plan_->GetPredicate();
  if(predicate== nullptr || predicate->Evaluate(temp_tuple,output_schema).GetAs<bool>()){
    *tuple=temp_tuple;
    *rid = rid_;
    return true;
  }
  return Next(tuple,rid);
}

}  // namespace bustub
