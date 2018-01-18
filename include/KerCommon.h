#ifndef _KER_COMMON_H
#define _KER_COMMON_H

#ifndef __CUDACC__
#define __CUDACC__
#include <device_launch_parameters.h>
#include <device_functions.h>
#undef __CUDACC__
#endif

#define BLOCK_DIM    blockDim.x
#define THREAD_IDX   threadIdx.x

const int MeanVertDegree = 8; 

__forceinline__ __device__ int getCurThreadIdx()
{
    const int threadsPerBlock   = blockDim.x;
    const int curThreadIdx      = ( blockIdx.x * threadsPerBlock ) + threadIdx.x;
    return curThreadIdx;
}

__forceinline__ __device__ int getThreadNum()
{
    const int blocksPerGrid     = gridDim.x;
    const int threadsPerBlock   = blockDim.x;
    const int threadNum         = blocksPerGrid * threadsPerBlock;
    return threadNum;
}

#endif // _KER_COMMON_H