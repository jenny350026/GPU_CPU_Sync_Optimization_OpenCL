#!/bin/bash

export AMDAPPSDKROOT="/opt/AMDAPPSDK-3.0-0-Beta"
export LD_LIBRARY_PATH="/opt/AMDAPPSDK-3.0-0-Beta/lib/x86_64/"

echo "** Removing previous MIS_sync.o **"
if [ -f MIS_sync.o ] ; then rm MIS_sync.o ; fi

echo "** Removing previous MIS_sync2.o **"
if [ -f MIS_sync2.o ] ; then rm MIS_sync2.o ; fi

echo "g++ -o MIS_sync.o -c MIS_sync.cpp  debug_helpers.cpp -I$AMDAPPSDKROOT/include"
g++ -std=c++11 -o MIS_sync.o -c MIS_sync.cpp   -I ./ -I$AMDAPPSDKROOT/include 
g++ -std=c++11 -c debug_helpers.cpp  -o MIS_sync2.o -I ./ -I$AMDAPPSDKROOT/include 

echo "** Removing previous executable MIS_sync **" 
if [ -f MIS_sync ] ; then rm MIS_sync ; fi
echo "g++ -o MIS_sync MIS_sync.o -L$AMDAPPSDKROOT/lib/x86_64"
g++ -std=c++11 -o MIS_sync MIS_sync.o  MIS_sync2.o -lOpenCL -L$AMDAPPSDKROOT/lib/x86_64
