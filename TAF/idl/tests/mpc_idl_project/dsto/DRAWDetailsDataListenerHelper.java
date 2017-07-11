package dsto;
public abstract class DRAWDetailsDataListenerHelper {
  // Any and TypeCode operations not currently implemented
  public static String id() { return "IDL:dsto/DRAWDetailsDataListener:1.0"; }
  public static DRAWDetailsDataListener narrow(org.omg.CORBA.Object obj) {
    if (obj == null)
      return null;
    else if (obj instanceof DRAWDetailsDataListener)
      return (DRAWDetailsDataListener)obj;
    else if (!obj._is_a (id ()))
      throw new org.omg.CORBA.BAD_PARAM ();
    else
      return native_unarrow(obj);
  }

  public static DRAWDetailsDataListener unchecked_narrow(org.omg.CORBA.Object obj) {
    if (obj == null)
      return null;
    else if (obj instanceof DRAWDetailsDataListener)
      return (DRAWDetailsDataListener)obj;
    else
      return native_unarrow(obj);
  }

  private native static DRAWDetailsDataListener native_unarrow(org.omg.CORBA.Object obj);
}
