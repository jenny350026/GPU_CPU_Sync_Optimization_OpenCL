#!/bin/bash

export AMDAPPSDKROOT="/opt/AMDAPPSDK-3.0-0-Beta"
export LD_LIBRARY_PATH="/opt/AMDAPPSDK-3.0-0-Beta/lib/x86_64/"

echo "Removing previous MIS_sync.o"
if [ -f MIS_sync.o ] ; then rm MIS_sync.o ; fi
echo "g++ -o MIS_sync.o -c MIS_sync.cpp -I$AMDAPPSDKROOT/include"
g++ -o MIS_sync.o -c MIS_sync.cpp -I ./ -I$AMDAPPSDKROOT/include 

echo "Removing previous executable MIS_sync" 
if [ -f MIS_sync ] ; then rm MIS_sync ; fi
echo "g++ -o MIS_sync MIS_sync.o -L$AMDAPPSDKROOT/lib/x86_64"
g++ -o MIS_sync MIS_sync.o -lOpenCL -L$AMDAPPSDKROOT/lib/x86_64
