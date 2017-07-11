package dsto;
public class _DRAWDetailsPlanListenerStub extends i2jrt.TAOObject implements DRAWDetailsPlanListener {
  protected _DRAWDetailsPlanListenerStub(long ptr) {
    super(ptr);
  }

  public native String get_plan_name();

  public native int task(dsto.DRAWDetails data);

}
