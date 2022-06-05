#include"Thread.h"
#include<Windows.h>
#include <sstream>
threadsafe_queue<int> queue;
threadsafe_queue<int> bcqueue;
// std::condition_variable condition;
std::atomic<int> job_shared(0);
int counter = 1;
std::atomic<bool> bc_shared(0);


void runA();
void runC(int runNum);
void runB();

int main() {
	Thread A;
	Thread B;
	// ThreadB* B = new ThreadB();
	A.setRunFunc(runA);
	B.setRunFunc(runB);
	A.start();
	B.start();
	std::cout << "all started " << std::endl;

	return 0;
}
void runA()
{
	for (int idx = 0; idx < 10; idx++)
	{
		for (int j = 0; j < 20; j++)
		{
			int num = rand();
			std::ostringstream ostr;
			queue.push(num);
			ostr << "ThreadA: Group " << idx + 1 << " num " << j + 1 << " is " << num << "\n";
			//生成随机数
			std::cout << ostr.str();
			Sleep(1);
		}
		job_shared.fetch_add(1);
	}
	std::cout << "A End" << std::endl;
}
void runC()
{
	int temp[20] = {};
	double sum = 0.0;
	for (int idx = 0; idx < 20; ++idx)
	{
		std::ostringstream ostr;
		auto data = queue.pop();
		bcqueue.push(*data);
		sum += *data;
		temp[idx] = *data;
	}
	double average = sum / 20;
	double tempSum = 0.0;
	for (int item : temp)
	{
		tempSum += pow((double)item - average, 2);
	}
	tempSum /= 20;
	std::ostringstream ostr;
	ostr << "ThreadC Group "<< counter<< " End ===============================" << "\n";
	std::cout << ostr.str();
	bc_shared.store(true);
}
void runB()
{
	int runNum = 0;
	std::mutex m;
	std::unique_lock<std::mutex> lk(m);
	while (runNum < 10)
	{
		if (job_shared.load()>0)
		{
			job_shared.fetch_sub(1);
			Thread c;
			c.setRunFunc(runC);
			c.start();
		}
		if (bc_shared.load())
		{
			bc_shared.store(false);
			for (int idx = 0; idx < 20; ++idx)
			{
				std::ostringstream ostr;
				auto data = bcqueue.pop();
				ostr << "ThreadB Group " << runNum + 1 << " num " << idx + 1 << " is " << *data << "\n";
				std::cout << ostr.str();
				Sleep(1);
			}
			counter++;
			runNum++;

		}
	
	}
	std::cout << "B End" << std::endl;

}