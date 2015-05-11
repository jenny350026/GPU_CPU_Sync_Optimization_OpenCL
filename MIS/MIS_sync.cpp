#include <CL/cl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include "status.h"
#include "debug_helpers.h"
#include "CL/cl_ext.h"

#include <CLUtil.hpp>
#define OCL_COMPILER_FLAGS  "FineGrainSVM_OclFlags.txt"

using namespace appsdk;

int convertToString(const char *filename, std::string& s)
{
	size_t size;
	char*  str;
	std::fstream f(filename, (std::fstream::in | std::fstream::binary));

	if(f.is_open())
	{
		size_t fileSize;
		f.seekg(0, std::fstream::end);
		size = fileSize = (size_t)f.tellg();
		f.seekg(0, std::fstream::beg);
		str = new char[size+1];
		if(!str)
		{
			f.close();
			return 0;
		}

		f.read(str, fileSize);
		f.close();
		str[size] = '\0';
		s = str;
		delete[] str;
		return 0;
	}
	cout<<"Error: failed to open file\n:"<<filename<<endl;
	return 1;
}


int read_input_file(string filename, int **addr_node_array, int **addr_index_array, int *num_nodes, int *num_edges, cl_context contextfrommain )
{
    	return 0;
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


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

int main(int argc, char* argv[])
{

    string outFilename; 
    string inFilename; 
    string logFileName;

    if (argc <= 3) {
        cout << "not enough arguments" << endl;
        cout <<" you need to provide the sparse graph file name and then desired output file name and a logfile name"<<endl; 
        exit (EXIT_FAILURE); 
    }else{
        inFilename = argv[1];
        outFilename = argv[2]; 
        logFileName = argv[3]; 
    } 



    //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------- 
    //-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------   
    //---------guide:::  open cl parameters intialization
    /*Step1: Getting platforms and choose an available one.*/
    cl_uint numPlatforms;	//the NO. of platforms
    cl_platform_id platform = NULL;	//the chosen platform
    cl_int	status = clGetPlatformIDs(0, NULL, &numPlatforms);

    if (status != CL_SUCCESS)
    {
        cout << "Error: Getting platforms!" << endl;
        return 1;
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
	cl_uint numDevices = 0;
	cl_device_id *devices;
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
		cout << "num of devices is = " << numDevices << endl;
		devices = (cl_device_id*)malloc(numDevices * sizeof(cl_device_id));
		status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, numDevices, devices, NULL);
	}
	
	cl_int errorcode; //this will be used for error checking	

	/*Step 3: Create context.*/
	cl_context context = clCreateContext(NULL,1, devices,NULL,NULL,&errorcode);
		if(errorcode != CL_SUCCESS) cout << "Error Creating Context : " << get_error_string(errorcode) << endl;
		else cout << "Context created!" << endl;


	cout << "before queue creation" << endl;
	/*Step 4: Creating command queue associate with the context.*/
	cl_command_queue commandQueue = clCreateCommandQueueWithProperties(context,devices[0], NULL, &errorcode);
	if(errorcode != CL_SUCCESS) cout << "Error Creating Queue0 : " << get_error_string(errorcode) << endl;
		else cout << "Queue0 created!" << endl;




    /*Step 6: Build program. */
    cl_program program;
    buildProgramData buildData;
    buildData.kernelName = std::string("MIS_sync.cl");
    buildData.devices = devices;
    buildData.deviceId = 0;
    buildData.flagsStr = std::string("");
    buildData.flagsFileName = std::string(OCL_COMPILER_FLAGS);

    int retVal = buildOpenCLProgram(program, context, buildData);
    cout << "Retur value from build: " <<retVal << endl; 
    CHECK_ERROR(retVal,CL_SUCCESS ,"program build failed");
 
    cl_program program_deactivate;
    buildProgramData buildData2;
    buildData2.kernelName = std::string("MIS_sync_deactivate.cl");
    buildData2.devices = devices;
    buildData2.deviceId = 0;
    buildData2.flagsStr = std::string("");
    buildData2.flagsFileName = std::string(OCL_COMPILER_FLAGS);

    retVal = buildOpenCLProgram(program_deactivate, context, buildData2);
    cout << "Retur value from build: " <<retVal << endl; 
    
    char buf[0x10000];
    clGetProgramBuildInfo(program,devices[0],CL_PROGRAM_BUILD_LOG,0x10000,buf,NULL);
    printf("%s\n",buf);

    char buf2[0x30000];
    clGetProgramBuildInfo(program_deactivate,devices[0],CL_PROGRAM_BUILD_LOG,0x30000,buf2,NULL);
    printf("%s\n",buf2);

    //-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    //---------guide::: end of opencl basics  
   //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------- 
   //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------- 
        //creating CSR representation fomr the spraseMatrix 
        ifstream myfile;
        myfile.open(inFilename.c_str());
        if(!myfile.is_open()){
            cout << "Error opening file!" << endl; 
            return 1; 
        }

        string line;
        getline(myfile, line);
        istringstream iss(line);

        int numofnodes = 0;
        int total_numofneighbors = 0;
        iss >> numofnodes;
        iss >> total_numofneighbors;  // number is INCORRECT right now


        //#define numofnodes numofnodes
        int* nodes = (int*) clSVMAlloc(context, CL_MEM_READ_ONLY, sizeof(int)*total_numofneighbors*2, 0);//new int[total_numofneighbors * 2]; //neighbours of each vertex, CSR representation
        
        //define index array  
        int* index_array = (int*) clSVMAlloc(context, CL_MEM_READ_ONLY, sizeof(int)*(numofnodes+1),0); //new int[numofnodes + 1]; //index values(address values) for each node, CSR representation
        int node_index = 0;
        int accum_numofneighbors = 0;

        cl_int readerror; //error checking variable 
        //---------guide:::  mapping so CPU can use the variable
        readerror = clEnqueueSVMMap(commandQueue, CL_TRUE, CL_MAP_WRITE, nodes, sizeof(int)*total_numofneighbors*2, 0, NULL, NULL);
            if(readerror != CL_SUCCESS) cout << "Error for mapping nodes into host to create nodes array!" << get_error_string(readerror) << endl;
        readerror = clEnqueueSVMMap(commandQueue, CL_TRUE, CL_MAP_WRITE, index_array, sizeof(int)*(numofnodes+1), 0, NULL, NULL);
            if(readerror != CL_SUCCESS) cout << "Error for mapping index_array into hos1 to create index_array!" << get_error_string(readerror) << endl;
         
            
        //---------guide::: parsing the file
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
                nodes[accum_numofneighbors] = temp - 1;    
                accum_numofneighbors++;
        	}
        	node_index++;
    	}
    	index_array[numofnodes] = accum_numofneighbors;

        //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------- 
        //---------guide:::  Allocating Buffers shared
        //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------- 
        //---------guide:::  Unmapping so CPU can use the variable
        readerror = clEnqueueSVMUnmap(commandQueue,nodes, 0, NULL, NULL);
            if(readerror != CL_SUCCESS) cout << "Error for unmapping nodes!" << get_error_string(readerror) << endl;
        readerror = clEnqueueSVMUnmap(commandQueue,index_array, 0, NULL, NULL);
            if(readerror != CL_SUCCESS) cout << "Error for unmapping index_array!" << get_error_string(readerror) << endl;
       //---------guide:::  end of reading the file and getting the CSR arrays (node and index_array)
	
	   
       /*Step 7: Setup the inputs*/
       cout<< "SVMAllocing nodes_randvalues..." << endl;
       //---------guide:::  define and isntantiate the randvalues
       float* nodes_randvalues = (float*) clSVMAlloc(context,CL_MEM_READ_ONLY, sizeof(float)*numofnodes,0); //new float[numofnodes];
       readerror = clEnqueueSVMMap(commandQueue, CL_TRUE, CL_MAP_WRITE, nodes_randvalues, sizeof(float)*(numofnodes), 0, NULL, NULL);
       if(readerror != CL_SUCCESS) cout << "Error nodes_randvalues are not mapped!" << get_error_string(readerror) << endl;
       std::fill_n(nodes_randvalues, numofnodes, 0);		

       readerror = clEnqueueSVMUnmap(commandQueue,nodes_randvalues, 0, NULL, NULL);
		if(readerror != CL_SUCCESS) cout << "Error for unmapping nodes_randvalues!" << get_error_string(readerror) << endl;
    	cout<< "SVMAllocing nodes_randvalues...DONE" << endl;
	
        
        
        //---------guide::: define and instantiate the node_status  (probably not needed)
        /* 
        cout<< "SVMallocing nodes_status..." << endl;
        int *nodes_status = (int*)clSVMAlloc(context, CL_MEM_READ_WRITE, sizeof(int)*numofnodes,0); //new int[numofnodes];

        readerror = clEnqueueSVMMap(commandQueue, CL_TRUE, CL_MAP_WRITE, nodes_status, sizeof(int)*(numofnodes), 0, NULL, NULL);
            if(readerror != CL_SUCCESS) cout << "Error nodes_status are not mapped!" << get_error_string(readerror) << endl;

        cout<< "nodes_status are being initialized to ACTIVE..." << endl;
        std::fill_n(nodes_status, numofnodes, ACTIVE);		

        cout<< "SVMAllocing nodes_status...DONE" << endl;


        readerror = clEnqueueSVMUnmap(commandQueue,nodes_status, 0, NULL, NULL);
            if(readerror != CL_SUCCESS) cout << "Error for unmapping nodes_status!" << get_error_string(readerror) << endl;
        ;
       */
       //
	
       //---------guide::: define and instantiate the node_status_parallel  
        cout<< "SVMallocing nodes_status_parallel..." << endl;
        int *nodes_status_parallel = (int*)clSVMAlloc(context, CL_MEM_READ_WRITE, sizeof(int)*numofnodes,0); //new int[numofnodes];

        readerror = clEnqueueSVMMap(commandQueue, CL_TRUE, CL_MAP_WRITE, nodes_status_parallel, sizeof(int)*(numofnodes), 0, NULL, NULL);
            if(readerror != CL_SUCCESS) cout << "Error nodes_status_parallel are not mapped!" << get_error_string(readerror) << endl;

        cout<< "nodes_status_parallel are being initialized to ACTIVE..." << endl;
        std::fill_n(nodes_status_parallel, numofnodes, ACTIVE);		

        cout<< "SVMAllocing nodes_status_parallel...DONE" << endl;


        readerror = clEnqueueSVMUnmap(commandQueue,nodes_status_parallel, 0, NULL, NULL);
            if(readerror != CL_SUCCESS) cout << "Error for unmapping nodes_status_parallel!" << get_error_string(readerror) << endl;
        //
	
            
       //---------guide::: define and instantiate the node_execute
       int *nodes_execute = (int*)clSVMAlloc(context, CL_MEM_READ_WRITE, sizeof(int)*numofnodes,0); // new int[numofnodes];


       //---------guide::: define and instantiate the node_remaining_nodes
       int* gpu_remainingnodes = (int*)clSVMAlloc(context, CL_MEM_READ_WRITE, sizeof(int),0);
       readerror = clEnqueueSVMMap(commandQueue, CL_TRUE, CL_MAP_WRITE, gpu_remainingnodes, sizeof(int), 0, NULL, NULL);
            if(readerror != CL_SUCCESS) cout << "Error gpu_remainingnodes is not mapped!" << get_error_string(readerror) << endl;

       *gpu_remainingnodes = numofnodes;
	   cout << "Initilizing gpu_remainingnodes!" << endl;
	
	   readerror = clEnqueueSVMUnmap(commandQueue,gpu_remainingnodes, 0, NULL, NULL);
	         if(readerror != CL_SUCCESS) cout << "Error for unmapping gpu_remainingnodes!" << get_error_string(readerror) << endl;


       int step = 0;
	   
       //---------guide::: define and instantiate the counter_gpu
       int *counter_gpu = (int*)clSVMAlloc(context,CL_MEM_READ_WRITE, sizeof(int)*numofnodes, 0); // new int[numofnodes]; 
	   //this will be used to store counter information for every gpu thread
	   readerror = clEnqueueSVMMap(commandQueue, CL_TRUE, CL_MAP_WRITE, counter_gpu, sizeof(int)*numofnodes, 0, NULL, NULL);
		if(readerror != CL_SUCCESS) cout << "Error counter_gpu is not mapped!" << get_error_string(readerror) << endl;

        std::fill_n(counter_gpu, numofnodes,0);	
        cout << "Initializing counter_gpu to Zero!" << endl;

        readerror = clEnqueueSVMUnmap(commandQueue,counter_gpu, 0, NULL, NULL);
		    if(readerror != CL_SUCCESS) cout << "Error for unmapping counter_gpu!" << get_error_string(readerror) << endl;

	    cout << "Inputs (buffers) are created!" << endl;

        
        //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------- 
        //---------guide::: end of allocating Buffers shared
        //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------- 
        
       //---------guide:::  dealing with Kernel
        cl_int error;
        cl_int error2;
        /*  Create Kernel Object*/
        cl_kernel kernel = clCreateKernel(program,"mis_parallel_async",&error);
        cl_kernel kernel_deactivate = clCreateKernel(program_deactivate,"deactivate_neighbors",&error2);

        if (error != CL_SUCCESS) cout << "Error kernel = " << get_error_string(error) << endl;
        if (error2 != CL_SUCCESS) cout << "Error kernel_deactivate = " << get_error_string(error2) << endl;
	
       //---------guide:::  setting kernel arguments
	   //kernel : execution kernel	
	   //mis_parallel_async(counter_gpu,nodes,nodes_randvalues,nodes_status_parallel, index_array,nodes_execute,lparm); 
        status = clSetKernelArgSVMPointer(kernel,0, (void *)(counter_gpu));
			if (status != CL_SUCCESS) cout << "Error kernel kernelarg 1 = " << get_error_string(status) << endl;
		status = clSetKernelArgSVMPointer(kernel,1, (void *)(nodes));
			if (status != CL_SUCCESS) cout << "Error kernel kernelarg 2 = " << get_error_string(status) << endl;
		status = clSetKernelArgSVMPointer(kernel,2, (void *)(nodes_randvalues));
			if (status != CL_SUCCESS) cout << "Error kernel kernelarg 3 = " << get_error_string(status) << endl;
		status = clSetKernelArgSVMPointer(kernel,3, (void *)(nodes_status_parallel));
			if (status != CL_SUCCESS) cout << "Error kernel kernelarg 4 = " << get_error_string(status) << endl;
		status = clSetKernelArgSVMPointer(kernel,4, (void *)(index_array));
			if (status != CL_SUCCESS) cout << "Error kernel kernelarg 5 = " << get_error_string(status) << endl;
		status = clSetKernelArgSVMPointer(kernel,5, (void *)(nodes_execute));
			if (status != CL_SUCCESS) cout << "Error kernel kernelarg 6 = " << get_error_string(status) << endl;
	    
        //kernel: deactivate kernel
		//deactivate_neighbors(nodes,nodes_randvalues,nodes_status_parallel,&gpu_remainingnodes, index_array,nodes_execute,lparm);
        status = clSetKernelArgSVMPointer(kernel_deactivate,0, (void *)(nodes));
			if (status != CL_SUCCESS) cout << "Error kernel kernelarg deactivate 1 = " << get_error_string(status) << endl;
		status = clSetKernelArgSVMPointer(kernel_deactivate,1, (void *)(nodes_randvalues));
			if (status != CL_SUCCESS) cout << "Error kernel kernelarg deactivate 2 = " << get_error_string(status) << endl;
		status = clSetKernelArgSVMPointer(kernel_deactivate,2, (void *)(nodes_status_parallel));
			if (status != CL_SUCCESS) cout << "Error kernel kernelarg deactivate 3 = " << get_error_string(status) << endl;
		status = clSetKernelArgSVMPointer(kernel_deactivate,3, (void *)(gpu_remainingnodes)); 
			if (status != CL_SUCCESS) cout << "Error kernel kernelarg deactivate 4 = " << get_error_string(status) << endl;
		status = clSetKernelArgSVMPointer(kernel_deactivate,4, (void *)(index_array));
			if (status != CL_SUCCESS) cout << "Error kernel kernelarg deactivate 5 = " << get_error_string(status) << endl;
		status = clSetKernelArgSVMPointer(kernel_deactivate,5, (void *)(nodes_execute));
			if (status != CL_SUCCESS) cout << "Error kernel kernelarg deactivate 6 = " << get_error_string(status) << endl;
        //	
        ;
        /*  Rand and Call GPU*/
        //---------guide:::  detemrining the number of used compute units
        cl_uint compute_units;
        size_t threadnumber;
        clGetDeviceInfo(devices[0], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &compute_units, NULL);
        cout << "Compute Units =" << compute_units << endl;

        threadnumber = numofnodes;
        int locked = 0;

        //---------guide:::  get the gput remaining node
        cl_int errormap;
        status = clEnqueueSVMMap(commandQueue, CL_TRUE, CL_MAP_READ, gpu_remainingnodes, sizeof(int), 0, NULL, NULL);
        if(errormap != CL_SUCCESS) cout << "Error for reading back" << get_error_string(errormap) << endl;
        else	{
            cout << "GPU_REMAININGNODES is mapped to Host with = " << *gpu_remainingnodes << endl;
            locked = 1;
        }

        //---------guide:::  
        KernelWorkGroupInfo kernelInfo;
        kernelInfo.setKernelWorkGroupInfo(kernel,devices[0]);
       //---------guide::: for now we are setting the number of workgropu size to maixmum. can be change later
 	   size_t localThreads = kernelInfo.kernelWorkGroupSize;
       cout << "Local Threads = " << localThreads << endl;
   
       //---------guide:::  generating the seed
       srand (static_cast <unsigned> (time(0)));


       while(*gpu_remainingnodes > 0 )
       {
           //locked is one when cpu has mapped (has sole access to the gpu_remainingnodes) 
           if(locked == 1)
           {
               //unamp so gpu can use 
               status = clEnqueueSVMUnmap(commandQueue, gpu_remainingnodes,0,NULL,NULL);
               locked = 0;
               cout << "gpu_remainingnodes is unmapped from host!" << endl;
           }

          //---------guide:::  randomize values 
           cout << "Randomizing Values..." << endl;
           readerror = clEnqueueSVMMap(commandQueue, CL_TRUE, CL_MAP_WRITE, nodes_randvalues, sizeof(float)*(numofnodes), 0, NULL, NULL);
           if(readerror != CL_SUCCESS) cout << "Error nodes_randvalues are not mapped to rand them!" << get_error_string(readerror) << endl;

           for(int i = 0; i < numofnodes; i++)
           {
               nodes_randvalues[i]= static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/40));
               //printf("randvalues[%d] = %.5f\n",i,nodes_randvalues[i]);
           }
           readerror = clEnqueueSVMUnmap(commandQueue,nodes_randvalues, 0, NULL, NULL);
           if(readerror != CL_SUCCESS) cout << "Error for unmapping nodes_status!" << get_error_string(readerror) << endl;
          
           printf("Randed. Now calling Execution Kernel...\n");
          
           //---------guide::: call execution kernel 
           status = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, &threadnumber, &localThreads, 0, NULL, NULL);
           if (status != CL_SUCCESS) cout << "Error kernel call! = " << get_error_string(status) << endl;
           else cout << "Execution kernel sent!" <<endl;       
           
           status = clFlush(commandQueue);
           if(status != CL_SUCCESS) cout << "Error Flushing First = " << get_error_string(status) << endl;
           else cout << "Flush sent after execution successfully!" << endl;


           //---------guide::: call dispatch kernel 
           status = clEnqueueNDRangeKernel(commandQueue, kernel_deactivate, 1, NULL, &threadnumber, &localThreads, 0,NULL,NULL);
           if (status != CL_SUCCESS) cout << "Error kernel_deactivate call! = " << get_error_string(status) << endl;
           else cout << "Deactivation kernel sent!" << endl;

           status = clFlush(commandQueue);
           if (status != CL_SUCCESS) cout << "Error flushing second  = " << get_error_string(status) << endl;
           else cout << "Flush sent after deactivation succesfully!" << endl;	

           //		status = clFinish(commandQueue);
           //			if(status != CL_SUCCESS) cout << "Error in finishing!" << endl;		
           //			else cout << "Finished clFinish okayly!" << endl;
           
          //---------guide:::  map gpu_remaining node again and set lock
           status = clEnqueueSVMMap(commandQueue, CL_TRUE, CL_MAP_READ, gpu_remainingnodes, sizeof(int), 0, NULL, NULL);
           if(errormap != CL_SUCCESS) cout << "Error for reading back" << get_error_string(errormap) << endl;
           else locked = 1;
           printf("GPU Remaining Nodes = %d\n", *gpu_remainingnodes);
       }

       
    //---------guide:::  writing to the output
	cout << "write to file before" << endl;
    //writing the random values in the log file 
    writeToFileNodeInfo(nodes_status_parallel, nodes_randvalues, numofnodes,logFileName, "all");
    showNodesInfo(nodes_status_parallel, nodes_randvalues, numofnodes, "all");

/*
#if DEBUG2
	for(int k=0 ; k<numofnodes; k++)
		if(counter_gpu[k] != 0)
			printf("Counter is not zero at '%d' with value '%d'\n",k,counter_gpu[k]);
#endif	
*/	
    
    //---------guide::: done with kernels
	clFinish(commandQueue);
	errormap = clEnqueueSVMMap(commandQueue, CL_TRUE, CL_MAP_READ, nodes_randvalues, sizeof(float)*numofnodes, 0, NULL, NULL);
		if(errormap != CL_SUCCESS) cout << "Error for reading back" << get_error_string(errormap) << endl;
	errormap = clEnqueueSVMMap(commandQueue, CL_TRUE, CL_MAP_READ, nodes_status_parallel, sizeof(int)*numofnodes, 0, NULL, NULL);
		if(errormap != CL_SUCCESS) cout << "Error for reading back" << get_error_string(errormap) << endl;
	errormap = clEnqueueSVMMap(commandQueue, CL_TRUE, CL_MAP_READ, nodes_execute, sizeof(int)*numofnodes, 0, NULL, NULL);
		if(errormap != CL_SUCCESS) cout << "Error for reading back" << get_error_string(errormap) << endl;
			
	cout << "End of the execution, clFinish has been satisfied!" << endl;
	
     //---------guide:::  printing the results
	for(int l=0; l<numofnodes; l++)
	{
		printf("Rand[%d]= %.5f -- ",l,nodes_randvalues[l]);
		printf("Status[%d] = %d",l,nodes_status_parallel[l]);
		printf("Execute[%d] = %d\n",l,nodes_execute[l]);
	}

    writeToFileResult(nodes, index_array, nodes_status_parallel, numofnodes, logFileName);

	
    errormap = clEnqueueSVMUnmap(commandQueue,nodes_randvalues, 0, NULL, NULL);
		if(errormap != CL_SUCCESS) cout << "Error for reading back" << get_error_string(errormap) << endl;
	errormap = clEnqueueSVMUnmap(commandQueue,nodes_status_parallel, 0, NULL, NULL);
		if(errormap != CL_SUCCESS) cout << "Error for reading back" << get_error_string(errormap) << endl;
	
    cl_event SMVMapEvt;
    errormap = clEnqueueSVMUnmap(commandQueue, nodes_execute, 0, NULL, &SMVMapEvt);
		if(errormap != CL_SUCCESS) cout << "Error for reading back" << get_error_string(errormap) << endl;

    status = waitForEventAndRelease(&SMVMapEvt); //can use this line to wait for events
    
    //writing the result to the output 
    write_output(outFilename , nodes_status_parallel, numofnodes);
    //cout <<"whatever! it works\n"<<endl; 
    //---------guide::: free everything
    clSVMFree(context, nodes);
    clSVMFree(context, nodes_randvalues);
    clSVMFree(context, gpu_remainingnodes);
    clSVMFree(context, nodes_status_parallel);
    clSVMFree(context, counter_gpu);
    clSVMFree(context, nodes_execute);
    clSVMFree(context, index_array);
    status = clReleaseKernel(kernel);
    status = clReleaseKernel(kernel_deactivate);
    status = clReleaseProgram(program);
    status = clReleaseProgram(program_deactivate);
    status = clReleaseCommandQueue(commandQueue);
    status = clReleaseContext(context);
    cout <<"here! it works\n"<<endl; 
    

	return 0;
}
