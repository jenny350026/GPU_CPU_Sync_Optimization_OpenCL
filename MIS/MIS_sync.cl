__kernel void mis_parallel_async(__global int* counter,__global int *nodes, __global float *nodes_randvalues, __global int *nodes_status, __global int* indexarray, __global int* execute) {
    
	int i = get_global_id(0);
	execute[i] = 1;

	int numofneighbour = indexarray[i+1] - indexarray[i];
	
	if(nodes_status[i] == 1 )
	{	 
       		for(int k = 0; k < numofneighbour; k++)
		{
       
			if(nodes_status[nodes[indexarray[i] + k]] == 1 && nodes_randvalues[i] > nodes_randvalues[nodes[indexarray[i] + k]]) 
            		{
                	execute[i] = 0;
                	break;
            		}
        	}   
	}	
    	else
        	execute[i] = 0;


}

__kernel void deactivate_neighbors(__global int *nodes, __global float *nodes_randvalues, __global int *nodes_status, __global int *remaining_nodes, __global int* indexarray, __global int* execute) {
    int i = get_global_id(0);

    
    int numofneighbour = indexarray[i+1] - indexarray[i];

    if (execute[i] == 1)
    {
        nodes_status[i] = 2;  // select myself
        atomic_dec(remaining_nodes);  // remove myself from active node count
        for(int k = 0 ; k < numofneighbour; k++)
        {
            if( atomic_xchg(&nodes_status[nodes[indexarray[i] + k]], 0) )
            {
                atomic_dec(remaining_nodes);
            }
       	}
    }
}
