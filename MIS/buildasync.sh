#!/bin/bash

export AMDAPPSDKROOT="/opt/AMDAPPSDK-3.0-0-Beta"
export LD_LIBRARY_PATH="/opt/AMDAPPSDK-3.0-0-Beta/lib/x86_64/"

echo "Removing previous MIS_parallel_async.o"
if [ -f MIS_parallel_async.o ] ; then rm MIS_parallel_async.o ; fi
echo "Removing previous debug_helpers.o"
if [ -f debug_helpers.o ] ; then rm debug_helpers.o ; fi
echo "g++ -o MIS_parallel_async.o -c MIS_parallel_async.cpp  debug_helpers.cpp -I$AMDAPPSDKROOT/include"
g++ -std=c++11 -o MIS_parallel_async.o -c MIS_parallel_async.cpp   -I ./ -I$AMDAPPSDKROOT/include 
g++ -std=c++11 -c debug_helpers.cpp  -o debug_helpers.o -I ./ -I$AMDAPPSDKROOT/include 

echo "Removing previous executable MIS_parallel_async" 
if [ -f MIS_parallel_async ] ; then rm MIS_parallel_async ; fi
echo "g++ -std=c++11 -o MIS_parallel_async MIS_parallel_async.o  debug_helpers.o -lOpenCL -L$AMDAPPSDKROOT/lib/x86_64"
g++ -std=c++11 -o MIS_parallel_async MIS_parallel_async.o  debug_helpers.o -lOpenCL -L$AMDAPPSDKROOT/lib/x86_64
