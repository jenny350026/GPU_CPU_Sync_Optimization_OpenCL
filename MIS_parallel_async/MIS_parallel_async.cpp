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
#define SUCCESS 0
#define FAILURE 1

using namespace appsdk;
using namespace std;

#define SAMPLE_VERSION "AMD-APP-SDK-v3.0.0.0"
#define OCL_COMPILER_FLAGS  "FineGrainSVM_OclFlags.txt"

/* convert the kernel file into a string */
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
	return FAILURE;
}

int main(int argc, char* argv[])
{

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
    cl_command_queue_properties props = CL_QUEUE_PROFILING_ENABLE;
    cl_command_queue commandQueue = clCreateCommandQueue(context, devices[0], props, &status); //we can set outof order execution here, also profiling of the commands
    CHECK_OPENCL_ERROR(status, "clCreateCommandQueue failed(commandQueue)");
	
    ///*Step 5: Create program object */
	cl_program program;
    
    /*Step 6: Build program. */
    buildProgramData buildData;
    buildData.kernelName = std::string("MIS_parallel_async_kernel.cl");
    buildData.devices = devices;
    buildData.deviceId = 0;
    buildData.flagsStr = std::string("");
    buildData.flagsFileName = std::string(OCL_COMPILER_FLAGS);

	//int retVal = clBuildProgram(program, 1,devices, "-std=c++11",NULL,NULL);
    int retVal = buildOpenCLProgram(program, context, buildData);
    cout << "Retur value from build: " <<retVal << endl; 
    CHECK_ERROR(retVal,CL_SUCCESS ,"program build failed");
    
    //body of the program where I initialize the buffer	
    cl_int bufferSize = 5 ;
    cl_int *firstInput = new cl_int[bufferSize];
    for (int i = 0; i < bufferSize; i++) {
       firstInput[i] = i;
    }

    cl_int* buffer = (int*) clSVMAlloc(context, CL_MEM_SVM_FINE_GRAIN_BUFFER, (bufferSize+1)*sizeof(int), 4);
    CHECK_ALLOCATION(buffer, "Failed to allocate SVM memory. (buffer)");
    *buffer = 0;

    cl_int* atomicBuffer = (int*) clSVMAlloc(context, CL_MEM_SVM_FINE_GRAIN_BUFFER|CL_MEM_SVM_ATOMICS, (bufferSize+1)*sizeof(int), 4);
    CHECK_ALLOCATION(atomicBuffer, "Failed to allocate SVM memory. (atomicBuffer)");
    *atomicBuffer = 0;


	/*Step 8: Create kernel object */
	cl_kernel myKernel = clCreateKernel(program,"simple_program", &status);  //uses the opencl file. create couple if you have a copule of kernels
    CHECK_OPENCL_ERROR(status, "clCreateKernel failed");
    
   /*Step 9: Sets Kernel arguments.*/
	status = clSetKernelArgSVMPointer(
                myKernel, 
                0,
                buffer);
    CHECK_OPENCL_ERROR(status, "setting kernel arguments failed asdfas"); 

    status = clSetKernelArgSVMPointer(
                myKernel, 
                1, 
                atomicBuffer); 
    CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 
    // Set Global and Local work items
    //
    size_t globalWorkItems = bufferSize; //total number of threads
    size_t localWorkItems = 64; //number of threads within a work group

     //launching the kernel
    cl_event ndrEvt;
    status = clEnqueueNDRangeKernel(
                 commandQueue,
                 myKernel,
                 1, //indicates that there is only one dimension
                 NULL,
                 &globalWorkItems,
                 &localWorkItems,
                 0,
                 NULL,
                 &ndrEvt);
    CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel(counterKernel) failed."); 

    status = clFlush(commandQueue);
    CHECK_OPENCL_ERROR(status, "clFlush failed.(commandQueue)");
	
    /*Step 11: Read the cout put back to host memory.*/
    //cl_event readEvt;
    //status = clEnqueueReadBuffer(
    //             commandQueue,
    //             inputBuffer, //a valid buffer to read from
    //             CL_FALSE,
    //             0,
    //             bufferSize * sizeof(cl_uint),
    //             firstInput, //where to store the data
    //             0,
    //             NULL,
    //             &readEvt);
    //CHECK_OPENCL_ERROR(status, "clEnqueueReadBuffer(counterOutBuf) failed.");
    //status = clFlush(commandQueue);
    //status = waitForEventAndRelease(&readEvt);
    //CHECK_OPENCL_ERROR(status, "waitForEventAndRelease(readEvt) failed.");
    for (int i =0 ;i < bufferSize; i++) {
        buffer[i] = 64;
    }

    std::atomic_store_explicit ((std::atomic<int>*)&atomicBuffer[0], 99, std::memory_order_release);

    status = waitForEventAndRelease(&ndrEvt);
    CHECK_ERROR(status, SDK_SUCCESS, "WaitForEventAndRelease(ndrEvt) Failed");
    
    int passed = 1;
    
	for (cl_uint i=0;i<bufferSize;i++){
        std::cout << buffer[i] << endl;
		if (buffer[i] != (64+i))
			passed = 0;
    }


	/*Step 12: Clean the resources.*/
	status = clReleaseKernel(myKernel);				//Release kernel.
	status = clReleaseProgram(program);				//Release the program object.
	//status = clReleaseMemObject(inputBuffer);		//Release mem object.
	//status = clReleaseMemObject(outputBuffer);
	status = clReleaseCommandQueue(commandQueue);	//Release  Command queue.
	status = clReleaseContext(context);				//Release context.

//	if (output != NULL)
//	{
//		free(output);
//		output = NULL;
//	}



	if (devices != NULL)
	{
		free(devices);
		devices = NULL;
	}

    if(passed)
	    std::cout<<"Passed!\n";
    else
	    std::cout<<"Failed!\n";
	return SUCCESS;
}
