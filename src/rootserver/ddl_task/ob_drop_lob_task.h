/**
 * Copyright (c) 2021 OceanBase
 * OceanBase CE is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan PubL v2.
 * You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PubL v2 for more details.
 */

#ifndef OCEANBASE_ROOTSERVER_OB_DROP_LOB_TASK_H
#define OCEANBASE_ROOTSERVER_OB_DROP_LOB_TASK_H

#include "rootserver/ddl_task/ob_ddl_task.h"

namespace oceanbase
{
namespace rootserver
{

class ObDropLobTask : public ObDDLTask
{
public:
  ObDropLobTask();
  virtual ~ObDropLobTask();
  int init(
      const uint64_t tenant_id,
      const int64_t task_id,
      const uint64_t aux_lob_meta_table_id,
      const uint64_t data_table_id,
      const int64_t schema_version,
      const int64_t parent_task_id,
      const int64_t consumer_group_id,
      const obrpc::ObDDLArg &ddl_arg);
  int init(const ObDDLTaskRecord &task_record);
  virtual int process() override;
  virtual bool is_valid() const override;
  virtual int serialize_params_to_message(char *buf, const int64_t buf_size, int64_t &pos) const override;
  virtual int deserialize_params_from_message(const uint64_t tenant_id, const char *buf, const int64_t buf_size, int64_t &pos) override;
  virtual int64_t get_serialize_param_size() const override;
  INHERIT_TO_STRING_KV("ObDDLTask", ObDDLTask, KP_(root_service), K_(ddl_arg));
  virtual int cleanup_impl() override;
private:
  int check_switch_succ_();

  int prepare(const share::ObDDLTaskStatus new_status);
  int drop_lob_impl();
  int drop_lob(const share::ObDDLTaskStatus new_status);
  int succ();
  int fail();
  int deep_copy_ddl_arg(common::ObIAllocator &allocator,
                        const obrpc::ObDDLArg &src_ddl_arg,
                        obrpc::ObDDLArg &dst_ddl_arg);
  virtual bool is_error_need_retry(const int ret_code) override
  {
    UNUSED(ret_code);
    // we should always retry on drop lob task
    return task_status_ <= share::ObDDLTaskStatus::DROP_SCHEMA;
  }
private:
  static const int64_t OB_DROP_LOB_TASK_VERSION = 1;
  ObDDLWaitTransEndCtx wait_trans_ctx_;
  ObRootService *root_service_;
  obrpc::ObDDLArg ddl_arg_;
};

}  // end namespace rootserver
}  // end namespace oceanbase

#endif  // OCEANBASE_ROOTSERVER_OB_DROP_INDEX_TASK_H
