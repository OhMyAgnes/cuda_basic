#ifndef _CUDA_TIMER_H
#define _CUDA_TIMER_H
#include "cuda_wrapper.h"
#include "perf_timer.h"

class CudaTimer : public PerfTimer
{
public:
    void start()
    {
        CudaSafeCall(cudaDeviceSynchronize());
        PerfTimer::Start();
    }

    void stop()
    {
        CudaSafeCall(cudaDeviceSynchronize());
        PerfTimer::Stop();
    }

    void pause()
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