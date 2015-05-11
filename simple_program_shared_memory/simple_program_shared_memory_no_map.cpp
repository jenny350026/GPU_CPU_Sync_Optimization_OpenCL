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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include "CLUtil.hpp"
#define SUCCESS 0
#define FAILURE 1

using namespace appsdk;
using namespace std;

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
	
    /*Step 5: Create program object */
	const char *filename = "simple_program_shared_memory_kernel.cl"; //^^^provide the opencl file here
	string sourceStr;
	status = convertToString(filename, sourceStr);
	const char *source = sourceStr.c_str();
	size_t sourceSize[] = {strlen(source)};
	cl_program program = clCreateProgramWithSource(context, 1, &source, sourceSize, &status);
    CHECK_OPENCL_ERROR(status, "create program with source error");
	
    
    /*Step 6: Build program. */
	int retVal = clBuildProgram(program, 1,devices,NULL,NULL,NULL);
    cout << "this is the retVal " <<retVal << endl; 
   size_t log_size; 
   clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, 0 , NULL, &log_size);
   char *log  = (char*)  malloc(log_size);
    clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, log_size ,log, NULL);
    printf("%s\n",log );
    CHECK_ERROR(retVal,CL_SUCCESS ,"program build failed");
    
    //body of the program where I initialize the buffer	
    cl_int bufferSize = 5 ;
    cl_int *firstInput = new cl_int[bufferSize];
    for (int i = 0; i < bufferSize; i++) {
       firstInput[i] = i;
    }
    //creating a buffer	
    
     //writing into bufferSize buffer 
    cl_mem bufferSizeBuffer= clCreateBuffer(context, CL_MEM_READ_ONLY,  sizeof(cl_int), NULL, &status);
    cl_event writeEvt;
    status = clEnqueueWriteBuffer(
                 commandQueue,
                 bufferSizeBuffer, //the buffername to be added
                 CL_FALSE, //whether blocking or no
                 0, //offset
                 sizeof(cl_int),
                 &bufferSize, //the the buffer in host where the data is gotten from
                 0,
                 NULL,
                 &writeEvt);
    CHECK_OPENCL_ERROR(status, "clEnqueueWriteBuffer(inBuf) failed..");
    //status = waitForEventAndRelease(&readEvt); can use this line to wait for events
    status = clFlush(commandQueue);
     
    
     
    //create a memory object 
    //two different options for zero copy, used MEM_ALLOC_HOST_PTR or CL_MEM_USE_HOST_PTR
    //---------guide:::  use the already known allocated memory
    cl_mem inputBuffer = clCreateBuffer(context, CL_MEM_USE_HOST_PTR, bufferSize*sizeof(cl_int), firstInput, &status);
    //---------guide::: allocated a part of host memory and have the device memory to point at it
    //cl_mem inputBuffer = clCreateBuffer(context,CL_MEM_ALLOC_HOST_PTR,  bufferSize*sizeof(cl_int), NULL, &status);
    CHECK_OPENCL_ERROR(status, "clCreateBuffer failed.(inBuf)");
   
    //---------guide::: mapping memory. Allows CPU to get access to the buffer temporarily, till Unmapp is issued
    //map the memory object, this is done because of the address translation necessary between cpu and gpu,
    //if the device is discrete, this implecitly transfers data over, however in the case of APU devices,
//    cl_event MapEvt;
//    int *mappedData = (int *)clEnqueueMapBuffer(
//            commandQueue,
//            inputBuffer, //the buffername to be added
//            CL_FALSE, //whether blocking or no
//            CL_MAP_READ | CL_MAP_WRITE, //whether mapped for reading or writing
//            0, //offset
//            bufferSize * sizeof(cl_uint),
//            0, //number of events to be implemented before this command 
//            NULL, //event lists
//            &MapEvt, //event name
//            &status);
//    CHECK_OPENCL_ERROR(status, "clEnqueueMapBuffer(inBuf) failed..");
//    //status = waitForEventAndRelease(&readEvt); can use this line to wait for events
//    status = clFlush(commandQueue);
//    //---------guide:::  is necessary to allow cpu manipulate the buffer
//    memcpy(mappedData, firstInput, bufferSize*sizeof(int));  //don't forget to add sizeof 
//    mappedData[0] +=5;  
    

//   //unmapping, so that the device can safely use the buffer
//    cl_event inUnmapEvt;
//    status = clEnqueueUnmapMemObject(
//            commandQueue,
//            inputBuffer,
//            mappedData,
//            0,
//            NULL,
//            &inUnmapEvt);
//    CHECK_OPENCL_ERROR(status, "clEnqueueUnmapMemObject failed. (inputBuffer)");   //writing to a buffer 
//    status = clFlush(commandQueue);
//    cl_event writeEvt;
//    status = clEnqueueWriteBuffer(
//                 commandQueue,
//                 inputBuffer, //the buffername to be added
//                 CL_FALSE, //whether blocking or no
//                 0, //offset
//                 bufferSize * sizeof(cl_uint),
//                 firstInput, //the the buffer in host where the data is gotten from
//                 0,
//                 NULL,
//                 &writeEvt);
//    CHECK_OPENCL_ERROR(status, "clEnqueueWriteBuffer(inBuf) failed..");
//    //status = waitForEventAndRelease(&readEvt); can use this line to wait for events
//    status = clFlush(commandQueue);
//    CHECK_OPENCL_ERROR(status, "clFlush(commandQueue) failed.");
    
    for (int i =0; i<5; i++) {
        firstInput[i] *=2;
    }
    /*Step 8: Create kernel object */
    cl_kernel myKernel = clCreateKernel(program,"simple_program", &status);  //uses the opencl file. create couple if you have a copule of kernels
    CHECK_OPENCL_ERROR(status, "clCreateKernel failed");

    
    /*Step 9: Sets Kernel arguments.*/
	status = clSetKernelArg(myKernel, 0, sizeof(cl_mem), (void *)&inputBuffer);
    CHECK_OPENCL_ERROR(status, "setting kernel arguments failed asdfas"); 
    status = clSetKernelArg(myKernel, 1, sizeof(cl_mem), &bufferSizeBuffer); 
    CHECK_OPENCL_ERROR(status, "setting kernel arguments failed"); 
    // Set Global and Local work items
    //
    size_t globalWorkItems = bufferSize; //total number of threads
    size_t localWorkItems = 64; //number of threads within a work group

     //launching the kernel
     
    cl_event krEvt;
    status = clEnqueueNDRangeKernel(                                                                                                          
                 commandQueue,
                 myKernel,
                 1, //indicates that there is only one dimension
                 NULL,
                 &globalWorkItems,
                 &localWorkItems,
                 0,
                 NULL,
                 &krEvt);
    CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel(counterKernel) failed."); 
    status = waitForEventAndRelease(&krEvt); // if you don't use this ,the kernel would be unblocking
    
    cl_event MapEvt;
    int *mappedData = (int *)clEnqueueMapBuffer(
            commandQueue,
            inputBuffer, //the buffername to be added
            CL_FALSE, //whether blocking or no
            CL_MAP_READ | CL_MAP_WRITE, //whether mapped for reading or writing
            0, //offset
            bufferSize * sizeof(cl_uint),
            0, //number of events to be implemented before this command 
            NULL, //event lists
            &MapEvt, //event name
            &status);
    CHECK_OPENCL_ERROR(status, "clEnqueueMapBuffer(inBuf) failed..");
    //status = waitForEventAndRelease(&readEvt); can use this line to wait for events
    status = clFlush(commandQueue);
    
    
//    /*Step 11: Read the cout put back to host memory.*/
//    cl_event readEvt;
//    status = clEnqueueReadBuffer(
//                 commandQueue,
//                 inputBuffer, //a valid buffer to read from
//                 CL_FALSE,
//                 0,
//                 bufferSize * sizeof(cl_uint),
//                 firstInput, //where to store the data
//                 0,
//                 NULL,
//                 &readEvt);
//    CHECK_OPENCL_ERROR(status, "clEnqueueReadBuffer(counterOutBuf) failed.");
    //inputBuffer[2] =inputBuffer[2]*inputBuffer[2];  
    //firstInput[2] =firstInput[2]* firstInput[2];  
    //status = clFlush(commandQueue);
    //status = waitForEventAndRelease(&readEvt);
    //CHECK_OPENCL_ERROR(status, "waitForEventAndRelease(readEvt) failed.");
    for (int i =0 ;i < bufferSize; i++) {
        cout << "the " << i << "th value of the input is: " << mappedData[i] << endl;

    }
    cout<<"here"<<endl; 
    cout << "done with the program " <<endl;
	/*Step 12: Clean the resources.*/
	status = clReleaseKernel(myKernel);				//Release kernel.
	status = clReleaseProgram(program);				//Release the program object.
	status = clReleaseMemObject(inputBuffer);		//Release mem object.
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

	std::cout<<"Passed!\n";
	return SUCCESS;
}
