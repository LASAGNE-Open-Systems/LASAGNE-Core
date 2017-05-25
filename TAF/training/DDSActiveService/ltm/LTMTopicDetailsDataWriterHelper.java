package ltm;
public abstract class LTMTopicDetailsDataWriterHelper {
  // Any and TypeCode operations not currently implemented
  public static String id() { return "IDL:dst.defence.gov.au/ltm/LTMTopicDetailsDataWriter:1.0"; }
  public static LTMTopicDetailsDataWriter narrow(org.omg.CORBA.Object obj) {
    if (obj == null)
      return null;
    else if (obj instanceof LTMTopicDetailsDataWriter)
      return (LTMTopicDetailsDataWriter)obj;
    else if (!obj._is_a (id ()))
      throw new org.omg.CORBA.BAD_PARAM ();
    else
      return native_unarrow(obj);
  }

  public static LTMTopicDetailsDataWriter unchecked_narrow(org.omg.CORBA.Object obj) {
    if (obj == null)
      return null;
    else if (obj instanceof LTMTopicDetailsDataWriter)
      return (LTMTopicDetailsDataWriter)obj;
    else
      return native_unarrow(obj);
  }

  private native static LTMTopicDetailsDataWriter native_unarrow(org.omg.CORBA.Object obj);

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
