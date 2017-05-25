package ltm;
public interface LTMTopicDetailsDataWriterOperations extends DDS.DataWriterOperations {
  int register_instance(ltm.LTMTopicDetails instance);
  int register_instance_w_timestamp(ltm.LTMTopicDetails instance, DDS.Time_t timestamp);
  int unregister_instance(ltm.LTMTopicDetails instance, int handle);
  int unregister_instance_w_timestamp(ltm.LTMTopicDetails instance, int handle, DDS.Time_t timestamp);
  int write(ltm.LTMTopicDetails instance_data, int handle);
  int write_w_timestamp(ltm.LTMTopicDetails instance_data, int handle, DDS.Time_t source_timestamp);
  int dispose(ltm.LTMTopicDetails instance_data, int instance_handle);
  int dispose_w_timestamp(ltm.LTMTopicDetails instance_data, int instance_handle, DDS.Time_t source_timestamp);
  int get_key_value(ltm.LTMTopicDetailsHolder key_holder, int handle);
  int lookup_instance(ltm.LTMTopicDetails instance_data);
}
