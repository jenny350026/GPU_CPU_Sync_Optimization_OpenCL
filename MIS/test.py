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

numOfNodes = 8000 
numOfTests = 1 

#graphType = "completeChaos" #the graph is completely random, meaning the degree is not set
graphType = "dense"  #it will generate a dense graph. not sure if this is dense enough though. so preferably use the degreeBased and set the degree
#graphType = "degreeBased" # if you chose this graph type, make sure to set the degree
degree = 20 


#---------guide::: priming info
doPrime = True
primeNumber = 3
primeFull = False


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
#result = []
#for i in range(0, numOfNodes, 2000):
#    primeNum = i;
#    totalTime = testAll(testName, algorithm, generateGraph, sparseRepFileName, numOfNodes, numOfTests, graphType, degree, doPrime, primeNum, primeFull)
#    result += [totalTime]
#print result


testAll(testName, algorithm, generateGraph, sparseRepFileName, numOfNodes, numOfTests, graphType, degree, doPrime, primeNum, primeFull)

