
#include"MyThread.h"
#include <functional>
#include <Windows.h>
#include <random>
#include <atomic>


MyThread::MyThread():
_fp(nullptr)
{
	
}

MyThread::~MyThread()
{
	// _thread.join();
	if (_thread.joinable())
	{
		_thread.join();
	}
}


void MyThread::start()
{
	std::thread th(std::bind(&MyThread::run, this));
	_thread = std::move(th);
}

void MyThread::run()
{
	if (_fp)
	{
		_fp();
	}
}

void MyThread::setRunFunc(void (*fp)())
{
	_fp = fp;
}
