/***************************************************************
    Copyright 2016, 2017 Defence Science and Technology Group,
    Department of Defence,
    Australian Government

	This file is part of LASAGNE.

    LASAGNE is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation, either version 3
    of the License, or (at your option) any later version.

    LASAGNE is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with LASAGNE.  If not, see <http://www.gnu.org/licenses/>.
***************************************************************/

import TAF_XMPL.SimpleServer;
import TAF_XMPL.SimpleServerHelper;
import TAF_XMPL.SimpleCallback;
import TAF_XMPL.SimpleCallbackHelper;
import org.omg.CORBA.*;

public class TestClient
{
	static int locate_retry = 1;

	public static SimpleServer locate_SimpleServer(org.omg.CORBA.ORB orb)
	{
	    SimpleServer objRef;

	    for(;;) {
	    	try {

	    		org.omg.CORBA.Object server_proxy = orb.resolve_initial_references(TAF_XMPL.MY_TESTSERVER_OID.value);
	    		if ((objRef = SimpleServerHelper.narrow(server_proxy)) == null) {
		    		System.out.println("CLIENT: SimpleServer Object reference is invalid - Retry(" + locate_retry++ + ").");
 	    		} else break;

	    	} catch (Exception ex) {
	    	}
	    	try { Thread.sleep(2000); } catch (Exception ex) {}
	    }
	    return objRef;
	}

    public static void main(String args[])
    {
    	try {

    		org.omg.CORBA.ORB orb = org.omg.CORBA.ORB.init(args, null);
    		org.omg.PortableServer.POA rootPOA = org.omg.PortableServer.POAHelper.narrow(orb.resolve_initial_references("RootPOA"));

    		rootPOA.the_POAManager().activate();

            SimpleCallback_impl callback_impl = new SimpleCallback_impl();

            byte oid[] = rootPOA.activate_object(callback_impl);

            org.omg.CORBA.Object cb_proxy = rootPOA.id_to_reference(oid);

            SimpleCallback cb_object = SimpleCallbackHelper.narrow(cb_proxy);

            String cb_ref_ior = orb.object_to_string(cb_object);

            System.out.println("CLIENT:Callback Activated as:\n  <" + cb_ref_ior + ">");

            SimpleServer server_objRef = null;

            for (int i = 0;; i++) {

            	try {

            		while (server_objRef == null) {
	            		server_objRef = locate_SimpleServer(orb);
	            	}

 	                System.out.println("\n********* CLIENT::LOOP START [" + i + "] ************************************");

					server_objRef.test_val(i);

					int val = server_objRef.test_val();

					System.out.println("<-> " + val + " == CLIENT::test_val(" + i + "):");

	 				TAF_XMPL.StructureHolder out_holder	= new TAF_XMPL.StructureHolder(new TAF_XMPL.Structure((short)i, 0, ""));

	                TAF_XMPL.Structure in_struct = new TAF_XMPL.Structure((short)i,0,"");

	                StringHolder name = new StringHolder("My name is DEREK!");

	                long r = server_objRef.test_method(i, in_struct, out_holder, name);

	                System.out.println("<->" + r + " == CLIENT::test_method():<- out_struct->i = " + out_holder.value.i + ", name = <" + name.value + ">");

	                System.out.println("--> CLIENT::test_callback():");
	                server_objRef.test_callback(cb_object);

	                System.out.println("--> CLIENT::test_oneway():");
	                server_objRef.test_oneway("Test ONEWAY String!");

	                try {
	                    server_objRef.raise_system_exception();
	                } catch (org.omg.CORBA.NO_PERMISSION ex) {
	                    System.out.println("<-> CLIENT::expected CORBA System Exception caught!");
	                }
	                try {
	                    server_objRef.raise_user_exception();
	                } catch (TAF_XMPL.test_exception ex) {
	                	System.out.println("<-> CLIENT::expected User Exception caught!");
	                }

            	} catch(Exception e) {
            		e.printStackTrace(); server_objRef = null;
            	}

            	Thread.sleep(100);

            }

    	} catch (Exception e) {
    		e.printStackTrace();
        }
    }

}
