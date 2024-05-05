### 类型推导

template和auto都有类型推导，它们的规则略有不同。如template

```c++
template<typename T>
void f(ParamType param);
```

传递引用/指针时，例如

```c++
template<typename T>
void f(T& param);

int a;   //T int param int&
int& b;  //T int param const int&
const int& c； //T int param const int&
```

推导结果如上。如果param类型改成`const T&,T*`也是这种情况

而使用右值引用时,规则则发生了改变

- 如果`expr`是左值，`T`和`ParamType`都会被推导为左值引用。这非常不寻常，第一，这是模板类型推导中唯一一种`T`被推导为引用的情况。第二，虽然`ParamType`被声明为右值引用类型，但是最后推导的结果是左值引用。
- 如果`expr`是右值，就使用正常的（也就是**情景一**）推导规则

```C++ 
template<typename T>
void f(T&& param);

int a;   //T int&, param int&
int& b;  //T const int& ,param const int&
const int& c; //T const int&, param const int&
27;//T int&, param int&&
```

基本也是和上面一样，但是当你传入的是右值的时候，则推导参数类型为 `&&`

按值传递时，传入类型的`const`和引用会被忽略

```c++
template<typename T>
void f(T param);

int a;   //T int, param int
int& b;  //T int ,param int
const int& c; //T int, param int
```

- 和之前一样，如果`expr`的类型是一个引用，忽略这个引用部分

- 如果忽略`expr`的引用性（reference-ness）之后，`expr`是一个`const`，那就再忽略`const`。如果它是`volatile`，也忽略`volatile`

传指针时则是传指针的拷贝



传数组作为实参时，如果是按值传递，则会退化为指针，如果按引用传递，则是数组自身

```c++
template<typename T>
void f(T param);

void f2(T& param);

int t[32] // f int*, f2 int[32]

```

函数则是函数指针和指向函数的引用



auto则类似于函数模板中的T，变量名的类型则类似于param的类型。但是有一个区别，那就是对于初始化表的区别，template没法识别初始化表

总结

- 在模板类型推导时，有引用的实参会被视为无引用，他们的引用会被忽略
- 对于通用引用的推导，左值实参会被特殊对待
- 对于传值类型推导，`const`和/或`volatile`实参会被认为是non-`const`的和non-`volatile`的
- 在模板类型推导时，数组名或者函数名实参会退化为指针，除非它们被用于初始化引用

- `auto`类型推导通常和模板类型推导相同，但是`auto`类型推导假定花括号初始化代表`std::initializer_list`，而模板类型推导不这样做
- 在C++14中`auto`允许出现在函数返回值或者*lambda*函数形参中，但是它的工作机制是模板类型推导那一套方案，而不是`auto`类型推导
- `decltype`总是不加修改的产生变量或者表达式的类型。
- 对于`T`类型的不是单纯的变量名的左值表达式，`decltype`总是产出`T`的引用即`T&`。
- `auto`变量必须初始化，通常它可以避免一些移植性和效率性的问题，也使得重构更方便，但是可能出现问题
- 通过显示类型转换可以强制auto的类型（但是意义呢）





### 现代C++

总体来讲比较常规

构造函数相关

- 花括号初始化是最广泛使用的初始化语法，它防止变窄转换，并且对于C++最令人头疼的解析有天生的免疫性
- 在构造函数重载决议中，编译器会尽最大努力将括号初始化与`std::initializer_list`参数匹配，即便其他构造函数看起来是更好的选择
- 对于数值类型的`std::vector`来说使用花括号初始化和圆括号初始化会造成巨大的不同
- 在模板类选择使用圆括号初始化或使用花括号初始化创建对象是一个挑战。

Null和nullptr模板相关

- 优先考虑`nullptr`而非`0`和`NULL`
- 避免重载指针和整型

使用using，别用typedef了

- `typedef`不支持模板化，但是别名声明支持。
- 别名模板避免了使用“`::type`”后缀，而且在模板中使用`typedef`还需要在前面加上`typename`
- C++14提供了C++11所有*type traits*转换的别名声明版

enum Class，但是这样要写更多的代码

- 限域`enum`的枚举名仅在`enum`内可见。要转换为其它类型只能使用*cast*。
- 非限域/限域`enum`都支持底层类型说明语法，限域`enum`底层类型默认是`int`。非限域`enum`没有默认底层类型。

其他的一些

- 使用override声明重载
- 迭代器使用const
- `constexpr`对象是`const`，它被在编译期可知的值初始化





### 智能指针

这章没有太多新东西，都是一些常规可以理解的

- unique_ptr,shared_ptr,weak_ptr各有各的适用场景
- shared_ptr更大，是unique_ptr的两倍

- 默认资源销毁是通过`delete`，但是也支持自定义删除器。删除器的类型是什么对于`std::shared_ptr`的类型没有影响。
- 和直接使用`new`相比，`make`函数消除了代码重复，提高了异常安全性。对于`std::make_shared`和`std::allocate_shared`，生成的代码更小更快。
- 不适合使用`make`函数的情况包括需要指定自定义删除器和希望用花括号初始化。



### 左右值引用和move

通用引用(auto&&)可以指向各种对象主要有一个现象就是引用折叠了，对于右值就是右值引用，穿左值就是左值引用。

std::move其实主要是做了一个cast，将值转成右值。

- 如果一个函数模板形参的类型为`T&&`，并且`T`需要被推导得知，或者如果一个对象被声明为`auto&&`，这个形参或者对象就是一个通用引用。
- 如果类型声明的形式不是标准的`type&&`，或者如果类型推导没有发生，那么`type&&`代表一个右值引用。
- 通用引用，如果它被右值初始化，就会对应地成为右值引用；如果它被左值初始化，就会成为左值引用。

- `std::move`执行到右值的无条件的转换，但就自身而言，它不移动任何东西。
- `std::forward`只有当它的参数被绑定到一个右值时，才将参数转换为右值。



### Lambda表达式

Lambda表达式这章总体看下来，主要有几点，主要是多用初始化捕获

- 默认的按引用捕获可能会导致悬空引用。
- 使用初始化捕获（看起来就是要先初始化一波再move到lambda里面去）

### 其他

- 对于可拷贝，移动开销低，而且无条件被拷贝的形参，按值传递效率基本与按引用传递效率一致，而且易于实现，还生成更少的目标代码。
- 通过构造拷贝形参可能比通过赋值拷贝形参开销大的多。
- 使用emplace_back不用pushback->这条之前也有见过，不过看实现目前应该是相似的了
