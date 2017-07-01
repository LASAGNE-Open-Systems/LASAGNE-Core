package ltm;
public abstract class CORBActiveServiceHelper {
  // Any and TypeCode operations not currently implemented
  public static String id() { return "IDL:dst.defence.gov.au/ltm/CORBActiveService:1.0"; }
  public static CORBActiveService narrow(org.omg.CORBA.Object obj) {
    if (obj == null)
      return null;
    else if (obj instanceof CORBActiveService)
      return (CORBActiveService)obj;
    else if (!obj._is_a (id ()))
      throw new org.omg.CORBA.BAD_PARAM ();
    else
      return native_unarrow(obj);
  }

  public static CORBActiveService unchecked_narrow(org.omg.CORBA.Object obj) {
    if (obj == null)
      return null;
    else if (obj instanceof CORBActiveService)
      return (CORBActiveService)obj;
    else
      return native_unarrow(obj);
  }

  private native static CORBActiveService native_unarrow(org.omg.CORBA.Object obj);

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
