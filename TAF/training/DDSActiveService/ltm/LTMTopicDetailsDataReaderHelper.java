package ltm;
public abstract class LTMTopicDetailsDataReaderHelper {
  // Any and TypeCode operations not currently implemented
  public static String id() { return "IDL:dst.defence.gov.au/ltm/LTMTopicDetailsDataReader:1.0"; }
  public static LTMTopicDetailsDataReader narrow(org.omg.CORBA.Object obj) {
    if (obj == null)
      return null;
    else if (obj instanceof LTMTopicDetailsDataReader)
      return (LTMTopicDetailsDataReader)obj;
    else if (!obj._is_a (id ()))
      throw new org.omg.CORBA.BAD_PARAM ();
    else
      return native_unarrow(obj);
  }

  public static LTMTopicDetailsDataReader unchecked_narrow(org.omg.CORBA.Object obj) {
    if (obj == null)
      return null;
    else if (obj instanceof LTMTopicDetailsDataReader)
      return (LTMTopicDetailsDataReader)obj;
    else
      return native_unarrow(obj);
  }

  private native static LTMTopicDetailsDataReader native_unarrow(org.omg.CORBA.Object obj);

  static {
    String propVal = System.getProperty("opendds.native.debug");
    if (propVal != null && ("1".equalsIgnoreCase(propVal) ||
        "y".equalsIgnoreCase(propVal) ||
        "yes".equalsIgnoreCase(propVal) ||
        "t".equalsIgnoreCase(propVal) ||
        "true".equalsIgnoreCase(propVal)))
      System.loadLibrary("ODDS_DDSActiveServiced");
    else System.loadLibrary("ODDS_DDSActiveService");
  }

}
