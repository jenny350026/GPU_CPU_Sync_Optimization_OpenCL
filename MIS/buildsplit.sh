#!/bin/bash

export AMDAPPSDKROOT="/opt/AMDAPPSDK-3.0-0-Beta"
export LD_LIBRARY_PATH="/opt/AMDAPPSDK-3.0-0-Beta/lib/x86_64/"

echo "Removing previous MIS_split_thread.o"
if [ -f MIS_split_thread.o ] ; then rm MIS_split_thread.o ; fi
echo "Removing previous debug_helpers.o"
if [ -f debug_helpers.o ] ; then rm debug_helpers.o ; fi
echo "g++ -o MIS_split_thread.o -c MIS_split_thread.cpp  debug_helpers.cpp -I$AMDAPPSDKROOT/include"
g++ -std=c++11 -o MIS_split_thread.o -c MIS_split_thread.cpp   -I ./ -I$AMDAPPSDKROOT/include 
g++ -std=c++11 -c debug_helpers.cpp  -o debug_helpers.o -I ./ -I$AMDAPPSDKROOT/include 

echo "Removing previous executable MIS_split_thread" 
if [ -f MIS_split_thread ] ; then rm MIS_split_thread ; fi
echo "g++ -std=c++11 -o MIS_split_thread MIS_split_thread.o  debug_helpers.o -lOpenCL -L$AMDAPPSDKROOT/lib/x86_64"
g++ -g -std=c++11 -o MIS_split_thread MIS_split_thread.o  debug_helpers.o -lOpenCL -L$AMDAPPSDKROOT/lib/x86_64
