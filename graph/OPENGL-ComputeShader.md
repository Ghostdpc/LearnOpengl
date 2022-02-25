#### 概述

computeshader是一个在管线中没有固定的执行位置的一个shader，但是，它也能执行一个普通shader的各种操作。另外，它没有固定的输入和输出，所有的默认输入通过一组内置变量来传递，我们可以通过操作内存，来做各种各样的操作。当然，其副作用是图像存储，原子操作，以及对原子计数器的访问

computeshader的执行取决于什么时候提交执行执行。在opengl中，使用`gldispatchcompute`函数来进行提交。而它的创建过程则和普通的shader差不多，通过`glCreateShader`创建，通过`glCompileShader`编译，通过`glAttachShader`绑定，通过`glLinkProgram`连接。需要注意的是，使用了computeshader的program，不可以和正常的vs,fs的program一起混用

#### 工作组相关

由于GPU的运行通过大量的并行来获得性能上的提高，computeshader也是并行运行的，在opengl中，我们用工作组（workgroup）的概念来代称（在dx或者是其他的说法中可能是线程组），假设一个工作组有4*4个工作单元，可以简单理解为一个有16个线程在并行运行。多个工作组可以组合成一个全局工作组（global_workgroup），我们通过着色器代码来声明工作组的大小 

```glsl
layout(local_size_x =16,local_size_y =16,local_size_x =1)
```

这些大小分为x,y,z三个维度，默认值都为1。而全局工作组则是在执行的时候指定大小,同样也是三个维度

```c++
glDispatchCompute(num_groups_x = 16,num_groups_y = 1,num_groups_z = 1)
```

在同一个全局工作组中，可以通过内置变量知道每个本地工作组的相对坐标，大小等信息，在每个本地工作组的工作单元之间，可以通过变量和显存进行通讯，同时也可以执行同步操作保持一致性。

computeshader中的内置变量

```glsl
const uvec3 gl_WorkGroupSize;   // 本地工作组大小，常数
in uvec3 gl_NumWorkGroups;   // glDispatchCompute 传递的参数x,y,z
in uvec3 gl_LocalInvocationID;  //当前执行单元在本地工作组中的位置 range uvec3(0)~gl_WorkGroupSize-uvec3(1)
in uvec3 gl_WorkGroupID;  // 本地工作组在全局工作组中的位置 uvec3(0)~gl_WorkGroupSize-uvec3(1)
in uvec3 gl_GlobalInvocationID;  // 当前执行单元在全局工作组中的位置
in uint gl_LocalInvocationIndex;  
```

#### 通讯

我们可以使用 **shared关键字** 来声明着色器中的变量，其格式与其它的关键字，例如uniform、in、out等类似

```glsl
shared uint fool;  
// 一个共享的向量数组  
shared vec4 bar[128];  
// 一个共享的数据块  
shared struct baz_struct  
{  
    vec4 a_vector;  
    int an_integer;  
    ivec2 an_array_of_integers[27];  
}baz[42];
```

如果一个变量被声明为shared，那么它将被保存到特定的位置，从而 **对同一个本地工作组内的所有计算着色器请求可见** 。如果某个计算着色器请求对共享变量进行写入，那么这个数据的修改信息将最终 通知 给 同一个本地工作组 的所有着色器请求。在这里我们用了“最终”这个词，这是因为各个着色器请求的执行顺序并没有定义，就算是同一个本地工作组内也是如此。

#### 同步

同步命令的类型有两种 。首先是 运行屏障 （ execution barrier ），可以通过 barrier ()函数触发。 它与细分控制着色器中的barrier()函数类似 ，后者可以用来实现控制点处理过程中的请求同步。如果计算着色器的一个请求遇到了barrier()，那么它会停止运行，并等待同一个本地工作组的所有请求到达为止。当请求从barrier()中断的地方重新开始运行的时候，我们可以断定其它所有的请求也已经到达了barrier()，并且在此之前的所有操作均已经完成。

例如在上面的例子中。、，本地工作组内进行请求间的通信，那么可以在一个请求中写入共享变量，然后在另一个请求中读取。但是，我们 **必须** 确定目标请求中读取共享变量的时机，即在源请求已经完成对应的写入操作之后。为了确保这一点，我们可以在源请求中写入变量，然后在两个请求中 同时执行barrier ()函数。当目标请求从barrier()返回的时候，源请求必然已经执行了同一个函数（也就是完成共享变量的写入），因此可以安全地读取变量的值了。

第二种类型的同步叫做 内存屏障 （ memory barrier ）。内存屏障的最直接的版本就是 **memoryBarrier** ()。如果调用memoryBarrier()，那么就可以 保证 着色器请求内存的写入操作一定是提交到内存端，而不是通过缓冲区（cache）或者调度队列之类的方式。 所有发生在memoryBarrier()之后的操作在读取同一处内存的时候，都可以使用这些内存写入的结果，即使是同一个计算着色器的其它请求也是如此 。

#### 数据的传输和共享

这部分内容是重中之重，，之后补充

#### 踩过的坑

1. 在实际写代码的过程中，由于使用的是learnopengl的代码框体，因此glfw设置的版本特别低，所以导致computeshader完全没法使用，因此查了许久，这一点还是很坑的，learnopengl采用的教程是3-3，而computeshader需要的版本是4-3。

```c++
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
```

2. 关于ssbo，ssbo在vulkan的gpu例子中被广泛的使用到了，但是在手机端，mali平台下，是没办法使用ssbo这一特性的，因此需要用其他方式进行数据的传递。mali平台同时也会有分离模式没法正常使用的问题