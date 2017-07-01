package dsto;
public abstract class DRAWDetailsEventListenerHelper {
  // Any and TypeCode operations not currently implemented
  public static String id() { return "IDL:dsto/DRAWDetailsEventListener:1.0"; }
  public static DRAWDetailsEventListener narrow(org.omg.CORBA.Object obj) {
    if (obj == null)
      return null;
    else if (obj instanceof DRAWDetailsEventListener)
      return (DRAWDetailsEventListener)obj;
    else if (!obj._is_a (id ()))
      throw new org.omg.CORBA.BAD_PARAM ();
    else
      return native_unarrow(obj);
  }

  public static DRAWDetailsEventListener unchecked_narrow(org.omg.CORBA.Object obj) {
    if (obj == null)
      return null;
    else if (obj instanceof DRAWDetailsEventListener)
      return (DRAWDetailsEventListener)obj;
    else
      return native_unarrow(obj);
  }

  private native static DRAWDetailsEventListener native_unarrow(org.omg.CORBA.Object obj);
}
