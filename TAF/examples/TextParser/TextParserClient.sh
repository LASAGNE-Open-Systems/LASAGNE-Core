#!/bin/bash



BASE_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

$BASE_DIR/TextParserClient -z -w 5 -ORBInitRef TAF_XMPL_TextParserService=corbaloc:iiop:localhost:8989/TAF_XMPL_TextParserService -ORBObjRefStyle URL
 