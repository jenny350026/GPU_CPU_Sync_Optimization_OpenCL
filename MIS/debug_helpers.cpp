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
void writeToFileNodeStatus(int *nodesStatus, float *nodeRandValues, int nodeArraySize, string fileName){
    //open the file
    ofstream myfile;
    myfile.open (fileName.c_str(), std::ios_base::app);
    //this veriable determines whether we want to show the complete name of the status or just numbers 
    bool showFullStatusName = true; 

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
    myfile.close();
}


//-----------------------------------------------------------------------------------------------------------------------------
////-----------------------------------------------------------------------------------------------------------------------------
////---------module_name::: writeToFileInfo
////---------functionlity::: writes the status of the nodes to the file
////-----------------------------------------------------------------------------------------------------------------------------
////-----------------------------------------------------------------------------------------------------------------------------
void writeToFileNodeRandVals(int *nodesStatus, float *nodeRandValues, int nodeArraySize, string fileName){
    ofstream myfile;
    myfile.open (fileName.c_str(), std::ios_base::app);
    //this veriable determines whether we want to show the complete name of the status or just numbers 
    bool showFullStatusName = true; 
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
    myfile.close();
} 

//-----------------------------------------------------------------------------------------------------------------------------
////-----------------------------------------------------------------------------------------------------------------------------
////---------module_name::: writeToFileInfo
////---------functionlity::: writes the status of the nodes to the file
////-----------------------------------------------------------------------------------------------------------------------------
////-----------------------------------------------------------------------------------------------------------------------------
void writeToFileNodeInfo(int *nodesStatus, float *nodeRandValues, int nodeArraySize, string fileName, string request){
    //show status 
    if (request == "status") 
        writeToFileNodeStatus(nodesStatus, nodeRandValues, nodeArraySize, fileName);
    else if (request == "randValues") 
        writeToFileNodeRandVals(nodesStatus, nodeRandValues, nodeArraySize, fileName);
    else if (request == "all") {
        writeToFileNodeStatus(nodesStatus, nodeRandValues, nodeArraySize, fileName);
        writeToFileNodeRandVals(nodesStatus, nodeRandValues, nodeArraySize, fileName);
    }else {
        ofstream myfile;
        myfile.open (fileName.c_str(), std::ios_base::app);
        myfile<< "this information is not applicable"<<endl;
        myfile.close();
    }
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

void writeToFileTiming(vector<double> step_times, vector<double> rand_times, int numofnodes, string fileName){
    //open the file
    ofstream myfile;
    myfile.open (fileName.c_str(), std::ios_base::app);

    double total_kernel_time = 0;
    double total_rand_time = 0;
    for(int i = 0; i < step_times.size(); ++i){
        //myfile << "Step [" << i << "] time " << step_times[i] << endl;
        myfile << "timeKernel: " << step_times[i] << endl;
        myfile << "timeRand: " << rand_times[i] << endl;
        total_kernel_time += step_times[i];
        total_rand_time += rand_times[i];
    }

    myfile << "Total steps: " << step_times.size() << endl;
    myfile << "Total time kernel: " << total_kernel_time << endl;
    myfile << "Total rand kernel: " << total_rand_time << endl;

    myfile.close();
}

void writeToFileCounters(int *nodes_counter, int step, int numofnodes, string fileName){
    ofstream myfile;
    myfile.open (fileName.c_str(), std::ios_base::app);
    myfile << "Step [" << step << "] start" << endl;
    for(int j = 0; j < numofnodes; ++j){
        if(nodes_counter[j] != 0)
            myfile << "counter[" << j + 1 << "]: " << nodes_counter[j] <<endl;
    }
    myfile << "Step [" << step << "] end" << endl;
    myfile.close();
}
