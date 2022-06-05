#include <thread>
#include <iostream>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <atomic>

class graph
{
	graph() :_counter(0) {};
private:
	std::atomic<int> _counter;
};