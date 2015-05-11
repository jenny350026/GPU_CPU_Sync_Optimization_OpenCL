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
__kernel void simple_program(__global int* firstInput,__global int* bufferSize)
{
	 int num = get_global_id(0);
     if (num < *bufferSize)  {
       firstInput[num] =10;
     }
}

__kernel void simple_program_wait(__global int* firstInput,__global int* bufferSize)
{
#pragma OPENCL EXTENSION cl_amd_printf : enable
int num = get_global_id(0);
if (num < *bufferSize)  {
         while(firstInput[num] < 10*num) { 
             //cout<<"here is the value inside opencl. num:" << num << " and arrval:"<<firstInput[num] << endl;
         printf("%d\n", num);  
        }        
        firstInput[num] =2;
     }
}

__kernel void simple_program_2(__global int* firstInput)
{
	 int num = get_global_id(0);
     if (num < 5)  {
       firstInput[num] +=1;
     }
}



