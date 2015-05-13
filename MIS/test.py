#!/usr/bin/python
from test_platform import *

#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#------------------------------------------ONLY change the VALUES BELLOW------------------------------------------------------------------------------------------------------------------------------------
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#---------guide::: test type
testName = "runSomeSampleTests"
#testName = "runTillFailure" 

#---------guide::: algorithm type
#algorithm = "serial"
algorithm = "asyncParallel"
#algorithm = "syncParallel"
#algorithm = "all" #includes serial, synchronous parallel, and asynchrnous parallel, more might be added later

#---------guide::: generated graph parameters 
generateGraph = True 
sparseRepFileName = "../../exotic_graphs/nlpkkt120.graph" #sparse representation of the graph

numOfNodes = 80
numOfTests = 1 

#graphType = "completeChaos" #the graph is completely random, meaning the degree is not set
#graphType = "dense"  #it will generate a dense graph. not sure if this is dense enough though. so preferably use the degreeBased and set the degree
graphType = "degreeBased" # if you chose this graph type, make sure to set the degree
degree = 20


#---------guide::: priming info
doPrime = True
primeNumber = 3
primeFull = False


#---------guide::: sweep or only test once
sweep = True #if this is set to true, then we sweep the space with different prime numbers between primeNumLowBound and HighBound given bellow
#sweep = False #if false, runs the test only once
#set the following variables if sweep is through
primeNumLowBound = 20
primeNumHighBound = numOfNodes
primeStepSize =   20 




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
if (sweep):
    timeRand, timeKernel, totalTimeKernel, totalTimeRand = testSweep(testName, algorithm, generateGraph, sparseRepFileName, numOfNodes, numOfTests, graphType, degree, doPrime, primeNumLowBound,primeNumHighBound, primeStepSize, primeFull)
    print "timeRand is" + str(timeRand)
    print "timeKernel is" + str(timeKernel)
    print "totalTimeKernel is " + str(totalTimeKernel)
    print "totalTimeRand is " + str(totalTimeRand)
else:
    timeRand, timeKernel, totalTimeKernel, totalTimeRand =  testOnce(testName, algorithm, generateGraph, sparseRepFileName, numOfNodes, numOfTests, graphType, degree, doPrime, primeNum, primeFull)
    print "timeRand is" + str(timeRand)
    print "timeKernel is" + str(timeKernel)
    print "totalTimeKernel is " + str(totalTimeKernel)
    print "totalTimeRand is " + str(totalTimeRand)


