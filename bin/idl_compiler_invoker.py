'''
Created on 19 Nov 2015

@author: pinkm
'''

import os
from optparse import OptionParser
import sys
from subprocess import call


# Main function.
def main():
       
    parser = OptionParser(usage="usage: %prog [options]",
                          version="%prog 1.0")
    parser.add_option("-l", "--idl_location",
                      dest="idl_location_var",
                      help="The location of the IDL")
    (options, args) = parser.parse_args()
    
    # Exit if the correct options are not given.
    if options.idl_location_var == None: 
        parser.print_help()
        sys.exit()
        
    idl_location = ""    
    if options.idl_location_var != "":
        idl_location = options.idl_location_var
    
    # Create a var for the idl path/exe.
    idl_compiler_path = os.environ['DAF_ROOT'] + "/bin/lasagne_idl.exe"
        
    # Iterate through the given folder and generate run the compiler on each IDL file.
    for root, dirs, files in os.walk(idl_location, topdown=True):
        for name in files:
            # Process all IDL but not the TypeSupport IDL files if they have already been generated.
            if name.endswith(".idl") == True and name.endswith("TypeSupport.idl") == False:
                
                # Run the compiler on the given file.
                call([idl_compiler_path, os.path.join(root, name)])
        
        # Breaking here since we don't want to process sub-directories. Or.. Do we?
        break

# Invoke main function.
if __name__ == '__main__':
    main()