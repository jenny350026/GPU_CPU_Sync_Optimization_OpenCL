#include <string.h>
#include <stdlib.h>
#include <iostream>
using namespace std;
//#include "MIS_parallel.h"
#include <fstream>

#include <vector>
#include <cstdlib>
#include <ctime>
#include <sstream>

#include "status.h"
#include <iostream>

//-----------------------------------------------------------------------------------------------------------------------------
////-----------------------------------------------------------------------------------------------------------------------------
////---------module_name:::showNodesStatus
////---------functionlity:::shows the status of the nodes
////-----------------------------------------------------------------------------------------------------------------------------
////-----------------------------------------------------------------------------------------------------------------------------
void showNodesInfo(int *nodesStatus, float *nodeRandValues, int nodeArraySize, string request){
    //this veriable determines whether we want tow show the complete name of the status or just numbers 
    bool showFullStatusName = true; 
    //change the following if status.h changed 
    //show status 
    if (request == "status") {
        cout<<"***********************************************";
        cout<<"showing all the nodes status was requests"<<endl; 
        string nodeStatusString[3] = {"INACTIVE", "ACTIVE", "SELECTED"};
        for (int i = 0; i< nodeArraySize; i++){
            if(showFullStatusName) {
                cout<<"node "<< i  + 1<< "th status is: " << nodeStatusString[nodesStatus[i]]<<endl;
            }else{
                cout<<"node "<< i + 1 << "th status is: " << nodesStatus[i]<<endl;
            }
        }
        cout<<"***********************************************"; 
    }
    //show rand values 
    else if (request == "randValues") {
        cout<<"***********************************************"; 
        cout<<"showing all the nodes rand values was requests"<<endl; 
        for (int i = 0; i< nodeArraySize; i++){
            cout<<"node "<< i  + 1<< "th status is: " << nodeRandValues[i]<<endl;
        }
        cout<<"***********************************************"; 
    } 
    //show all the info 
    else if (request == "all") {
        //showNodesInfo(nodesStatus, nodeRandValues, nodeArraySize, "status");
        //showNodesInfo(nodesStatus, nodeRandValues, nodeArraySize, "randValues");
        cout<<"***********************************************";
        cout<<"showing all the nodes status was requests"<<endl; 
        string nodeStatusString[3] = {"INACTIVE", "ACTIVE", "SELECTED"};
        for (int i = 0; i< nodeArraySize; i++){
            cout <<"node "<< i + 1 << ":\t" << nodeStatusString[nodesStatus[i]] << "\t" << nodeRandValues[i] << "\t" << nodesStatus[i] <<  endl;
        }
        cout<<"***********************************************"; 
    } 
    else {
        cout<< "this information is not applicable"<<endl;
    }
}

//-----------------------------------------------------------------------------------------------------------------------------
////-----------------------------------------------------------------------------------------------------------------------------
////---------module_name::: writeToFileInfo
////---------functionlity::: writes the status of the nodes to the file
////-----------------------------------------------------------------------------------------------------------------------------
////-----------------------------------------------------------------------------------------------------------------------------
void writeToFileNodeInfo(int *nodesStatus, float *nodeRandValues, int nodeArraySize, string fileName, string request){
    //open the file
    ofstream myfile;
    myfile.open (fileName.c_str(), std::ios_base::app);
    //this veriable determines whether we want to show the complete name of the status or just numbers 
    bool showFullStatusName = true; 

    //change the following if status.h changed 
    //show status 
    if (request == "status") {
        myfile<<"***********************************************"<<endl;
        myfile<<"showing all the nodes status was requests"<<endl; 
        string nodeStatusString[3] = {"INACTIVE", "ACTIVE", "SELECTED"};
        for (int i = 0; i< nodeArraySize; i++){
            if(showFullStatusName) {
                myfile<<"node "<< i + 1 << "th status is: " << nodeStatusString[nodesStatus[i]]<<endl;
            }else{
                myfile<<"node "<< i  + 1<< "th status is: " << nodesStatus[i]<<endl;
            }
        }
        myfile<<"***********************************************"<<endl;; 
        myfile<< "status:" << " "; 
        for (int i = 0; i< nodeArraySize; i++){
            if(showFullStatusName) {
                //myfile<<"node "<< i + 1<< "th status is: " << nodeStatusString[nodesStatus[i]]<<endl;
                myfile<< nodeStatusString[nodesStatus[i]]<< " ";
            }
        }
        myfile << endl; 
    }
    //show rand values 
    else if (request == "randValues") {
        myfile<<"***********************************************"<<endl;; 
        myfile<<"showing all the nodes rand values was requests"<<endl; 
        for (int i = 0; i< nodeArraySize; i++){
            //myfile<<"node "<< i + 1<< "th status is: " << nodeStatusString[nodesStatus[i]]<<endl;
            myfile<<"node "<< i + 1<< "rand value is: " << nodeRandValues[i] << endl;
        }
        myfile<<"***********************************************"<<endl; 
        myfile<< "randValues:" << " "; 
        for (int i = 0; i< nodeArraySize; i++){
            //myfile<<"node "<< i + 1<< "th status is: " << nodeStatusString[nodesStatus[i]]<<endl;
            myfile<< nodeRandValues[i] << " ";
        }
        myfile<< endl; 
        myfile<<"***********************************************"<<endl;; 
    } 

    //show all the info 
    //
    else if (request == "all") {
        writeToFileNodeInfo(nodesStatus, nodeRandValues, nodeArraySize, fileName, "status");
        writeToFileNodeInfo(nodesStatus, nodeRandValues, nodeArraySize, fileName, "randValues");
//        cout << "the file name is" << fileName; 
//        string nodeStatusString[3] = {"INACTIVE", "ACTIVE", "SELECTED"};
//        myfile<<"***********************************************"<<endl;; 
//        myfile<<"showing all the nodes rand values was requests"<<endl; 
//        for (int i = 0; i< nodeArraySize; i++){
//            //myfile<<"node "<< i + 1<< "th status is: " << nodeStatusString[nodesStatus[i]]<<endl;
//            myfile<<"node "<< i + 1 << ":\t" << nodeStatusString[nodesStatus[i]] << "\t" << nodeRandValues[i] << "\t" << nodesStatus[i] <<  endl;
//        }
//        myfile<<"***********************************************"<<endl; 
//        myfile<< "randValues:" << " "; 
//        for (int i = 0; i< nodeArraySize; i++){
//            //myfile<<"node "<< i + 1<< "th status is: " << nodeStatusString[nodesStatus[i]]<<endl;
//            myfile<< nodeRandValues[i] << " ";
//        }
//        myfile<< endl; 
//        myfile<<"***********************************************"<<endl;; 
    }else {
        myfile<< "this information is not applicable"<<endl;

    }
    //closing the file 
    myfile.close();
}

/*
    this function is to check if all the nodes with status SELECTED are actually independent
*/

bool check_independence(int *node_array, int *index_array, int *status_array, int numofnodes){
    bool independent = true;

    // true until found a node with its neighbor also SELECTED
    for(int i = 0; i < numofnodes; i++){
        if(status_array[i] == SELECTED){
            int numofneighbors = index_array[i+1] - index_array[i];
            for(int j = 0; j < numofneighbors; j++){
#if DEBUG
                cout << "Node " << i << " neighbor node " << node_array[index_array[i] + j] << " status " << status_array[node_array[index_array[i] + j]] << endl;
#endif
                if( status_array[node_array[index_array[i] + j]] == SELECTED ){
                    independent = false;
                    return independent; 
                } 
            }
        }
    }
    return independent;
}

/*
    this function checks if all the nodes that are NOT SELECTED 
    have at least one neighbor that is SELECTED
*/

bool check_maximal(int *node_array, int *index_array, int *status_array, int numofnodes){
    for(int i = 0; i < numofnodes; i++){
        if(status_array[i] == INACTIVE){
            int numofneighbors = index_array[i+1] - index_array[i];
            bool neighbor_selected = false;
            for(int j = 0; j < numofneighbors; j++){
                if ( status_array[node_array[index_array[i] + j]] == SELECTED ){
                    neighbor_selected = true; 
                    break;
                } 
            }
            if(!neighbor_selected){
                return  false;
            }
        }
    }
    return true;
}

void writeToFileResult(int *nodes, int *index_array, int *nodes_status,int numofnodes, string fileName){

    //open the file
    ofstream myfile;
    myfile.open (fileName.c_str(), std::ios_base::app);

    if(check_independence(nodes, index_array, nodes_status, numofnodes))
        myfile << "independenceTestResult: -----------> passed" << endl;
    else
        myfile << "independenceTestResult: -----------> failed " << endl;
    
    if(check_maximal(nodes, index_array, nodes_status, numofnodes))
        myfile << "maximalTestResult: -----------> passed" << endl;
    else
        myfile << "maximalTestResult: -----------> failed" << endl;

    myfile.close();
}


