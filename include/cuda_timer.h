#ifndef _CUDA_TIMER_H
#define _CUDA_TIMER_H
#include "cuda_wrapper.h"
#include "perf_timer.h"

class CudaTimer : public PerfTimer
{
public:
    void Start()
    {
        CudaSafeCall(cudaDeviceSynchronize());
        PerfTimer::Start();
    }

    void Stop()
    {
        CudaSafeCall(cudaDeviceSynchronize());
        PerfTimer::Stop();
    }

    void Pause()
    {
        CudaSafeCall(cudaDeviceSynchronize());
        PerfTimer::Pause();
    }

    double GetElapsedTime()
    {
        return PerfTimer::GetElapsedTime();
    }
};

#endif // _CUDA_TIMER_H