#pragma once

#include <thread>
#include <iostream>
#include <mutex>
#include <queue>
#include <condition_variable>


template<typename T>
class threadsafe_queue
{
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    // std::condition_variable data_cond;

public:
    threadsafe_queue()
    {}

    void push(T new_value)
    {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(std::move(new_value));
    }


    std::shared_ptr<T> pop()  // 3
    {
        std::lock_guard<std::mutex> lock(mut);
        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
        data_queue.pop();
        return res;
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }

	int size() const
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.size();
    }
};

class Thread 
{
public:
	Thread();
	virtual ~Thread();

	void start();
	void run();
    void setRunFunc(void (*fp)());
    //void SetDataSource(std::atomic<bool>* var, threadsafe_queue<int>* thread);

protected:
	std::thread _thread;
    void (*_fp)();
    // threadsafe_queue<int>* _queue;
    // std::atomic<bool>* _condition;
};
;

