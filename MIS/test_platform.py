#!/usr/bin/python
import os
from book_keeping import *
from generate_graph import *
from verify import *
from time import gmtime, strftime
from generate_graph import *


#-------------------------------------------------------------------------------------------------------------------------------------------------------------
#-------------------------------------------------------------------------------------------------------------------------------------------------------------
#-------------------------------------------------------------------------------------------------------------------------------------------------------------



#-------------------------------------------------------------------------------------------------------------------------------------------------------------
#-------------------------------------------------------------------------------------------------------------------------------------------------------------
#-------------------------------------------------------------------------------------------------------------------------------------------------------------

#-------------------------------------------------------------------------------------------------------------------------------------------------------------
#variables
MISResultToVerifyFileNameSerial = "MIS_result_serial.txt" #generated by the serail implementation written by us
MISResultToVerifyFileNameParallel = "MIS_result_parallel.txt" #generated by the parralel implementation written by us

#running ./buildrun once to make sure that we have the .h files
#os.system("./buildrun.sh")

#MISResultRefFileName = "MISResultRef.txt" #generated by the reference serial implementation found in interner
#adjacentRepFilName = "adjacent_rep.txt" #adjacent matrix representation of the graph

#------------------------------------------------------------------------------------------------------------------------------------------------
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#---------guide::: test the serial code 
#------------------------------------------------------------------------------------------------------------------------------------------------
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
def writeSparse(logFileName ,sparseRepFileName):
    logFilePtr = open(logFileName, "a");
    print logFileName 
    sparseRepFilePtr= open(sparseRepFileName, "r");
    #---------guide:::  writing the sparse graph in the log file
    logFilePtr.write("\n");
    logFilePtr.write("*************************sparse representation of the graph************************************\n")
    logFilePtr.write("sparseMatrixStart \n")
    logFilePtr.write(sparseRepFilePtr.read());
    logFilePtr.write("sparseMatrixEnd\n")
	







#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#---------guide::: test the synchrnous parallel code
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
def runSyncParallel(sparseRepFileName, MISResultToVerifyFileNameParallel, logFileName, logFileNameFailedCase):
    os.system("./buildsync.sh")
    os.system("./MIS_sync" + " " + sparseRepFileName + " " + MISResultToVerifyFileNameParallel + " " + logFileName); 
    logFilePtr = open(logFileName, "a");
    logFilePtr.write("\n");
    logFilePtr.write("\n");
    logFilePtr.write("*************************Result of the syncParralel implemetations*********************************\n")
    MISResultToVerifyFileParallelPtr = open(MISResultToVerifyFileNameParallel, "r");
    logFilePtr.write(MISResultToVerifyFileParallelPtr.read());
    #   ##---------guide::: verify
    print "verifying the synchparallel exection...."
    logFilePtr.write("***********************************************************************************************\n")
    logFilePtr.write("*************************syncParralel Test Results: ***********************************************\n")
        
    #logFilePtr.write("*******************************************************************************************************************\n")
    logFilePtr.close() 
    
    return verifyMaximalSetCVersion(sparseRepFileName,  MISResultToVerifyFileNameParallel, logFileName)
    #return verifyMaximalSet(sparseRepFileName,  MISResultToVerifyFileNameParallel, logFileName)




#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#---------guide::: test the synchrnous parallel code
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
def runAsyncParallel(sparseRepFileName, MISResultToVerifyFileNameSerial, logFileName, logFileNameFailedCasek, primeNum):
    os.system("./buildasync.sh")
    os.system("./MIS_parallel_async" + " " + sparseRepFileName + " " + MISResultToVerifyFileNameParallel + " " + logFileName + " " + str(primeNum)); 
    logFilePtr = open(logFileName, "a");
    logFilePtr.write("\n");
    logFilePtr.write("\n");
    logFilePtr.write("*************************Result of the AsyncParralel implemetations*********************************\n")
    MISResultToVerifyFileParallelPtr = open(MISResultToVerifyFileNameParallel, "r");
    logFilePtr.write(MISResultToVerifyFileParallelPtr.read());
    #   ##---------guide::: verify
    print "verifying the Asyncparallel exection...."
    logFilePtr.write("***********************************************************************************************\n")
    logFilePtr.write("*************************AsyncParralel Test Results: ***********************************************\n")
    
    #logFilePtr.write("*******************************************************************************************************************\n")
    logFilePtr.close() 
    
    return verifyMaximalSetCVersion(sparseRepFileName,  MISResultToVerifyFileNameParallel, logFileName)
    #return verifyMaximalSet(sparseRepFileName,  MISResultToVerifyFileNameParallel, logFileName)
       
##------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
##---------guide::: test the serial code 
#def runSerial(sparseRepFileName, MISResultToVerifyFileNameSerial, logFileName, logFileNameFailedCase):
#    #---------guide::: run the serial code
#    os.system("g++ MIS_serial.cpp debug_helpers.cpp -o MIS_serial")
#    os.system("./MIS_serial" + " " + sparseRepFileName + " " + MISResultToVerifyFileNameSerial+ " " + logFileName); 
#    #
#    #os.remove(logFileName) 
#    logFilePtr = open(logFileName, "a");
#    logFilePtr.write("\n");
#    logFilePtr.write("\n");
#    logFilePtr.write("*************************Result of the serial implemetation************************************\n")
#    MISResultToVerifyFileSerialPtr = open(MISResultToVerifyFileNameSerial, "r");
#    logFilePtr.write(MISResultToVerifyFileSerialPtr.read());
#    #---------guide::: verify
#    print "verifying the serial exection...."
#    logFilePtr.write("***********************************************************************************************\n")
#    logFilePtr.write("*************************Serial Test Results: *************************************************\n")
#    logFilePtr.close() 
#    return verifyMaximalSet(sparseRepFileName,  MISResultToVerifyFileNameSerial, logFileName)
# 

#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#---------guide::: test the serial code 
def runSerial(sparseRepFileName, MISResultToVerifyFileNameSerial, logFileName, logFileNameFailedCase):
    #---------guide::: run the serial code
    os.system("g++ MIS_serial.cpp debug_helpers.cpp -o MIS_serial")
    os.system("./MIS_serial" + " " + sparseRepFileName + " " + MISResultToVerifyFileNameSerial+ " " + logFileName); 
    #
    #os.remove(logFileName) 
    logFilePtr = open(logFileName, "a");
    logFilePtr.write("\n");
    logFilePtr.write("\n");
    logFilePtr.write("*************************Result of the serial implemetation************************************\n")
    MISResultToVerifyFileSerialPtr = open(MISResultToVerifyFileNameSerial, "r");
    logFilePtr.write(MISResultToVerifyFileSerialPtr.read());
    #---------guide::: verify
    print "verifying the serial exection...."
    logFilePtr.write("***********************************************************************************************\n")
    logFilePtr.write("*************************Serial Test Results: *************************************************\n")
    logFilePtr.close() 
    return verifyMaximalSet(sparseRepFileName,  MISResultToVerifyFileNameSerial, logFileName)
       

#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#generates a log file name based on the time stamp
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
def generateLogFileName():
    #---------guide:::  generate a log file and append the spares rep nad resultto it
    if not(os.path.isdir("./log")):
        os.system("mkdir log");
    timeSuffix = strftime("%M", gmtime()) + strftime("%S", gmtime()) +".txt"
    logFileName = "./log/log_" + timeSuffix
    if not(os.path.isdir("./log/failed")):
        os.system("mkdir log/failed");
    logFileNameFailedCase= "./log/failed/log_" + timeSuffix 
     
    logFilePtr = open(logFileName, "a");
    logFilePtr.close() 
    return logFileName, logFileNameFailedCase


#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
print "hello"
#---------guide::: tests the all code 
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

def runAll(sparseRepFileName, MISResultToVerifyFileNameSerial, logFileName, logFileNameFailedCase):
    #---------guide:::  run serial test
    failed1 = runSerial(sparseRepFileName, MISResultToVerifyFileNameSerial, logFileName, logFileNameFailedCase)
    #---------guide:::  run parallel test
    failed3 = runAsyncParallel(sparseRepFileName, MISResultToVerifyFileNameSerial, logFileName, logFileNameFailedCase, primeNum)
    return failed1 or failed2 or failed3
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#---------guide::: testing everything
def testAll(testName, algorithm, generateGraph, sparseRepFileName, numOfNodes, numOfTests, graphType, degree, doPrime, primeNum, primeFull):
	if (testName == "runSomeSampleTests"):
	    counter  = 0
	    for i in range (0, numOfTests):
	        #generate a graph       
	        if (generateGraph) :
	            while not(generate_random_graphs(numOfNodes, sparseRepFileName, graphType, int(degree))):
	                counter +=1; #this is just so that while has a body, but the point is that we need to run the generate random_graph till the result is 1
	        logFileName, logFileNameFailedCase = generateLogFileName()    
	        writeSparse(logFileName, sparseRepFileName) 
	#        
	        if (algorithm == "serial"): 
	            #---------guide:::  run serial test
	           runSerial(sparseRepFileName, MISResultToVerifyFileNameSerial, logFileName, logFileNameFailedCase) 
	        elif (algorithm == "syncParallel"): 
	            #---------guide:::  run parallel
	            runSyncParallel(sparseRepFileName, MISResultToVerifyFileNameSerial, logFileName, logFileNameFailedCase) 
	        elif (algorithm == "asyncParallel"): 
	            runAsyncParallel(sparseRepFileName, MISResultToVerifyFileNameSerial, logFileName, logFileNameFailedCase, primeNum)
	        elif(algorithm == "all"):
	            runAll(sparseRepFileName, MISResultToVerifyFileNameSerial, logFileName, logFileNameFailedCase) 
	        else:
	            print "this algorithm is not acceptable" 
	            exit()
	

	if (testName == "runTillFailure"): 
	    failed = 0
	    counter  = 0
	    while (not (failed)): 
	        #generate a graph       
	        if (generateGraph) :
	            while not(generate_random_graphs(numOfNodes, sparseRepFileName, graphType, degree)):
	                counter +=1; #this is just so that while has a body, but the point is that we need to run the generate random_graph till the result is 1
	         
	        logFileName, logFileNameFailedCase = generateLogFileName()    
	        writeSparse(logFileName, sparseRepFileName) 
	        if (algorithm == "serial"): 
	            #---------guide:::  run serial test
	           failed = runSerial(sparseRepFileName, MISResultToVerifyFileNameSerial, logFileName, logFileNameFailedCase) 
	        elif (algorithm == "syncParallel"): 
	            #---------guide:::  run parallel
	            failed = runSyncParallel(sparseRepFileName, MISResultToVerifyFileNameSerial, logFileName, logFileNameFailedCase) 
	        elif(algorithm == "all"):
	            failed = runAll(sparseRepFileName, MISResultToVerifyFileNameSerial, logFileName, logFileNameFailedCase) 
	
	        elif (algorithm == "asyncParallel"): 
	            failed = runAsyncParallel(sparseRepFileName, MISResultToVerifyFileNameSerial, logFileName, logFileNameFailedCase)
	        else:
	            print "this algorithm is not acceptable" 
	            exit()
	
	        if (failed):
	            logFileFailedCasePtr = open(logFileNameFailedCase, "a")
	            logFilePtr = open(logFileName, "r")
	            logFileFailedCasePtr.write(logFilePtr.read());
	            logFilePtr.close()
	            logFileFailedCasePtr.close(); 
	            print "right here" 
	            drawGraphDebug(logFileName)
	#        else:
	#            os.system("rm -r ./log")
	#            os.system("rm -r ./log")
	#





