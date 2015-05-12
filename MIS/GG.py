#!/usr/bin/python
import os
from generate_graph import *

#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#------------------------------------------ONLY change the VALUES BELLOW------------------------------------------------------------------------------------------------------------------------------------
#--------------------------------------------------------------------------------------------------------------------------------------------------------

numOfNodes = 8000

graphType = "completeChaos" #the graph is completely random, meaning the degree is not set
#graphType = "dense"  #it will generate a dense graph. not sure if this is dense enough though. so preferably use the degreeBased and set the degree
#graphType = "degreeBased" # if you chose this graph type, make sure to set the degree

degree = 100 #this value won't matter if you are not using the degreeBased graph


#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#----------------------------------------ONLY CHANGE THE VALUES ABOVE---------------------------------------------------------------------------------------------------------
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#--------------------------------------------------------------------------------------------------------------------------------------------------------

#---------guide::: making the directory to place the graphs in, if they don't exist
baseDirectory = "./sparse_graph_dir"
if not(os.path.isdir(baseDirectory)):
    os.system("mkdir " + baseDirectory)



#---------guide::: making the file name
if(graphType == "degreeBased"):
    sparseRepFileName = baseDirectory + "/"+"sparseRep_"+str(numOfNodes)+"_"+str(degree)
else:
    sparseRepFileName = baseDirectory+ "/"+"sparseRep_"+str(numOfNodes)+"_"+"xx"  #xx means that this is not a degree based

#---------guide::: generating the graph
counter = 0 #this varaible is just there to keep the while going if while failes
while not(generate_random_graphs(numOfNodes, sparseRepFileName, graphType, int(degree))):
    counter +=1
