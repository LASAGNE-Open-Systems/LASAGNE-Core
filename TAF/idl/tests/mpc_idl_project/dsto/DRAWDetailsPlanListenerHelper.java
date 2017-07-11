package dsto;
public abstract class DRAWDetailsPlanListenerHelper {
  // Any and TypeCode operations not currently implemented
  public static String id() { return "IDL:dsto/DRAWDetailsPlanListener:1.0"; }
  public static DRAWDetailsPlanListener narrow(org.omg.CORBA.Object obj) {
    if (obj == null)
      return null;
    else if (obj instanceof DRAWDetailsPlanListener)
      return (DRAWDetailsPlanListener)obj;
    else if (!obj._is_a (id ()))
      throw new org.omg.CORBA.BAD_PARAM ();
    else
      return native_unarrow(obj);
  }

  public static DRAWDetailsPlanListener unchecked_narrow(org.omg.CORBA.Object obj) {
    if (obj == null)
      return null;
    else if (obj instanceof DRAWDetailsPlanListener)
      return (DRAWDetailsPlanListener)obj;
    else
      return native_unarrow(obj);
  }

  private native static DRAWDetailsPlanListener native_unarrow(org.omg.CORBA.Object obj);
}
