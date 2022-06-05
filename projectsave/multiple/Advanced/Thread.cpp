
#include"Thread.h"
#include <functional>
#include <Windows.h>
#include <random>
#include <atomic>


Thread::Thread() :
	_fp(nullptr)
{

}

Thread::~Thread()
{
	// _thread.join();
	if (_thread.joinable())
	{
		_thread.join();
	}
}


void Thread::start()
{
	std::thread th(std::bind(&Thread::run, this));
	_thread = std::move(th);
}

void Thread::run()
{
	if (_fp)
	{
		_fp();
	}
}

void Thread::setRunFunc(void (*fp)())
{
	_fp = fp;
}
