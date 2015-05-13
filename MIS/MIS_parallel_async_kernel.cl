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
#include "status.h"
#pragma OPENCL EXTENSION cl_amd_printf : enable
__kernel void simple_program(__global int* buffer, __global atomic_int* atomicBuffer)
{
	int num = get_global_id(0);

    int status = SELECTED;
    
    if( num == 0 )
        printf("%d%s%d\n", num, " ", status);
    
    while (atomic_load_explicit (&atomicBuffer[0], memory_order_acquire, memory_scope_all_svm_devices) != 99);

    buffer[num] += num;

    atomic_store_explicit (&atomicBuffer[num], (100 + num), memory_order_release, memory_scope_all_svm_devices);

}

__kernel void mis_parallel_async(__global int *nodes, __global float *nodes_randvalues, __global int *nodes_status, __global int* indexarray, __global int* execute, volatile __global atomic_int *ready, __global int* node_counters) { //, __global int* node_neighbor_counters) {
    int i = get_global_id(0);
    
    execute[i] = 1;

    int numofneighbour = indexarray[i+1] - indexarray[i];

    if(nodes_status[i] == ACTIVE )
    {   
        //int counter = 0;
        //while (/*counter < 100000000 &&*/ atomic_load_explicit (&ready[i], memory_order_acquire, memory_scope_all_svm_devices) != 1){
        //    //++counter;
        //}

        //node_counters[i] = counter;
        
        for(int k = 0; k < numofneighbour; k++){
            //counter = 0;
            //while (/*counter < 10000000 && */ atomic_load_explicit (&ready[nodes[indexarray[i] + k]], memory_order_acquire, memory_scope_all_svm_devices) != 1){
            //    //++counter;
            //}
            //node_counters[i] = counter;

            if(nodes_status[nodes[indexarray[i] + k]] == ACTIVE && nodes_randvalues[i] > nodes_randvalues[nodes[indexarray[i] + k]]) 
            {
                execute[i] = 0;
                break;
            }
        }   
    }
    else
        execute[i] = 0;
}

__kernel void deactivate_neighbors(__global int *nodes, __global float *nodes_randvalues, volatile __global atomic_int *nodes_status, volatile __global int *remaining_nodes, __global int* indexarray, __global int* execute) {
    int i = get_global_id(0);
    
    int numofneighbour = indexarray[i+1] - indexarray[i];

    if (execute[i] == 1)
    {
        nodes_status[i] = SELECTED;  // select myself
        atomic_dec(remaining_nodes);  // remove myself from active node count
        for(int k = 0 ; k < numofneighbour; k++)
        {
            if( atomic_exchange(&nodes_status[nodes[indexarray[i] + k]], INACTIVE) )
            {
                //nodes_status[nodes[indexarray[i] + k ]] = 0;
                atomic_dec(remaining_nodes);
            }
        }
    }
}



