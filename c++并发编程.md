### 一、并发编程

#### 多进程并发

此种并发方式多个进程，他们在同一时刻运行，独立的进程在同一时刻运行，可以通过信号，套接字，文件，管道等方式进行进程间的通信，这些通信机制要么设置复杂，要么速度慢。原因是操作系统会避免一个进程修改另外一个进程的数据，因此提供一些保护措施，还有一个缺点就是启动进程本身的开销，操作系统需要内部资源管理进程等。但是优点是容易编写更安全的并发代码，并且可以使用远程连接。

#### 多线程并发

另外一种方式是多线程并发，指在单个进程中运行多个线程，线程很像轻量级的进程，每个线程独立运行，可以在不同指令序列中运行。但是，进程中的所有线程都共享地址空间。指针，对象的引用，数据可以在线程中传递。如果数据要被多个线程访问，程序员必须确保每个线程访问的数据是一致的。由于多线程开小更小，因此会更加受到欢迎

#### 使用并发的原因

**关注点分离（soc）**和性能

### 二、线程管理基础

主要内容包括

- 产生线程
- 管理线程
- 线程唯一标识符

#### 2.1产生线程

##### 初始化线程

每个程序都至少有一个线程，例如说我们的main函数就会有一个线程。如果我们想要启动一个线程，可以通过创建并启用一个线程对象。如下

```c++
void dosth();
std::thread mythread(dosth);
```

`std::thread`库可以通过调用类型构造，将带有函数调用符类型的实例传入到`std::thread`中，例如

```c++
class bg_task()
{
 public:
    void operator()() const
    {
        dosth();
        //....
    }
}
bg_task task;
std::thread mythread(task);
```

需要注意的是，这里传递给thread的函数对象会复制到新线程的存储空间中，因为这个对象的拷贝需要和袁术的对象保持一致，不然会导致结果不符合预期。另外，还需要注意的一点就是，传递函数对象时应该注意解析成了函数声明

```c++
std::thread my_thread(bg_task());
```

这种就成了声明一个返回值为thread的函数。。很神秘。这里的解析是，声明了my_thread函数，函数带有一个参数（函数指针指向没有参数并返回一个bg_task对象的函数），这个返回一个std::thread对象。

避免这个方法，可以使用**多组括号**或者是**不同的初始化语法**

```c++
std::thread my_thread((bg_task());
std::thread my_thread{bg_task()};
```

##### 给线程函数传递参数

给线程函数传递参数的操作很简单，但是需要注意的一点是，参数的拷贝。例如

```c++
void f(int i ,std::string const& s);
std::thread t1(f,3,"hello");
char buffer[1024];
std::thread t2(f,3,buffer);
std::thread t3(f,3,std::string(buffer));
```

其中，t1对`char *`进行了向string的转化，但是t2中的buffer是一个指针变量，指向本地变量，本地变量通过buffer传递到线程中。这可以会导致一些未定义的行为。因为需要隐式转换将这个值转换为期望类型的字符串对象。所以正确的做法是t3的做法。在thread构造函数之前就转换为string对象，这样可以避免可能出现的悬垂指针问题

另外一个情景就是，想要传递一个引用，例如说，假设我们想要使用线程更新一个引用传递的数据结构，例如

```c++
void update_data_ref(int id,datacls& data);
\\...
std::thread t(update_data_ref,1,data);
```

虽然函数`update_data_ref`的初衷是传递一个data的引用并更新它，但是thread的构造函数并不这么想，它会**无视期待的函数类型，并且盲目的拷贝已提供的变量**。在上述代码中，参数会以右值的方式进行拷贝传递，然后以右值为参数调用更新函数，但是函数期望的是一个非常量引用作为参数，所以会编译出错，因此需要用下面的形式

```c++
std::thread t(update_data_ref,1,std::ref(data));
```

这样，函数就会收到一个data变量的引用，而不是data变量拷贝的引用。

>据说上述的方式和标准库std::bind的传参形式类似

 另外，我们还可以传递一个成员函数作为线程函数，与之对应的是，我们需要提供一个合适的对象指针作为第一个参数，然后后面跟着的是函数

```c++
class X;
void X::dosth(int param);
X my_X;
std::thread t(&::dosth,&my_X,param);
```

事实上这种写法让我想到了python的self

上述中提到的对象参数可以进行move操作，但是不能进行拷贝操作。类似于uniqueptr。如下面的代码就展示了move一个对象到线程中去

```c++
void process_object(std::unique_ptr<object>);
std::unique_ptr<object>p (new object);
std::thread t(process_obj,std::move(p));
```

原书中这段讲的不明不白的，这里核心还是在于，可以使用move函数，将对象转移到线程中去。进行一个资源占有权的转换

#### 2.2 管理线程

##### 等待线程

在我们创建完一个线程之后，实际上，主线程会继续执行，而当主线程执行完毕后，新线程可能还在运行。例如说我们新线程中的一个函数引用了主线程中的一个变量，而当主线程完成后，这个变量被销毁了，那就有可能访问已经销毁的变量。处理这种情况的常规方法就是将数据复制到线程中，如果我们使用前面提到的move对象到线程中，对于对象中包含的指针或者引用则需要十分谨慎。另外我们可以使用一个join函数，来确保线程在函数完成前结束，例如我们在main中定义了一个线程，使用join函数来确保它在main函数结束前结束，

```c++
void dosth();
std::thread mythread(dosth);
mythread.join();
```

join函数是粗暴的等待线程完成或者不等待，当我们需要对等待中的线程有更灵活的控制是，需要使用一些其他的机制。另外，由于join函数会清理线程相关的存储部分，**因此每个线程只能使用一次join**，一旦使用后，joinable判断就会返回false，即不能再join了

##### 特殊情况下的等待

如果要对一个还没有销毁的线程等待或者分离，可以直接调用join或者detach函数，

> 测试了一下发现，detach后就没法join了，和join一次，也是一生一次）

这里需要注意一种情况，即调用join之前就发生了错误，导致join失败。一般情况下我们有两种方法来规避这个问题，第一是在trycatch中调用

```c++
std::thread t();
try{
    dosth()
}catch()
{
    t.join();
}
t.join;
```

另外一种方式，则是使用被称为**“资源获取即初始化方式”（RAII Resource Acquisition Is Initialization）**的方法，并且提供一个类，在析构函数中使用join

```c++
class thread_guard
{
    std::thread &t;
    public:
    \\...constructor
    ~thread_guard(){
        if (t.joinable()){
            t.join();
        }
    }
    \\... otherfunc
}
```

这是一种十分巧妙的方式，它能够简化上面提到的方法

```c++
thread_guard t();
\\.. construct t
dosth();
```

和上面提到的方法类似，我们假设dosth中出现了报错，因此我们就会开始析构函数中的变量，当我们析构到t时，自动调用了join函数，因此会等待线程运行完

另外这个类的定义中，拷贝构造函数和拷贝赋值操作会被定义为delete，这个涉及到不让编译器自动生成他们，从而阻止thread_guard对象的赋值

如果不想等待的话，我们可以调用detach操作。

##### 后台运行线程

前面我们提到了detach操作，这个操作执行后会让线程在后台运行，这就意味着主线程不能与之产生直接交互。也就是说，不会等待这个线程结束，也就不能有thread对象引用它。分离线程退出时，运行库保证相关资源能够正确回收。

通常情况下，我们称分离线程为守护线程。现成的生命周期可能从应用开始一直到结束。

另外和join函数一样，一个函数能join就能detach

##### 线程所有权的转移

上面说到了move操作，thread本身也有move操作。C++标准库中有很多资源占有类型，例如`std::ifstream,std::unique_ptr,std::thread`类型都是可移动，但是不可拷贝的，这就说明执行线程的所有权可以再线程实例中移动。

```c++
std::thread t1(somefunc);
std::thread t2 = std::move(t1);
std::thread t3;
t3 = std::move(t2);
```

上面的操作中，通过move函数，创建的线程的所有权从t1转移倒了t3，而在move过后，这个线程就和原来的thread对象没有任何关系了，例如说，在move给t1之前，我们可以调用join，但是move过后就不行了

当然需要注意的是，如果你当前的thread对象已经有绑定一个线程了，那也是不能move给他的，会导致一个崩溃报错

可以move操作是thread可以作为函数的参数传递。

我们可以使用一些move敏感的容器（vector）来量产一些线程并等待其结束

##### 运行时决定线程数量

在运行时，我们可以通过`std::thread::hardwar_concurrency`函数来返回能并发在一个程序中的线程数量，在多核系统中，返回值可以使CPU核芯的数量，当系统信息无法获取是，函数会返回0

运行时需要的线程数量应该是一个需要计算得到的结果，我们不能对太少的操作开太多的线程

#### 2.3 标识线程

线程的标识类型为`std::thread::id`，并且可以通过两种方式获取，例如通过thread的函数`get_id`，或者在当前线程中调用`std::this_thread::get_id`

>需要注意的一点是，join和detach后，你都无法获取到它的id了

当两个线程标识符相同，那他们就是同一个线程，或者都是无线程，不相等就是两个不同的线程

### 三、线程间共享数据

本章节的主要内容主要分为三点

1. 共享数据带来的问题
2. 使用互斥量保护数据
3. 数据保护的替代方案

#### 3.1 共享数据带来的问题

在多线程之间共享数据时，当共享数据只是只读的时候，一般不会有什么问题。因为操作不影响数据，也不会涉及对数据的修改。但是当一个或者多个线程需要修改共享数据时，就会产生很多麻烦，这种情况下，就要小心谨慎。

书中提到了一个概念，被称为不变量，但是我倒是没有看到什么特别奇特的地方。

> **不变量（invariants）**通常会在一次更新中被破坏，例如说在双向链表中，不变量就是指节点A中指向下一个节点B的指针，以及节点A中的指向前一个量的指针。当我们从列表中删除一个节点是，两边的节点的指针都要更新，当两边都更新完成后，不变量就又稳定了

##### 条件竞争

条件竞争（Race condition）的）官方说法是“发生在多个线程同时访问同一个共享代码、变量、文件等没有进行锁操作或者同步操作的场景中。” 以电影院买票为例，你所能买的票的位置取决于购买方式的相对顺序。在并发编程中，当上面提到的不变量遭到破坏时，就会产生条件竞争。C++标准中定义了数据竞争（data race）这个术语，这是一个恶性 的条件竞争，在这个行为中，我们会并发的去修改一个独立的对象。这会导致未定义的行为。

恶性条件竞争通常发生于对多于一个的数据块进行修改时，例如说我们在一个线程正在进行数据的修改时，另外一个线程就对数据进行了访问。这种错误很难查找，也很难复现。在高负载的情况下可能会容易出现（因为是时间敏感型）

##### 避免恶性条件竞争

一种思路是对数据结构采取某种保护机制，确保其他访问的线程只能看到未修改和修改后的状态。例如使用互斥量。另外一个思路就是对数据结构的设计进行修改，即所谓的无锁编程。**这种方式下，不论是内存模型的细微差异，还是访问线程数据的能力，都对其有非常重大的影响**。此外还有一种思路，就是使用事务的方式去处理数据结构的更新，所需要的一些数据和读取都存储在事务日志中，然后将之前的操作合为一步，然后再进行提交。当数据结构被另外一个线程修改后，或者处理已经重启的情况下，提交就会无法进行。这被称为**软件事务内存（software transactional memory（STM））**

#### 3.2 使用互斥量来保护数据

使用互斥量的思路来保护共享数据，会在访问共享数据前，将数据锁住，然后再访问结束后，再将数据解锁。线程库需要保证当一个线程使用特定互斥量锁住共享数据是，其他线程想要访问锁住的数据都必须要等到之前的线程解锁后，才能访问，互斥量本身的问题是可能会造成死锁，或者使用不当导致降低了效率

##### 使用互斥量

c++中可以通过`std::mutex`来创建互斥量实例，通过对成员函数lock()来进行上锁，unlock进行解锁。C++标准库为互斥量提供了一个模板类`std::look_guard`。大致上的用例如下

```c++
#include<mutex>
#include<list>
#include<algorithm>
std::list<int> ls;
std::mutex ls_mutex
    
 void add_function(int val){
    std::look_guard<std::mutex> guard(ls_mutex);
    ls.push_back(val);
}

void contains_function(int val){
    std::look_guard<std::mutex> guard(ls_mutex);
    return xxxxx
}
```

look_guard 使我们之前提到的RAII语法。它的作用是在当前作用域上上锁，然后再退出当前作用域之后，就会解锁。这种避免直接调用lock和unlock的方式可以避免让我们在每个函数的出口都要unlock

在C++17中，有一个加强版的数据保护机制，`std::scoped_lock`

一般情况下，我们需要将互斥量和数据放在同一类中，这样就能让他们关联在一起，并且进行数据保护。当然，互斥量并不是总能起到作用的，例如说上面的contains函数中，我们返回的是被保护的数据的引用或者指针时，这个保护就失效了。具有访问能力的指针或者引用可以访问并修改被保护的数据，并且不会被互斥锁限制

举个栗子

```c++
class data
{
    int a;
   public:
    void dosth();
};

class protecter
{
 private:
    data _data;
    std::mutex m;
 public:
    template<typename Function>
    void process(Function func)
    {
        std::lock_guard<std::mutex> l(m);
        func(data);
    }
}
data test;
void func(data& prodata)
{
    test = &prodata;
}
protecter a
void main(){
    a.process(func);
    test->dosth();
}
```

从上述的代码中，我们可以发现，我们被保护的数据就这么通过传递引用被"偷"了出来。另外除了这种互斥量使用错误，还有一个问题就是，就算使用了互斥量，依旧存在的条件竞争

使用了互斥量不代表不需要担忧竞争条件，假设我们需要对一个双链表进行一个删除操作，那么我们应该防止对三个节点（被删除的节点，被删除节点前的节点，被删除节点后的节点）进行访问。如果只对指向某一个节点的指针进行防护，那么就和没有使用互斥量一样。整个数据结构和删除操作都需要保护。这里就要引申出一个新的内容，即接口之间导致的条件竞争了。

##### 接口间的条件竞争

我们先假设构建了一个类似于stack的数据结构，除了构造函数和swap函数外，我们可能需要提供五个函数，分别是push,pop,top,empty,size这几个函数。在单线程的情况下，这些个函数都是安全的。但是在多线程的情况下，结果则不一定是可靠的。

```c++
if(!s.empty()){
    int const value = s.top();
    s.pop();
    dosth(value)
}
```

上图中的函数咋一看是非常安全的，先判断了非空，再拿到栈顶端的数据，然后pop出数据，接下来做某些操作。但是对共享的对象来说，如果我在empty和top之前，使用了pop并删除掉最后一个对象，再对栈进行top操作，肯定是错误的，同时empty判断出来的结果，也就是不可靠的了。其中一个解决问题的方式就是使用同一互斥量来保护top和pop函数，不过有可能由于异常导致出现问题。

还是上面的例子，同一互斥量来保护top和pop函数的方向去考虑，我们只要将pop函数是需要返回顶部值并且将这个数据从栈中删除，这个时候我们只需要一个互斥量就可以解决上面的top和pop之间的问题。但是这种方式会引发额外的问题， 我们假设有一个vector的stack，`stack<vector>`，，当系统负荷比较大或者说vector中有大量元素的情况下，我们拷贝一个vector的操作就有可能失败。就有出现拷贝失败，但是数据的确被移除了的情况。因此`std::stack`的设计者就将top和pop函数拆分开了。然而这又回到之前的问题，那就是条件竞争。

但是将pop函数设计为返回顶部值并且将这个数据从栈中删除这个比较多线程友好的设计避开上述的问题，还是有一些其他方法的。一般来说有如下几个方法

**一、将变量的引用作为参数，传入pop函数中**

``` c++
std::vector<int>result;
stack.pop(result);
```

在大多数情况下这种方式还行，但是缺点也同样明显，需要构造出一个栈中的类型的实例用于接收数据。对于某一些类型，这是不现实的，因为临时构造第一个实例，开销很大。对于其他的类型，也不总能行得通。因为构造函数需要一些参数在这个阶段的代码不一定可用。最后就是需要可复制的存储类型，这是一个很大的限制，因为就算支持移动构造，甚至是拷贝构造，很多用户自定义的类型都不一定支持赋值操作

**二、无异常抛出的拷贝构造函数或者是移动构造函数**

 对于上述出现的可能出现异常的问题，我们只需要让拷贝构造函数不要抛出异常或者使用不会抛出异常的移动构造函数就可以了。这样也是线程安全的，因为不会抛出异常。所以是线程安全的。这个方式也有问题，在用户自定义的类型中，会有一些没法储存在这种类型栈中的问题（因为会抛出异常）。

**三、返回指向弹出值的指针**

返回一个指向弹出元素的指针，而不是直接返回值，指针的优势是自由拷贝，不会产生异常。缺点就是返回指针需要对对象的内存分配进行管理。对简单数据类型的内存管理的开销要比直接返回值要大。使用`std::shared_ptr`是一个比较好的选择，不仅能够避免内存泄漏，标准库也能完全控制内存分配方案，也就是说不需要new和delete，这个优化很有必要因为堆栈中的每个对象，都需要new来进行独立的内存分配。

**四、将一三组合或者将一二组合**

由于我们的代码需要一定的灵活性，因此还是需要多一些选择方式的。

那么对于上面说到的五个栈的函数，我们只需要组合上面三种方式的两种，然后将top和pop函数结合起来，就可以做到了。

锁的粒度过大同样会存在着一些问题，假设一个全局互斥量如果要保护全部共享数据，当系统中存在大量共享数据是，会抵消掉并发带来的性能优势。当粒度过细时，需要增加覆盖数据的粒度时，我们只需要锁住一个互斥量就可以了。但是这种方案也是有问题的。当一个操作要操作两个或者两个以上的互斥量时，一个新的问题出现了，那就是死锁

##### 死锁

死锁描绘的是每个线程都在等待另外的线程释放已锁的互斥量，然后导致两个线程都什么都做不了。避免死锁的一般方法就是让两个互斥量以相同的顺序上锁，例如AB互斥量上锁的顺序永远都是AB。但是具体实现的时候，并不能那么简单的就能如愿。例如说我们一个上锁函数，通过改变传参方式就又死锁了。但是在C++中，我们只需要使用`std::lock`函数就能一次性锁上多个互斥量，且不 会产生死锁

```c++
void swap(X& lhs,X& rhs)
{
    if(&lhs==&rhs)
        return;
    std::lock(lhs.m,rhs.m);
    std::lock_guard<std::mutex> lock_a(lhs.m,std::adopt_lock);
    std::lock_guard<std::mutex> lock_b(rhs.m,std::adopt_lock);
    swap(lhs.some_detail,rhs.some_detail);
}
```

lock函数，用于对多个互斥量进行加锁。首先我们队两个互斥量进行加锁。为了防止难以预料的行为，所以我们这里需要先比较lhs和rhs是否为同一个对象。接下来我们使用lock_gurad对两个互斥量进行管理。我们利用`std::adopt_lock`参数除了表示`std::lock_guard`对象可获取锁意外，还会将锁交给`std::lock_guard`对象管理，而不需要`std::lock_guard`对象再去构建新的锁

这样，我们利用lock函数对两个互斥量进行加锁，然后使用lock_guard函数进行锁的管理，这样就可以避免自己手动加锁导致的问题。

这里需要注意的一点就是，使用lock函数去加锁时，可能会抛出异常。例如当我们成功对一个互斥量上锁后，处理第二个锁出现了异常抛出，第一个锁也会被自动释放，lock函数要么两个一起锁上了，要么就一个都不锁。

c++17中，支持了这种情况，使用了`std::scoped_lock<>`这一新的RAII类型模板类型，和`std::lock_guard`的功能等价，能够以不定数量的互斥量类型作为模板参数，以及对应的互斥量（数量和类型）作为构造参数，互斥量支持构造及上锁，和`std::lock`的用法相同，解锁阶段是在析构中运行，因此swap操作可以重写如下

```
void swap(X& lhs,X& rhs){
 if(&lhs==&rhs)
 	return;
 std::scoped_lock guard<std::mutex,std::mutex>guard(lhs.m,rhs.m);
 swap(lhs.some_detail,rhs.some_detail);
}
```

这个函数的好处在于，可以替换所有的lock函数，减少潜在的错误。

虽然上面提到的两个函数可以再获取两个以上的锁的情况下避免思索，但是他没办法帮助我们获取其中一个锁。因此这里避免死锁需要依靠经验。下面将会介绍一些避免死锁的规则

##### 避免死锁的规则

**一、避免嵌套锁**

当一个线程已经获取一个锁时，尽量不要获取第二个，因为每个线程只持有一个锁，锁上就不会产生死锁。就算要获取多个，也应该使用`std::lock`来做这件事情。

**二、避免在持有锁时调用用户提供的代码**

**三、适用固定顺序获取锁**

如果实在没办法，你需要自己获取多个锁，那么你应该以固定的顺序获取他们。以之前说的链表为例子，链表中的每一项都有一个互斥量保护，为了访问链表，线程必须获取他们感兴趣节点上的互斥锁，这时候我们就必须限制每一个线程遍历获取互斥锁的顺序，并且在获取了下一个互斥锁的情况下，将上一个互斥锁释放。并且顺序一定是a->b->c,不能是其他顺序

**四、使用锁的层次结构**

看到这个规则的时候，我的第一反应就是，深度缓冲。我们会定义所得分层，当代码试图给一个互斥量上锁，但是该锁已经被低层持有时，就不被允许。同时，低层锁也不允许获取高层锁

举个例子，我们有三个层级的mutex abc分别是1 2 3的层级，假设我们调用一个函数，它的作用是加锁3，然后调用另外一个获取2的函数并加锁2，这没有问题，但是当我们调用一个函数加锁2，并调用一个获取并加锁3的函数，则不行，因为锁已经被低层持有。

具体的实现中，最重要的就是为当前线存储之前的层级值

**五、超越锁的延伸扩展**

使用`std::unique_lock`

##### std::unique_lock

相比起`std::lock_guard`，`std::unique_lock`要占用更多空间，但是也更灵活，它是一个使用更为自由的不变量。我们可以使用`std::adopt_lock`参数来对互斥量进行管理，同时也能传入`std::defer_lock`来作为第二个参数传递进去，表明互斥量应该解锁。例如

```c++
void swap(X& lhs,X& rhs)
{
    if(&lhs==&rhs)
        return;
    std::unique_lock<std::mutex> lock_a(lhs.m,std::defer_lock);
    std::unique_lock<std::mutex> lock_b(rhs.m,std::defer_lock);
    //留下未上锁的互斥量
    std::lock(lock_a,lock_b)//上锁;
    swap(lhs.some_detail,rhs.some_detail);
}
```

使用unique_lock还可以通过move操作来转移互斥量的所有权

此外，使用锁的时候，我们也应该注意，锁住合适粒度的数据，在不需要锁的时候，应该将其解锁，举个例子

```c++
friend bool operator==(Y const&lhs,Y const& rhs)
{
    if(&lhs==&rhs)
        return true;
    int const lhs_value = lhs.get_detail();
    int const ths_value = lhs.get_detail();
    return lhs_value==ths_value;
}

int get_detail() const {
    std::lock_guard<std::mutex> lock_a(m);
    return some_detail;
}
```

#### 3.3 保护共享数据的其他方式

##### 保护共享数据的初始化过程

查看如下代码

```c++
void foo()
{
    if(!resource_ptr)
    {
        resource_ptr.reset(new some_resource);
    }
    resource_ptr->dosth();
}
```

这显然是多线程不友好的，那么应该怎么做呢，先看如下的方法

```c++
void foo()
{
    std::unique_lock<std::mutex>lk( mutex);
    if(!xxx)
    {
        
    }
    lk.unlock;
    xxx
}
```

这么做的话，所有线程就需要在判断是否存在之前等待

还有另外一种十分声名狼藉的双重检查模式

```c++
void doubecheck(){
    if(!ptr)
    {
         std::lock_guard<std::mutex>lk( mutex);
        if(!ptr){
            xxxx
        }
    }
    dosth()
}
```

这个模式下，我们先判断为null，然后获取锁，然后在判断为null，确定没被初始化，但是这里有潜藏的条件竞争。

因为我们有一个最好的方法，那就是**std::once_flag 和std::call_once**

```c++
std::once_flag flag;
void init()
{
    std::call_once(onceflag,initfunc);
    dosth();
}
```

再介绍一种替换方案

```c++
class mcls;
mycls& get_class()
{
    static mycls instance;
    return instance;
}
```



##### 保护不常更新的数据结构

c++17 提供了`std::shared_mutex`和`std::shared_timed_mutex`,C++14值提供了后者。如果是c++11之前的话，可以使用BOOST库中实现的。

##### 嵌套锁

C++标准库中提供了`std::recursive_mutex`类，可以对统一线程单个实例上获取多个锁。

### 四、同步并发操作

并发的行为，就是所谓的自己做自己的，但是当我们有共享数据时，可能不仅仅想要保护数据，还想要对单独的线程进行同步。例如，在第一个线程完成前，可能需要等待另外一个线程执行完成。C++标准库提供了一些工具可以用于同步操作，形式上变现为条件变量和期望值。在并发技术规范中，为期望值添加了更多的操作，并且与新的同步工具锁存器和栅栏机制一起使用

#### 4.1 等待一个事件或其他条件

当线程等待另外一个线程完成任务，可以做的事情很多，例如说可以持续的检查共享数据标识（如用于做保护工作的互斥量），直到另外一个线程完成工作时对这个标志进行重设。但是由于一直在检查，所以是浪费的。或者可以使用`std::this_thread::sleep_for()`进行休眠，但是有一个问题就是无法确认合适的休眠时间，太长或太短都有问题。第三个也是最优先的选择就是使用标准库提供的工具进行事件的等待。这里书中写的方式是条件变量，条件变量可以做到在线程完成时向等待线程广播条件达成的信息

c++标准库对条件变量有两套实现，分别是`std::condition_variable`和`std::condition_variable_any`。这两个实现都包括在`<condition_variable>`的头文件声明中。两者都需要与一个互斥量才能一起工作，前者限定为`std::mutex`，后者则是可以和任何满足最低标准的互斥量一起工作。后者的适用性更强，因此开销也会更大。

下面的例子展现了一个条件变量的用法

```c++
#include<mutex>
#include<condition_variable>
std::mutex mut;
std::queue<data_chunk> data_queue;
std::condition_variable data_cond;

void data_preparation_thread()
{
    while(more_data_to_prepare())
    {
        data_chunk const data = prepare_data();
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(data);//1
        data_cond.notify_one();//2
    }
}

void data_processing_thread()
{
    while(true){
        std::unique_lock<std::mutex> lk(mut);//3
        data_cond.wait(lk,[]{return !data_queue.empty();})//4
        data_chunk data = data_queue.front();
        data_queue.pop();
        lk.unlock();
        process(data);
        if(xxx){
            break;
        }
    }
}
```

此处条件变量的用法就是，**在1处处理完数据后，通知等待的线程，这个等待的线程会先对互斥量上锁，然后这个锁会被传入到条件变量的wait函数中，wait函数会对传入的互斥量和条件函数进行检查，当条件不符合时，wait函数会解锁这个互斥量，然后将线程置于等待状态，当被notifyone函数唤醒之后，它会重新获取互斥锁，然后在条件满足是，它会继续持有锁并且进行下一步的操作。这就是为什么这里会使用unique_lock，因为它更灵活。可以再等待期间解锁互斥量。并在之后再次上锁。**

这里的wait是一种优化的实现，不理想的方式可能就是

```c++
while(!pred()){
    lk.unlock();
    lk.lock();
}
```

顺带一提，上述代码中虽然使用了whiletrue的循环，但是实际上这个wait并不需要循环，只需要有人notify一次，并且条件通过后，就会顺利执行下去了，例如说我调用pop函数，wait当数据不为空时

#### 4.2 使用期望值等待一次性事件

c++标准库将一次性的等待事件称为期望值，当线程需要等待特定的一次性事件时，某种程度上来说就需要知道这个事件在未来的期望结果。之后，这个线程会周期性的（较短周期）的检查或者等待事件是否触发，检查期间也会执行其他任务，直到对应的任务触发，之后期望值的状态就会变成就绪状态，一个期望值可能是数据相关的，并且当事件发生时，这个期望值就不能被重置

看到这里的时候，突然一股熟悉的味道就涌上心头，这不是promise的味道吗。

C++标准库中有两种期望值，使用两种类型模板实现，声明在`<future>`头文件中。分为两种`unique futures - std::future<>`和`shared future std::shared_future<>`。`std::future`类型的实例只能和一个指定事件相关联，而另外那个就可以关联多个事件。在后者的实现中，所有实例会在同时变为就绪状态，并且他们可以访问与事件相关的所有数据。当多个线程需要访问一个独立期望值对象时，必须使用互斥量或者同步机制对访问进行保护，

使用期望值，可以解决`std::thread`执行任务不能有返回值的问题

##### 后台任务的返回值

举个常见的例子，我需要一个长计算，但是不着急他的结果，我们使用`thread`是没法获取结果的，于是，我们可以使用`future`中的`std::async`函数模板来等待并获取结果。这个对象会返回一个`std::future`对象，我们只需要调用这个对象的get函数，就可以阻塞线程直到ready态为止。如下

```c++
#include<future>
#include<iostream>
int func();
int main()
{
    std::future<int> answer = std::async(func);
    //dosth
    std::cout<<answer.get();
}
```

和thread函数一样，async模板允许通过添加额外的调用参数，来传递额外的参数，当第一个函数是一个指向成员函数的指针，第二个参数提供具体对象（通过指针，std::ref包装等），就可以调用成员函数。否则第二个参数起就是函数的参数。当参数为右值时，拷贝操作将使用move操作来转移原始数据

```c++
struct test
{
    void foo(int a,int b){}
    std::string bar(int a){}
    /* data */
};
test t;
auto f1 = std::async(&test::foo,&t,1,2);//调用p.foo(),p是指向t的指针
auto f2 = std::async(&test::bar,t,1);//调用tmpt.foo(),temt是t的拷贝副本
struct Y
{
    double operator()(double);
};

Y y;
auto f3 = std::async(Y(),3.14);//调用tempy(3.14)，通过移动构造函数构造
auto f4 = std::async(std::ref(y),2.718);//调用y(3.14)

test baz(test& a){};
auto t = std::async(baz,std::ref(t));//调用baz(x)

auto f5 = std::async(move_only());//调用temp()，temp通过std::move(move_only)得到
```

具体的传参操作如上。大多数情况下，期望值是否等待取决于async是否启动一个线程，或者是否有任务正在进行同步。我们可以通过一个额外参数来表明函数调用被延时到何时进行。如下所示。

```c++
auto f6=std::async(std::launch::async,Y(),1.2); //在新线程上执行
std::future<test> f7 = std::async(std::launch::deferred,baz,std::ref(t)); //在wait()或者get()函数上执行
auto f8 = std::async(std::launch::deferred|std::launch::async,baz,std::ref(t)); //根据实现选择执行方式
auto f9 = std::async(baz,std::ref(t));
f7.wait();
```

上述的代码中，`std::launch::async`的作用是表示函数必须在独立的线程上执行，而`std::launch::deferred`的作用是表明函数调用延迟到wait或者get函数调用时才执行，**另外，析构函数也会导致立刻执行**。`std::launch::deferred|std::launch::async`这个则表示可以选择任意的方式执行。

`std::async`可以更容易的让算法分割到各个不同的任务中。当然，除了用`async`，我们还可以将任务包装进`std::packaged_task<>`实例中，或通过编写代码的方式，使用`std::promise<>`类型模板显式设置值。相对于promise，使用package_task 有更高层的抽象，我们就从它开始说起

##### 任务与期望值关联

这里我们需要聊得是`std::package_task`函数，这是一个仿函数，它本身和线程没啥关联，我们将其和期望值关联，就能和多线程扯上关系了

这个函数的模板是传入一个函数签名，类似于函数指针，我们调用这个package_task对象就相当于是调用了这个函数。同时我们可以通过这个函数获取期望值，这样就可以做到阻塞线程进行等待，如下

```c++
auto task = [](int i){return 1;};
std::packaged_task<int(int)> package{task};
std::future<int> f = package.get_future();
```

上面的函数中，调用package就相当于执行了task，我们用package创建线程，并通过期望值来获取运行的结果，大概就是长这样。

```c++
auto task = [](int i){return 1;};
std::packaged_task<int(int)> package{task};
std::future<int> f = package.get_future();

std::thread t{std::move(package),5};
f.get();//阻塞到t结束
t.join();
```

不过需要注意的是，调用get函数之前，一定要执行package_task，不然就会一直被阻塞

##### std::promise

看到这个内容的时候，不由得感慨程序设计上的东西还是很多相同的。

```c++

auto task = [](int i){return 1;};
std::promise<int> p;
std::thread tt{task,p.get_future()};
std::this_thread::sleep_for(std::chrono::seconds(5));
p.set_value(5);
tt.join();
```

查看上面的用法，通过future值和promise的结合使用，我们可以将线程阻塞，知道我们调用了set_value函数的时候，期望值的状态变成了就绪，于是我们就变成了随时随地的给新线程传值。

是不是很熟悉，这就是机制上和js中的相似，但是这里是多线程。

思考 std::async、std::packaged_task 和 std::promise 之间的关系。总体来说，std::async 接口最简单，做的事情最多，抽象程度最高；std::packaged_task，抽象程度次之，需要额外的操作但却比较灵活；std::promise 功能最为单一，是三者中抽象程度最低的。

##### 将异常存储在期望值中

 试想一个情景，有一个async函数中调用出错，抛出一个异常，会怎么办，事实上，这个异常会被存储在期望值中。之后期望值的状态被设置为就绪。然后调用get函数会获得一个异常对象。和其他语言的async抛出异常还是有点区别的。而如果是上面说的package_task任务包的话，那么也是会这样的。promise的话，就会有所区别。

```c++
std::promise<double> some_promise;
try
{
 some_promise.set_value(cakcykate_value());
}
catch()
{
	some_promise.set_exception(std::current_exception());
}
```

这里使用了`std::current_exception()`来检索抛出的异常，可用`std::copy_exception()`作为一个替代方案，它会存储一个新的异常而不抛出

```c++
some_promise.set_exception(std::copy_exception(std::logic_error("foo")));
```

比使用try catch更加清晰，当异常类型已知，就应该优先被使用，可以提高优化

另外一种方式就是在没有调用promise的任何一个设置函数的时候，在析构函数中调用存储一个错误异常。

当多个线程需要等待相同事件的结果时，就需要用`std::shared_future`了

##### 多个线程的等待

当多个线程需要等待相同事件的结果时，就需要用`std::shared_future`了，`shared_future`在多个线程需要共享这个量的时候，可以通过拷贝，让所有期望值都指向这个值。这样就可以做到大家都安全等待了

#### 4.3 限定等待时间

我们知道，阻塞会是一段不确定的时间，但是有的时候不一定会能等到回复。例如说请求断网了，你永远也无法收到回复。

我们有两种方式，一种是`wait_for`,一种是`wait_until`，前者是指定一段时间，后者是制定一个时间点。

这部分内容是时间库相关，这里先不提

#### 

### 五、C++内存模型和原子类型操作

 #### 5.1 内存模型基础

内存模型一方面是基本结构，和内存布局有关，另一方面就是并发，并发的基本结构很重要，特别是低层原子操作。C++所有对象都和内存位置有关

。

C++中的所有数据都是由对象构成，对象是C++数据构建块的声明。C++标准定义对象为“存储区域”，但是对象还是可以将自己的特性赋予其他对象，例如类型和生命周期。但是不论对象是怎么样的类型，对象都会存储在一个或者多个内存位置上。每个内存位置不是标量类型的对象，就是标量类型的子对象。基本类型都有确定的内存为止。

所有的东西都在内存中，当两个线程访问内存时，当他们访问同一个位置，并且要修改数据，就应该存在一定的访问顺序。除了互斥量，我们还应该使用原子操作来决定两个线程的访问顺序。当然，原子操作会本身只是将行为变成定义的行为，并不代表原子操作就不存在竞争
