package dsto;
public interface DRAWDetailsEventListenerOperations extends dsto.DRAWDetailsPlanListenerOperations {
  void register_task(dsto.DRAWDetailsPlanListener listener);
  void remove_task(dsto.DRAWDetailsPlanListener listener);
  int post(dsto.DRAWDetails data);
  String get_event_type();
}
