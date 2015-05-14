/**********************************************************************
Copyright ©2014 Advanced Micro Devices, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

•	Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
•	Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************/

// For clarity,error checking has been omitted.

#include <CL/cl.h>
#include <CL/cl_ext.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <atomic>
#include "CLUtil.hpp"
#include "status.h"
#define SUCCESS 0
#define FAILURE 1
#include "debug_helpers.h"
using namespace appsdk;
using namespace std;

#define SAMPLE_VERSION "AMD-APP-SDK-v3.0.0.0"
#define OCL_COMPILER_FLAGS  "FineGrainSVM_OclFlags.txt"

/*
    reads input file and allocate node_array (list of neighbors) and 
    index_array (number of neighbors) to represent the graph
    ** pass reference to int* in order to get the address of the allocated arrays
    returns the number of nodes in the graph
*/
int read_input_file(string filename, cl_int **addr_node_array, cl_int **addr_index_array, int *num_nodes, int *num_edges, cl_context *context)
{
    ifstream myfile;
    myfile.open(filename.c_str());
    if(!myfile.is_open()){
        cout << "Error opening file!" << endl; 
        exit(1);
    }

    string line;
    getline(myfile, line);
    istringstream iss(line);
    
    int numofnodes = 0;
    int total_numofneighbors = 0;
    iss >> numofnodes;
    iss >> total_numofneighbors;

    *num_nodes = numofnodes;
    *num_edges = total_numofneighbors;

#if DEBUG
    cout << "read_input_file: " << numofnodes << " " << total_numofneighbors <<endl;
#endif

    
    //int* node_array = new int[total_numofneighbors * 2]; //neighbours of each vertex, CSR representation
    //int* index_array = new cl_int[numofnodes + 1]; //index values(address values) for each node, CSR representation

    cl_int* node_array = (int*) clSVMAlloc(*context, CL_MEM_SVM_FINE_GRAIN_BUFFER, (total_numofneighbors*2)*sizeof(int), 0);
    CHECK_ALLOCATION(node_array, "Failed to allocate SVM memory. (node_array)");

    cl_int* index_array = (int*) clSVMAlloc(*context, CL_MEM_SVM_FINE_GRAIN_BUFFER, (numofnodes+1)*sizeof(int), 0);
    CHECK_ALLOCATION(index_array, "Failed to allocate SVM memory. (index_array)");

    int node_index = 0;
    int accum_numofneighbors = 0;
    while(!myfile.eof())
    {   
        getline(myfile,line);
        istringstream iss(line);
	    if (node_index < numofnodes) {	
            index_array[node_index] = accum_numofneighbors;
        }
        int temp;
        while (iss >> temp)
        {
            node_array[accum_numofneighbors] = temp - 1;    
            accum_numofneighbors++;
        }
        node_index++;
    }
    index_array[numofnodes] = accum_numofneighbors;

#if DEBUG
    cout << "read_input_file:" << endl;
    for(int i = 0; i < numofnodes; i++)
    {
        cout << "node "<< i << ": ";
        for(int j = 0; j < index_array[i+1] - index_array[i]; j++)
            cout << node_array[index_array[i] + j] << " ";

        cout << endl;
    }
#endif
    
    *addr_node_array = node_array;
    *addr_index_array = index_array;
}


void write_output(string filename, int *status_array, int numofnodes){

    ofstream ofs(filename.c_str(), ofstream::out);

    int count = 0;
    for(int i = 0; i < numofnodes; i++){
        if(status_array[i] == SELECTED)
            count++;
    }
    
    ofs << "(" << count << "): ";

    for(int i = 0; i < numofnodes; i++){
        if(status_array[i] == SELECTED)
            ofs << i + 1 << " ";
    }

    ofs << endl;
}

int main(int argc, char* argv[])
{
    /* get command line args */

    string outFilename; 
    string inFilename; 
    string logFileName;
    string counterFileName;
    int prime = 0;
    if (argc <= 3) {
        cout << "not enough arguments" << endl;
        cout <<" you need to provide the sparse graph file name and then desired output file name and a logfile name"<<endl; 
        exit (EXIT_FAILURE); 
    }else{
        inFilename = argv[1];
        outFilename = argv[2]; 
        logFileName = argv[3]; 
        counterFileName = argv[4]; 
    } 

    if ( argc >= 6 ){
        stringstream ss(argv[5]);
        ss >> prime;
    }
    SDKTimer *sdk_timer = new SDKTimer();
    int timer = sdk_timer->createTimer();
    int rand_timer = sdk_timer->createTimer();

	/*Step1: Getting platforms and choose an available one.*/
	cl_uint numPlatforms;	//the NO. of platforms
	cl_platform_id platform = NULL;	//the chosen platform
	cl_int	status = clGetPlatformIDs(0, NULL, &numPlatforms);
	if (status != CL_SUCCESS)
	{
		cout << "Error: Getting platforms!" << endl;
		return FAILURE;
	}

	/*For clarity, choose the first available platform. */
	if(numPlatforms > 0)
	{
		cl_platform_id* platforms = (cl_platform_id* )malloc(numPlatforms* sizeof(cl_platform_id));
		status = clGetPlatformIDs(numPlatforms, platforms, NULL);
		platform = platforms[0];
		free(platforms);
	}

	/*Step 2:Query the platform and choose the first GPU device if has one.Otherwise use the CPU as device.*/
	cl_uint		numDevices = 0;
	cl_device_id        *devices;
	status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices);	
	if (numDevices == 0)	//no GPU available.
	{
		cout << "No GPU device available." << endl;
		cout << "Choose CPU as default device." << endl;
		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 0, NULL, &numDevices);	
		devices = (cl_device_id*)malloc(numDevices * sizeof(cl_device_id));
		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, numDevices, devices, NULL);
	}
	else
	{
		devices = (cl_device_id*)malloc(numDevices * sizeof(cl_device_id));
		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, numDevices, devices, NULL);
	}
	

	/*Step 3: Create context.*/
	cl_context context = clCreateContext(NULL,1, devices,NULL,NULL,NULL);
	
	/*Step 4: Creating command queue associate with the context.*/
    //const cl_command_queue_properties props = CL_QUEUE_PROFILING_ENABLE;
    cl_command_queue commandQueue = clCreateCommandQueueWithProperties(context, devices[0], NULL, &status); //we can set outof order execution here, also profiling of the commands
    CHECK_OPENCL_ERROR(status, "clCreateCommandQueue failed(commandQueue)");
	
	//cagri: creating another queue
	cl_command_queue commandQueue2 = clCreateCommandQueueWithProperties(context, devices[0], NULL, &status); //we can set outof order execution here, also profiling of the commands
    CHECK_OPENCL_ERROR(status, "clCreateCommandQueue failed(commandQueue)");
	//cagri: creating another queue
	cl_command_queue commandQueue3 = clCreateCommandQueueWithProperties(context, devices[0], NULL, &status); //we can set outof order execution here, also profiling of the commands
    CHECK_OPENCL_ERROR(status, "clCreateCommandQueue failed(commandQueue)");
	//cagri: creating another queue
	cl_command_queue commandQueue4 = clCreateCommandQueueWithProperties(context, devices[0], NULL, &status); //we can set outof order execution here, also profiling of the commands
    CHECK_OPENCL_ERROR(status, "clCreateCommandQueue failed(commandQueue)");
	


    ///*Step 5: Create program object */
	cl_program program;
    
    /*Step 6: Build program. */
    buildProgramData buildData;
    buildData.kernelName = std::string("MIS_split_thread_kernel.cl");
    buildData.devices = devices;
    buildData.deviceId = 0;
    buildData.flagsStr = std::string("");
    buildData.flagsFileName = std::string(OCL_COMPILER_FLAGS);

    int retVal = buildOpenCLProgram(program, context, buildData);
    cout << "Return value from build: " << retVal << endl; 
    CHECK_ERROR(retVal,CL_SUCCESS ,"program build failed");
    
    /* Finally, time to read in all the inputs */
    cl_int *nodes, *index_array;
    int numofnodes, numofedges;

    read_input_file(inFilename, &nodes, &index_array, &numofnodes, &numofedges, &context);

    // set up execute, random, status, ready and counter arrays
    cl_int* nodes_execute = (int*) clSVMAlloc(context, CL_MEM_SVM_FINE_GRAIN_BUFFER, numofnodes*sizeof(int), 0);
    CHECK_ALLOCATION(nodes_execute, "Failed to allocate SVM memory. (nodes_execute)");
    std::fill_n(nodes_execute, numofnodes, 1);

    cl_float *nodes_randvalues = (float*) clSVMAlloc(context, CL_MEM_SVM_FINE_GRAIN_BUFFER, numofnodes*sizeof(float), 0);
    CHECK_ALLOCATION(nodes_randvalues, "Failed to allocate SVM memory. (nodes_randvalues)");
    srand (20);
    //srand (static_cast <unsigned> (time(0)));
    //for(int i = 0; i < numofnodes; i++)
    //    nodes_randvalues[i]= static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/20));

    cl_int* nodes_status = (int*) clSVMAlloc(context, CL_MEM_SVM_FINE_GRAIN_BUFFER|CL_MEM_SVM_ATOMICS, numofnodes*sizeof(int), 0);
    CHECK_ALLOCATION(nodes_status, "Failed to allocate SVM memory. (nodes_status)");
    std::fill_n(nodes_status, numofnodes, ACTIVE);

    cl_int* nodes_ready = (int*) clSVMAlloc(context, CL_MEM_SVM_FINE_GRAIN_BUFFER|CL_MEM_SVM_ATOMICS, numofnodes*sizeof(int), 0);
    CHECK_ALLOCATION(nodes_ready, "Failed to allocate SVM memory. (nodes_ready)");
    std::fill_n(nodes_ready, numofnodes, 0);

    cl_int* nodes_counter = (int*) clSVMAlloc(context, CL_MEM_SVM_FINE_GRAIN_BUFFER, numofnodes*sizeof(int), 0);
    CHECK_ALLOCATION(nodes_counter, "Failed to allocate SVM memory. (nodes_execute)");

	//cagri : allocating the offset variable
  	cl_int* offset = (int*) clSVMAlloc(context, CL_MEM_SVM_FINE_GRAIN_BUFFER, sizeof(int), 0);
    	CHECK_ALLOCATION(nodes_counter, "Failed to allocate SVM memory. (offset)");

	//cagri : allocating the step variable
  	cl_int* step_of_execution = (int*) clSVMAlloc(context, CL_MEM_SVM_FINE_GRAIN_BUFFER, sizeof(int), 0);
    	CHECK_ALLOCATION(nodes_counter, "Failed to allocate SVM memory. (step)");

	//cagri : allocating the step variable
  	cl_int* step_of_execution2 = (int*) clSVMAlloc(context, CL_MEM_SVM_FINE_GRAIN_BUFFER, sizeof(int), 0);
    	CHECK_ALLOCATION(nodes_counter, "Failed to allocate SVM memory. (step2)");
//cagri : allocating the step variable
  	cl_int* step_of_execution3 = (int*) clSVMAlloc(context, CL_MEM_SVM_FINE_GRAIN_BUFFER, sizeof(int), 0);
    	CHECK_ALLOCATION(nodes_counter, "Failed to allocate SVM memory. (step2)");
//cagri : allocating the step variable
  	cl_int* step_of_execution4 = (int*) clSVMAlloc(context, CL_MEM_SVM_FINE_GRAIN_BUFFER, sizeof(int), 0);
    	CHECK_ALLOCATION(nodes_counter, "Failed to allocate SVM memory. (step2)");
cl_int* step_of_execution5 = (int*) clSVMAlloc(context, CL_MEM_SVM_FINE_GRAIN_BUFFER, sizeof(int), 0);
    	CHECK_ALLOCATION(nodes_counter, "Failed to allocate SVM memory. (step2)");
cl_int* step_of_execution6 = (int*) clSVMAlloc(context, CL_MEM_SVM_FINE_GRAIN_BUFFER, sizeof(int), 0);
    	CHECK_ALLOCATION(nodes_counter, "Failed to allocate SVM memory. (step2)");
cl_int* step_of_execution7 = (int*) clSVMAlloc(context, CL_MEM_SVM_FINE_GRAIN_BUFFER, sizeof(int), 0);
    	CHECK_ALLOCATION(nodes_counter, "Failed to allocate SVM memory. (step2)");
cl_int* step_of_execution8 = (int*) clSVMAlloc(context, CL_MEM_SVM_FINE_GRAIN_BUFFER, sizeof(int), 0);
    	CHECK_ALLOCATION(nodes_counter, "Failed to allocate SVM memory. (step2)");



    //cl_int* nodes_neighbor_counter = (int*) clSVMAlloc(context, CL_MEM_SVM_FINE_GRAIN_BUFFER, numofedges*2*sizeof(int), 4);
    //CHECK_ALLOCATION(nodes_neighbor_counter, "Failed to allocate SVM memory. (nodes_execute)");

#if DEBUG
    cout << "MAIN: " <<endl;
    for(int p = 0; p < numofnodes; p++)
    {
        printf("RandValues[%d] = %f\t",p,nodes_randvalues[p]);
        printf("Status[%d] = %d ",p,nodes_status[p]); 
        printf("\n");
    }
#endif

    cl_int* gpu_remaining_nodes = (int*) clSVMAlloc(context, CL_MEM_SVM_FINE_GRAIN_BUFFER|CL_MEM_SVM_ATOMICS, sizeof(int), 0);
    CHECK_ALLOCATION(gpu_remaining_nodes, "Failed to allocate SVM memory. (gpu_remaining_nodes)");
    *gpu_remaining_nodes = numofnodes;

	/*Step 8: Create kernel object */
	cl_kernel mis_parallel_kernel = clCreateKernel(program,"mis_parallel_async", &status);  //uses the opencl file. create couple if you have a copule of kernels
    CHECK_OPENCL_ERROR(status, "clCreateKernel mis_parallel_async failed");

	cl_kernel deactivate_neighbors_kernel = clCreateKernel(program,"deactivate_neighbors", &status);  //uses the opencl file. create couple if you have a copule of kernels
    CHECK_OPENCL_ERROR(status, "clCreateKernel deactivate_neighbors failed");
    
   /*Step 9: Sets Kernel arguments.*/
    // For mis_parallel_async kernel
	status = clSetKernelArgSVMPointer(
                mis_parallel_kernel,
                0,
                nodes);
    CHECK_OPENCL_ERROR(status, "setting kernel arguments failed asdfas"); 

    status = clSetKernelArgSVMPointer(
                mis_parallel_kernel,
                1, 
                nodes_randvalues); 
    CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 

    status = clSetKernelArgSVMPointer(
                mis_parallel_kernel,
                2, 
                nodes_status); 
    CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 

    status = clSetKernelArgSVMPointer(
                mis_parallel_kernel,
                3, 
                index_array); 
    CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 

    status = clSetKernelArgSVMPointer(
                mis_parallel_kernel,
                4,
                nodes_execute); 
    CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 

    status = clSetKernelArgSVMPointer(
                mis_parallel_kernel,
                5,
                nodes_ready); 
    CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 

    status = clSetKernelArgSVMPointer(
                mis_parallel_kernel,
                6,
                nodes_counter); 
    CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 

	//cagri : adding offset parameter
	status = clSetKernelArgSVMPointer(
                mis_parallel_kernel,
                7,
                offset); 
    	CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 
	
	//cagri : adding step parameter
	status = clSetKernelArgSVMPointer(
                mis_parallel_kernel,
                8,
                step_of_execution); 
    	CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 


    //status = clSetKernelArgSVMPointer(
    //            mis_parallel_kernel,
    //            7,
    //            nodes_neighbor_counter); 
    //CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 

    // For deactivate_neighbors kernel
	status = clSetKernelArgSVMPointer(
                deactivate_neighbors_kernel,
                0,
                nodes);
    CHECK_OPENCL_ERROR(status, "setting kernel arguments failed asdfas"); 

    status = clSetKernelArgSVMPointer(
                deactivate_neighbors_kernel,
                1, 
                nodes_randvalues); 
    CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 

    status = clSetKernelArgSVMPointer(
                deactivate_neighbors_kernel,
                2, 
                nodes_status); 
    CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 

    status = clSetKernelArgSVMPointer(
                deactivate_neighbors_kernel,
                3, 
                gpu_remaining_nodes);
    CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 

    status = clSetKernelArgSVMPointer(
                deactivate_neighbors_kernel,
                4, 
                index_array); 
    CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 

    status = clSetKernelArgSVMPointer(
                deactivate_neighbors_kernel,
                5,
                nodes_execute); 
    CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 

	//cagri : adding parameter to pass	
	status = clSetKernelArgSVMPointer(
                deactivate_neighbors_kernel,
                6,
                offset); 
    	CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 

	//cagri : adding step parameter to kernel	
	status = clSetKernelArgSVMPointer(
                deactivate_neighbors_kernel,
                7,
                step_of_execution); 
    	CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 



 	// Set Global and Local work items
	//size_t globalWorkItems = numofnodes; //total number of threads
    	//cagri : for testing purposes i split by 2 now.
	size_t globalWorkItems = numofnodes / 8;
	//cagri : local thread number is same and at max!
	size_t localWorkItems = 256; //number of threads within a work group

	size_t globalWorkItems2 = numofnodes / 8;
	size_t globalWorkItems3 = numofnodes / 8;
	size_t globalWorkItems4 = numofnodes / 8;
	size_t globalWorkItems5 = numofnodes / 8;
	size_t globalWorkItems6 = numofnodes / 8;
	size_t globalWorkItems7 = numofnodes / 8;
	size_t globalWorkItems8 = numofnodes - 7*globalWorkItems;

     //launching the kernel

	//cagri : defining offset. offset is border like thing.
	*offset = numofnodes / 8;
	printf("Offset is %d\n", *offset);
	//cagri : step is SVMalloced bc I pass it to kernels

	*step_of_execution = 0; //cagri: Seems like step is used for writing and stuff so i decided to use another variable	
	*step_of_execution2 = 1;
	*step_of_execution3 = 2;
	*step_of_execution4 = 3;
	*step_of_execution5 = 4;
	*step_of_execution6 = 5;
	*step_of_execution7 = 6;
	*step_of_execution8 = 7;
	
	cl_event sync;

    	int step = 0;
    	cl_event ndrEvt;
    	vector<double> step_times;
    	vector<double> rand_times;
	//int locked = 0;		//cagri: this lock will be used to satisfy randomization requirements for other randomization steps

    while (step < 10 && *gpu_remaining_nodes > 0){
        cout << "**** Running Step " << step << " ***"<< endl;
        std::fill_n(nodes_ready, numofnodes, 0);

	*offset = numofnodes / 8;

//        if (prime != 0)
//            cout << " Prime enabled with = " << prime << endl;
 
	sdk_timer->resetTimer(rand_timer);
       	sdk_timer->startTimer(rand_timer);
//	printf("Randomizing, Step is %d, Offset is %d\n", (int)*step_of_execution, (int)*offset);
	for(int i = 0; i < prime/4; i++)
		{
      		nodes_randvalues[i]= static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/20));
      		std::atomic_store_explicit ((std::atomic<int>*)&nodes_ready[i], 1, std::memory_order_release);
       		//cout << nodes_randvalues[i] << endl;
       		}
       	sdk_timer->stopTimer(rand_timer);
	
	double half_rand_time = (double)(sdk_timer->readTimer(rand_timer));
        cout << "Half Rand Time = " <<  half_rand_time  << "s" << endl;
        step_times.push_back(half_rand_time);

//	cout << "First kernel launch..."<<endl;
	sdk_timer->resetTimer(timer);
        sdk_timer->startTimer(timer);

	status = clSetKernelArgSVMPointer(mis_parallel_kernel,8,step_of_execution); 
    	CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 
	status = clSetKernelArgSVMPointer(mis_parallel_kernel,7,offset); 
    	CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 
	status = clSetKernelArgSVMPointer(deactivate_neighbors_kernel,7,step_of_execution); 
    	CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 
	status = clSetKernelArgSVMPointer(deactivate_neighbors_kernel,6,offset); 
    	CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 


	status = clEnqueueNDRangeKernel(commandQueue,mis_parallel_kernel,1,NULL,&globalWorkItems,&localWorkItems,0,NULL,NULL);
        CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel(mis_parrallel_kernel) failed."); 
      	status = clEnqueueNDRangeKernel(commandQueue,deactivate_neighbors_kernel,1,NULL,&globalWorkItems,&localWorkItems,0,NULL,NULL);
        CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel(deactivate_neighbors_kernel) failed."); 
        status = clFlush(commandQueue);
        CHECK_OPENCL_ERROR(status, "clFlush failed.(commandQueue)");

//	cout << "1st Kernel is Sent!" << endl;
	
	/********222*******/
	status = clSetKernelArgSVMPointer(mis_parallel_kernel,8,step_of_execution2); 
    	CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 
	//status = clSetKernelArgSVMPointer(mis_parallel_kernel,7,globalWorkItems); 
    	//CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 
	status = clSetKernelArgSVMPointer(deactivate_neighbors_kernel,7,step_of_execution2); 
    	CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 
	//status = clSetKernelArgSVMPointer(deactivate_neighbors_kernel,6,globalWorkItems); 
    	//CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 

	status = clEnqueueNDRangeKernel(commandQueue,mis_parallel_kernel,1,NULL,&globalWorkItems2,&localWorkItems,0,NULL,NULL);
        CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel(mis_parrallel_kernel) failed."); 
      	status = clEnqueueNDRangeKernel(commandQueue,deactivate_neighbors_kernel,1,NULL,&globalWorkItems2,&localWorkItems,0,NULL,NULL);
        CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel(deactivate_neighbors_kernel) failed."); 
        status = clFlush(commandQueue);
        CHECK_OPENCL_ERROR(status, "clFlush failed.(commandQueue)");
	
//	cout << "2nd Kernel is Sent!" << endl;

	for(int i = (prime/4); i < (prime/2); i++)
		{
       		nodes_randvalues[i]= static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/20));
       		std::atomic_store_explicit ((std::atomic<int>*)&nodes_ready[i], 1, std::memory_order_release);
       		//cout << nodes_randvalues[i] << endl;
      		}
//	cout << "Prime/4 to Prime/2 is Randomized!" << endl;

	/**********333**************/
	status = clSetKernelArgSVMPointer(mis_parallel_kernel,8,step_of_execution3); 
    	CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 
	//status = clSetKernelArgSVMPointer(mis_parallel_kernel,7,globalWorkItems); 
    	//CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 
	status = clSetKernelArgSVMPointer(deactivate_neighbors_kernel,7,step_of_execution3); 
    	CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 
	//status = clSetKernelArgSVMPointer(deactivate_neighbors_kernel,6,globalWorkItems); 
    	//CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 

	status = clEnqueueNDRangeKernel(commandQueue2,mis_parallel_kernel,1,NULL,&globalWorkItems3,&localWorkItems,0,NULL,NULL);
        CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel(mis_parrallel_kernel) failed."); 
      	status = clEnqueueNDRangeKernel(commandQueue2,deactivate_neighbors_kernel,1,NULL,&globalWorkItems3,&localWorkItems,0,NULL,NULL);
        CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel(deactivate_neighbors_kernel) failed."); 
        status = clFlush(commandQueue2);
        CHECK_OPENCL_ERROR(status, "clFlush failed.(commandQueue)");
	
//	cout << "3rd Kernel is Sent!" << endl;	

	/**********444**************/

	status = clSetKernelArgSVMPointer(mis_parallel_kernel,8,step_of_execution4); 
    	CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 
	//status = clSetKernelArgSVMPointer(mis_parallel_kernel,7,offset); 
    	//CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 
	status = clSetKernelArgSVMPointer(deactivate_neighbors_kernel,7,step_of_execution4); 
    	CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 
	//status = clSetKernelArgSVMPointer(deactivate_neighbors_kernel,6,offset); 
    	//CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 

	status = clEnqueueNDRangeKernel(commandQueue2,mis_parallel_kernel,1,NULL,&globalWorkItems4,&localWorkItems,0,NULL,NULL);
        CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel(mis_parrallel_kernel) failed."); 
      	status = clEnqueueNDRangeKernel(commandQueue2,deactivate_neighbors_kernel,1,NULL,&globalWorkItems4,&localWorkItems,0,NULL,NULL);
        CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel(deactivate_neighbors_kernel) failed."); 
        status = clFlush(commandQueue2);
        CHECK_OPENCL_ERROR(status, "clFlush failed.(commandQueue)");
	
//	cout << "4th Kernel is Sent!" << endl;	

	/*~~~~~~~~~~~~~~~*/

	for(int i = (prime/2); i < (3*prime/4); i++)
		{
       		nodes_randvalues[i]= static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/20));
       		std::atomic_store_explicit ((std::atomic<int>*)&nodes_ready[i], 1, std::memory_order_release);
       		//cout << nodes_randvalues[i] << endl;
      		}
//	cout << "Prime/2 to 3*Prime/4 is Randomized!" << endl;
	

	/**********555**************/
	status = clSetKernelArgSVMPointer(mis_parallel_kernel,8,step_of_execution5); 
    	CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 
	//status = clSetKernelArgSVMPointer(mis_parallel_kernel,7,globalWorkItems); 
    	//CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 
	status = clSetKernelArgSVMPointer(deactivate_neighbors_kernel,7,step_of_execution5); 
    	CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 
	//status = clSetKernelArgSVMPointer(deactivate_neighbors_kernel,6,globalWorkItems); 
    	//CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 

	status = clEnqueueNDRangeKernel(commandQueue3,mis_parallel_kernel,1,NULL,&globalWorkItems5,&localWorkItems,0,NULL,NULL);
        CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel(mis_parrallel_kernel) failed."); 
      	status = clEnqueueNDRangeKernel(commandQueue3,deactivate_neighbors_kernel,1,NULL,&globalWorkItems5,&localWorkItems,0,NULL,NULL);
        CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel(deactivate_neighbors_kernel) failed."); 
        status = clFlush(commandQueue3);
        CHECK_OPENCL_ERROR(status, "clFlush failed.(commandQueue)");
	
//	cout << "5rd Kernel is Sent!" << endl;	

	/**********666**************/

	status = clSetKernelArgSVMPointer(mis_parallel_kernel,8,step_of_execution6); 
    	CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 
	//status = clSetKernelArgSVMPointer(mis_parallel_kernel,7,offset); 
    	//CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 
	status = clSetKernelArgSVMPointer(deactivate_neighbors_kernel,7,step_of_execution6); 
    	CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 
	//status = clSetKernelArgSVMPointer(deactivate_neighbors_kernel,6,offset); 
    	//CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 

	status = clEnqueueNDRangeKernel(commandQueue3,mis_parallel_kernel,1,NULL,&globalWorkItems6,&localWorkItems,0,NULL,NULL);
        CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel(mis_parrallel_kernel) failed."); 
      	status = clEnqueueNDRangeKernel(commandQueue3,deactivate_neighbors_kernel,1,NULL,&globalWorkItems6,&localWorkItems,0,NULL,NULL);
        CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel(deactivate_neighbors_kernel) failed."); 
        status = clFlush(commandQueue3);
        CHECK_OPENCL_ERROR(status, "clFlush failed.(commandQueue)");
	
//	cout << "6th Kernel is Sent!" << endl;	

	for(int i = 3*prime/4; i < prime; i++)
		{
       		nodes_randvalues[i]= static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/20));
       		std::atomic_store_explicit ((std::atomic<int>*)&nodes_ready[i], 1, std::memory_order_release);
       		//cout << nodes_randvalues[i] << endl;
      		}
//	cout << "3*Prime/4 to Prime is Randomized!" << endl;
	
	/**********777**************/
	status = clSetKernelArgSVMPointer(mis_parallel_kernel,8,step_of_execution7); 
    	CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 
	//status = clSetKernelArgSVMPointer(mis_parallel_kernel,7,globalWorkItems); 
    	//CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 
	status = clSetKernelArgSVMPointer(deactivate_neighbors_kernel,7,step_of_execution7); 
    	CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 
	//status = clSetKernelArgSVMPointer(deactivate_neighbors_kernel,6,globalWorkItems); 
    	//CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 

	status = clEnqueueNDRangeKernel(commandQueue4,mis_parallel_kernel,1,NULL,&globalWorkItems7,&localWorkItems,0,NULL,NULL);
        CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel(mis_parrallel_kernel) failed."); 
      	status = clEnqueueNDRangeKernel(commandQueue4,deactivate_neighbors_kernel,1,NULL,&globalWorkItems7,&localWorkItems,0,NULL,NULL);
        CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel(deactivate_neighbors_kernel) failed."); 
        status = clFlush(commandQueue4);
        CHECK_OPENCL_ERROR(status, "clFlush failed.(commandQueue)");
	
//	cout << "7rd Kernel is Sent!" << endl;	

	/**********888**************/

//	*offset = numofnodes - 7*(*offset);
	status = clSetKernelArgSVMPointer(mis_parallel_kernel,8,step_of_execution8); 
    	CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 
//	status = clSetKernelArgSVMPointer(mis_parallel_kernel,7,offset); 
  //  	CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 
	status = clSetKernelArgSVMPointer(deactivate_neighbors_kernel,7,step_of_execution8); 
    	CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 
//	status = clSetKernelArgSVMPointer(deactivate_neighbors_kernel,6,offset); 
//    	CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 

	status = clEnqueueNDRangeKernel(commandQueue4,mis_parallel_kernel,1,NULL,&globalWorkItems8,&localWorkItems,0,NULL,NULL);
        CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel(mis_parrallel_kernel) failed."); 
      	status = clEnqueueNDRangeKernel(commandQueue4,deactivate_neighbors_kernel,1,NULL,&globalWorkItems8,&localWorkItems,0,NULL,&sync);
        CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel(deactivate_neighbors_kernel) failed."); 
        status = clFlush(commandQueue4);
        CHECK_OPENCL_ERROR(status, "clFlush failed.(commandQueue)");
	
//	cout << "8th Kernel is Sent!" << endl;	

	status = waitForEventAndRelease(&sync);
	CHECK_OPENCL_ERROR(status, "waitForEventAndRelease failed.(sync)");

//	cout << "After Wait!" << endl;

	sdk_timer->stopTimer(timer);
	double time = (double)(sdk_timer->readTimer(timer));
        cout << "Time = " <<  time  << "s" << endl;
        step_times.push_back(time);

//	cout << "GPU Remaining Nodes = " << *gpu_remaining_nodes << endl;

      	//  double step_time = (double)(sdk_timer->readTimer(timer));
      	//  cout << "Kernel time (Kern+SecondHalfRand) = " <<  step_time  << "s" << endl;
      	//  step_times.push_back(step_time);

        //   cout << "Remaining: " << *gpu_remaining_nodes << endl;
        //for(int i = 0; i < numofnodes; ++i){
        //    if(nodes_counter[i] != 0)
        //        cout << "node " << i + 1 << " waited " << nodes_counter[i] << " iterations." <<endl;
        //}
            
        //---------guide:::  writing to the output
        //writing the random values in the log file 
        //writeToFileNodeInfo(nodes_status, nodes_randvalues, numofnodes,logFileName, "all");
        //showNodesInfo(nodes_status, nodes_randvalues, numofnodes, "all");
        // writeToFileCounters(nodes_counter, step, numofnodes, counterFileName);
//	}


        ++step;
//	*step_of_execution++;
	clFinish(commandQueue);	  
	clFinish(commandQueue2);	  
	clFinish(commandQueue3);	  
	clFinish(commandQueue4);	  
  
}
    

cout << "out of while loop!" << endl;

    //writing the result to the output 
//    write_output(outFilename , nodes_status, numofnodes);
    
    //writing the test results
//    writeToFileResult(nodes, index_array, nodes_status, numofnodes, logFileName);
    //writing the timing information
//    writeToFileTiming(step_times, rand_times, numofnodes, logFileName);

	/*Step 12: Clean the resources.*/
	status = clReleaseKernel(mis_parallel_kernel);				//Release kernel.
	status = clReleaseKernel(deactivate_neighbors_kernel);				//Release kernel.
	status = clReleaseProgram(program);				//Release the program object.
	status = clReleaseCommandQueue(commandQueue);	//Release  Command queue.
	status = clReleaseContext(context);				//Release context.


	if (devices != NULL)
	{
		free(devices);
		devices = NULL;
	}


	return SUCCESS;
}
