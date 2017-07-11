package ltm;
public class _CORBActiveServiceStub extends i2jrt.TAOObject implements CORBActiveService {
  protected _CORBActiveServiceStub(long ptr) {
    super(ptr);
  }

  public native int pushTopic(ltm.LTMTopicDetails td);


  static {
    String propVal = System.getProperty("opendds.native.debug");
    if (propVal != null && ("1".equalsIgnoreCase(propVal) ||
        "y".equalsIgnoreCase(propVal) ||
        "yes".equalsIgnoreCase(propVal) ||
        "t".equalsIgnoreCase(propVal) ||
        "true".equalsIgnoreCase(propVal)))
      System.loadLibrary("ODDS_CORBActiveServiced");
    else System.loadLibrary("ODDS_CORBActiveService");
  }

}
