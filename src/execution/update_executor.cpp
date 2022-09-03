//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// update_executor.cpp
//
// Identification: src/execution/update_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
#include <memory>

#include "execution/executors/update_executor.h"

namespace bustub {

UpdateExecutor::UpdateExecutor(ExecutorContext *exec_ctx, const UpdatePlanNode *plan,
                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx),plan_(plan),child_executor_(child_executor) {}

void UpdateExecutor::Init() {
  catalog_ = exec_ctx_->GetCatalog();
  table_info_ = catalog_->GetTable(plan_->TableOid());
  index_infos_ = catalog_.GetTableIndexes(table_info_->name_);
}

bool UpdateExecutor::Next([[maybe_unused]] Tuple *tuple, RID *rid) {
  Tuple old_tuple;
  Tuple new_tuple;
  RID cur_rid;

  auto lock_mgr = exec_ctx_->GetLockManager();
  auto txn = exec_ctx_->GetTransaction();

  //执行子查询
  while (true){
    //chile executor excute Next
    try{
    if(!child_executor_->Next(&old_tuple,&cur_rid)) {
      break;
    }
  }catch (Exception &e){
    throw Exception(ExceptionType::UNKNOWN_TYPE,"Child executor error");
    return false;
  }

  if(lock_mgr!= nullptr){
    if(txn->IsSharedLocked(cur_rid)){
      lock_mgr->LockUpgrade(txn,cur_rid);
    } else if(txn->IsExclusiveLocked(cur_rid)){
      lock_mgr->LockExclusive(txn,cur_rid);
    }
  }

  new_tuple = GenerateUpdatedTuple(old_tuple);
  table_info_->table_->UpdateTuple(new_tuple,cur_rid,txn);
  }

  for(const auto &index: index_infos_){
    auto index_info = index->index_.get();
    //delete
    index_info->DeleteEntry(old_tuple.KeyFromTuple(table_info_->schema_,*index_info->GetKeySchema(),index_info->GetKeyAttrs()),cur_rid,txn);
    //insert
    index_info->InsertEntry(new_tuple.KeyFromTuple(table_info_->schema_,*index_info->GetKeySchema(),index_info->GetKeyAttrs()),cur_rid,txn);
    IndexWriteRecord write_record(cur_rid, table_info_->oid_, WType::DELETE, new_tuple, index->index_oid_,
                                  exec_ctx_->GetCatalog());
    write_record.tuple_=new_tuple;
    txn->GetWriteSet()->emplace_back(write_record);

    if (txn->GetIsolationLevel() == IsolationLevel::READ_COMMITTED && lock_mgr != nullptr) {  // 提交读才需要解锁
      lock_mgr->Unlock(txn, tuple_rid);
    }
  }


  return false;
}

Tuple UpdateExecutor::GenerateUpdatedTuple(const Tuple &src_tuple) {
  const auto &update_attrs = plan_->GetUpdateAttr();
  Schema schema = table_info_->schema_;
  uint32_t col_count = schema.GetColumnCount();
  std::vector<Value> values;
  for (uint32_t idx = 0; idx < col_count; idx++) {
    if (update_attrs.find(idx) == update_attrs.cend()) {
      values.emplace_back(src_tuple.GetValue(&schema, idx));
    } else {
      const UpdateInfo info = update_attrs.at(idx);
      Value val = src_tuple.GetValue(&schema, idx);
      switch (info.type_) {
        case UpdateType::Add:
          values.emplace_back(val.Add(ValueFactory::GetIntegerValue(info.update_val_)));
          break;
        case UpdateType::Set:
          values.emplace_back(ValueFactory::GetIntegerValue(info.update_val_));
          break;
      }
    }
  }
  return Tuple{values, &schema};
}

}  // namespace bustub
