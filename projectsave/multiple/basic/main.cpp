#include"MyThread.h"
#include<Windows.h>
#include <sstream>
threadsafe_queue<int> queue;
std::condition_variable condition;
std::atomic<bool> job_shared(false);


void runA()
{
	for (int idx = 0; idx < 10; idx++)
	{
		for (int j = 0; j < 20; j++)
		{
			int num = rand();
			std::ostringstream ostr;
			queue.push(num);
			ostr << "ThreadA: Group " << idx+1 << " num " << j+1 << " is " << num << "\n";
			//生成随机数
			std::cout << ostr.str();
			Sleep(1);
		}
		condition.notify_one();
	}
	std::cout << "A End" << std::endl;
}

void runB()
{
	int runNum = 0;
	while (runNum<10)
	{
		std::mutex m;
		std::unique_lock<std::mutex> lk(m);//3
		condition.wait(lk, [] {return queue.size() >= 20; });
		for (int idx = 0; idx < 20; ++idx)
		{
				std::ostringstream ostr;
		 		auto data = queue.pop();
				ostr << "ThreadB Group " << runNum+1 << " num "<<idx+1<<" is " << *data<<"\n";
				std::cout << ostr.str();
		 		Sleep(1);
		 }
		runNum++;
		lk.unlock();
	}
	std::cout << "B End" << std::endl;

}

int main() {
	MyThread A;
	MyThread B;
	// ThreadB* B = new ThreadB();
	A.setRunFunc(runA);
	B.setRunFunc(runB);
	A.start();
	B.start();
	std:: cout<< "all started " << std::endl;
	
	return 0;
}
