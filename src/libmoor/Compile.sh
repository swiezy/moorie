#!/bin/bash
gcc -shared -Wall Decoder.cpp LibMoor.cpp MoorhuntHash.cpp -o libmoor.so -lboost_regex -lmhash -lmcrypt -lssl
#gcc -shared -I/usr/include/gmime-2.0 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include  Decoder.cpp LibMoor.cpp MoorhuntHash.cpp Util.cpp  -o libmoor.so -lboost_regex -lmhash -lmcrypt -lgmime-2.0 -lz -lnsl -lgobject-2.0 -lglib-2.0 -lssl