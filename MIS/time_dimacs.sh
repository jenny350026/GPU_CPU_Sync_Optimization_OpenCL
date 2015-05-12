#!/bin/bash

if [ -d "$time" ]; then rm -r time/; fi
mkdir time;

if [ $USER == 'eryilmaz' ] ;
then
./MIS_sync /home/$USER/original_inputfiles/af_shell9.graph out log > ./time/af_shell9_output;
./MIS_sync /home/$USER/original_inputfiles/ecology1.graph out log > ./time/ecology1_output;
./MIS_sync /home/$USER/original_inputfiles/nlpkkt120.graph out log > ./time/nlpkkt120_output;
fi

if [ $USER == 'jenny' ] ;
then
./MIS_sync /home/$USER/original_inputfiles/af_shell9.graph out log > ./time/af_shell9_output;
./MIS_sync /home/$USER/original_inputfiles/ecology1.graph out log > ./time/ecology1_output;
./MIS_sync /home/$USER/original_inputfiles/nlpkkt120.graph out log > ./time/nlpkkt120_output;
fi

if [ $USER == 'zaddan' ] ;
then
./MIS_sync /home/$USER/Desktop/original_inputfiles/af_shell9.graph out log > ./time/af_shell9_output;
./MIS_sync /home/$USER/Desktop/original_inputfiles/ecology1.graph out log > ./time/ecology1_output;
./MIS_sync /home/$USER/Desktop/original_inputfiles/nlpkkt120.graph out log > ./time/nlpkkt120_output;
fi
