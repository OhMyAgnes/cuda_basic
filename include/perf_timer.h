#ifndef _PERF_TIMER_H
#define _PERF_TIMER_H

#include <iostream>
#include <sstream>

#ifdef _WIN32

#define NOMINMAX
#include <windows.h>

class PerfTimer
{
private:
	LARGE_INTEGER startCount;//记录开始时间
	LARGE_INTEGER endCount;//记录结束时间
	LARGE_INTEGER freq;//本机CPU时钟频率
	long long leftover; //暂停时间
	double dbTime;//程序运行的时间保存在这
	
	std::string str;
	bool smartType;
	
public:
	TimerCounter(void)//构造函数
	{
		smartType = false;
		QueryPerformanceFrequency(&freq);//获取主机CPU时钟频率
		leftover = 0;
	}
	
	TimerCounter(const char * input)//构造函数
	{
		smartType = true;
		str = input;
		QueryPerformanceFrequency(&freq);//获取主机CPU时钟频率
		leftover = 0;
		
		Start();
	}
	
	~TimerCounter(void) //析构函数
	{
		if (smartType)
		{
			Stop();
			ostringstream os;
			os << str << ": " << GetElapsedTime() << "ms" << endl;;
			printf(os.str().data());
		}
	}
	
	void Start()//被测程序开始点处开始计时
	{
		QueryPerformanceCounter(&startCount);//开始计时
	}
	
	void Stop()//被测程序结束点处结束计时
	{
		QueryPerformanceCounter(&endCount);//停止计时
		dbTime=(leftover + (double)endCount.QuadPart - (double)startCount.QuadPart) / (double)freq.QuadPart;//获取时间差
		
		leftover = 0;
	}
	
	void Pause()
	{
		QueryPerformanceCounter(&endCount);
		leftover += (double)endCount.QuadPart - (double)startCount.QuadPart;
	}
	
	double GetElapsedTime() const //返回时间 单位ms
	{
		return dbTime * 1000;   //ms
	}
};

#else
#include <sys/time.h>

const long long NANO_PER_SEC = 1000000000LL;
const long long MICRO_TO_NANO = 1000LL;

class PerfTimer
{
private:
	long long startCount;//记录开始时间
	long long endCount;//记录结束时间
	long long leftover; //暂停时间
	long long dbTime;//程序运行的时间保存在这
	
	std::string str;
	bool smartType;
	
	long long _getTime()
    {
        struct timeval tv;
        long long ntime;

        if (0 == gettimeofday(&tv, NULL))
        {
            ntime  = NANO_PER_SEC;
            ntime *= tv.tv_sec;
            ntime += tv.tv_usec * MICRO_TO_NANO;
        }

        return ntime;
    }
	
public:
	TimerCounter(void)//构造函数
	{
		smartType = false;
		leftover = 0;
	}
	
	TimerCounter(const char * input)//构造函数
	{
		smartType = true;
		str = input;
		leftover = 0;
		
		Start();
	}
	
	~TimerCounter(void) //析构函数
	{
		if (smartType)
		{
			Stop();
			ostringstream os;
			os << str << ": " << GetElapsedTime() << "ms" << endl;;
			printf(os.str().data());
		}
	}
	
	void Start()//被测程序开始点处开始计时
	{
		startCount = _getTime();//开始计时
	}
	
	void Stop()//被测程序结束点处结束计时
	{
		endCount = _getTime();//停止计时
		dbTime = leftover + endCount - startCount;//获取时间差
		
		leftover = 0;
	}
	
	void Pause()
	{
		endCount = _getTime();//停止计时
		leftover += endCount - startCount;
	}
	
	double GetElapsedTime() const //返回时间 单位ms
	{
		return ((double) dbTime) / NANO_PER_SEC * 1000;   //ms
	}
};

#endif // _WIN32

#endif // _PERF_TIMER_H