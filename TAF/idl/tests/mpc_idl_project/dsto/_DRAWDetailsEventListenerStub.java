package dsto;
public class _DRAWDetailsEventListenerStub extends i2jrt.TAOObject implements DRAWDetailsEventListener {
  protected _DRAWDetailsEventListenerStub(long ptr) {
    super(ptr);
  }

  public native void register_task(dsto.DRAWDetailsPlanListener listener);

  public native void remove_task(dsto.DRAWDetailsPlanListener listener);

  public native int post(dsto.DRAWDetails data);

  public native String get_event_type();

  public native String get_plan_name();

  public native int task(dsto.DRAWDetails data);

}
