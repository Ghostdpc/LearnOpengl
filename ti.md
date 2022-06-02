设计一个带有两个线程的系统，线程a不断生成随机数，每创建20个，就传入到B中，B受到随机数之后，输出从线程a收到的随机数

要求线程a产生的随机数和线程B输出随机数不能互相干扰（AB同时会有输出）

```c++
class CThreadA
{
    void Run() {
        //循环10次
        for(idx in 10){
            for (j in 20){
                //生成随机数
                cout<<("ThreadA:%dm%dn",idx,num);
                sleep(1)
                
            }
            //传递给B
        }
    }
}
class CThreadB
{
  void Run() {
      while(!线程结束)
      {
          //接收数据
          for(j in 20){
              print
                  sleep(1)
          }
      }
  } 
}
```

进阶

加一个线程C，这个线程独立运行，每当B收到A传来的数据，创建任务让C进行计算方差，C计算好之后，B在输出，B不能阻塞等待C

附加题

完成简单的TaskGraph

使得node0,node1,node2执行完后执行node3,再执行node4,5,6,要求123并发，456并发

```c++
int main(){
    graph
    node = addnode
    AddConnection(node0,node3)
    AddConnection(node5,node3) 
}
```







调研报告

1. TheDestinyParticleArchitecture  siggraph2017
2. Hashed Alpha testing 