#!/usr/bin/python
from test_platform import *

#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#------------------------------------------ONLY change the VALUES BELLOW------------------------------------------------------------------------------------------------------------------------------------
#--------------<F12>----------------------------------------------------------------------------------------------------------------------------------------------------------------

#---------guide::: test type
testName = "runSomeSampleTests"
#testName = "runTillFailure" 

#---------guide::: algorithm type
#algorithm = "serial"
#algorithm = "asyncParallel"
#algorithm = "syncParallel"
algorithm = "splitThread"
#algorithm = "all" #includes serial, synchronous parallel, and asynchrnous parallel, more might be added later

#---------guide::: generated graph parameters 
generateGraph = False
#sparseRepFileName = "../../exotic_graphs/nlpkkt120.graph" #sparse representation of the graph
#sparseRepFileName = "../../exotic_graphs/af_shell9.graph" #sparse representation of the graph
#sparseRepFileName = "../../original_inputfiles/af_shell9.graph" #sparse representation of the graph
sparseRepFileName = "../../original_inputfiles/ecology1.graph" #sparse representation of the graph
#sparseRepFileName = "../../original_inputfiles/cage15.graph.5154859" #sparse representation of the graph

numOfNodes = 1000000
numOfTests = 1 

#graphType = "completeChaos" #the graph is completely random, meaning the degree is not set
#graphType = "dense"  #it will generate a dense graph. not sure if this is dense enough though. so preferably use the degreeBased and set the degree
graphType = "degreeBased" # if you chose this graph type, make sure to set the degree
degree = 20


#---------guide::: priming info
doPrime = False
primeNumber = 1000000
primeFull = True


#---------guide::: sweepPrime or only test once
sweepPrime = False #if this is set to true, then we sweepPrime the space with different prime numbers between primeNumLowBound and HighBound given bellow
#set the following variables if sweepPrime is through
primeNumLowBound =  5050000
primeNumHighBound = 5154859
primeStepSize =   10000

#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#---------guide::: sweepSplit


doSplit = True
splitNumber = 3

sweepSplit = False 
splitNumLowBound = 2
splitNumHighBound = 10
splitStepSize = 4 


#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#----------------------------------------ONLY CHANGE THE VALUES ABOVE---------------------------------------------------------------------------------------------------------
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#---------guide::: setting the variables appropirately based on the previous information
primeNum = 0
if (doPrime):
    if (primeFull):
        primeNum = numOfNodes;
    else:
        primeNum = primeNumber
else:
    primeNum = 0


splitNum = 0
if (doSplit):
    splitNum = splitNumber;
else:
    splitNum = 0


if (algorithm == "asyncParallel" and (doSplit or sweepSplit)):
    print "you can not run asyncparallel with doSplit or sweepPrim, pick splitThread"
    exit()
if (algorithm == "splitThread" and (doPrime or sweepPrime)):
    print "you can not run asyncparallel with doSplit or sweepPrim, pick splitThread"
    exit()


if (doPrime and doSplit):
    print "****ERROR: you can only either pick sweepPrim or sweepSplit"
    exit()

if not(doPrime):
    if(sweepPrime):
        print "****ERROR: you can not prime but sweep prime. double check your parameters and set doPrime to True if you want to sweep prime, otherwise set sweepPrim to False"
        exit()
if not(doSplit):
    if(sweepSplit):
        print "***ERROR: you can not split but sweep split. double check your parameters and set doSplit to True if you want to sweep split, otherwise set sweepSplit to False"
        exit()





#degree = 0
#if (graphType == "degreeBased"):
#    if (graphDegree > numOfNodes/1.5):
#        print "degree is too hight"
#        exit()
#    else: 
#        degree = graphDegree
#

if (generateGraph):
    sparseRepFileName = "sparse_rep.txt"





#---------guide::: testing everything

if (sweepPrime or sweepSplit):
    timeRandtimeKernel, timeRand, timeKernel, totalTimeKernel, totalTimeRand, totalTimesAdded = testSweep(testName, algorithm, generateGraph, sparseRepFileName, numOfNodes, numOfTests, graphType, degree, doPrime, primeNumLowBound,primeNumHighBound, primeStepSize, primeFull, splitNumLowBound, splitNumHighBound, doSplit, splitStepSize)
    print "timeRand is" + str(timeRand)
    print "timeKernel is" + str(timeKernel)
    print "timeRandtimeKernel is " + str(timeRandtimeKernel)
    print "totalTimeKernel is " + str(totalTimeKernel)
    print "totalTimeRand is " + str(totalTimeRand)
    print "total Times Added is " + str(totalTimesAdded)
else:
    timeRandtimeKernel, timeRand, timeKernel, totalTimeKernel, totalTimeRand ,totalTimesAdded =  testOnce(testName, algorithm, generateGraph, sparseRepFileName, numOfNodes, numOfTests, graphType, degree, doPrime, primeNum, primeFull, doSplit, splitNum)

    print "timeRand is" + str(timeRand)
    print "timeKernel is" + str(timeKernel)
    print "timeRandtimeKernel is " + str(timeRandtimeKernel)
    print "totalTimeKernel is " + str(totalTimeKernel)
    print "totalTimeKernel is " + str(totalTimeKernel)
    print "totalTimeRand is " + str(totalTimeRand)
    print "total Times Added is " + str(totalTimesAdded)

