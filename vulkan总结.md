vulkan是一个新时代的图形库，它的主要优点在于完全跨平台的优势，同时具有更好的性能。当然，跨平台带来的就是你需要完全从头开始设置程序，包括各种缓冲区设置，纹理图像等对象的内存管理，还需要指定命令队列等，按照官方的说法

> 这里要传达的信息是 Vulkan 并不适合所有人。它针对的是热衷于高性能计算机图形，并愿意投入一些工作的程序员。如果你对游戏开发更感兴趣，而不是计算机图形，那么你不妨坚持使用OpenGL或Direct3D，它们会不会很快被弃用以支持 Vulkan。另一种选择是使用[Unreal Engine](https://en.wikipedia.org/wiki/Unreal_Engine#Unreal_Engine_4) 或[Unity](https://en.wikipedia.org/wiki/Unity_(game_engine))等[引擎](https://en.wikipedia.org/wiki/Unreal_Engine#Unreal_Engine_4)，它们将能够使用 Vulkan，同时向您公开更高级别的 API

从另外一个角度来说，vulkan是一个更加复杂并且更加原生的东西，从我个人的理解来说，它更加贴近于引擎开发，对于顶层开发或者是shader开发来说，其实用vulkan和用opengl，DX，其实并没有区别。

### 画一个三角形

注：做这个的时候，一共写了大约900多行代码，才能够勉强画出一个一个三角形

#### 基本代码

画这个三角形的时候，我们需要有一个基础的代码架构

```c++
class HelloTriangleApplication {
public:
    void run() {
        initVulkan();
        mainLoop();
        cleanup();
    }
private:
    void initVulkan() {

    }
    void mainLoop() {
    }
    void cleanup() {
    }
};

int main() {
    HelloTriangleApplication app;
    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
```

这个基础架构中，最长并且最耗时的就是`initVulkan`了，这部分函数的主要目的就是为了正确的初始化好vulkan的各种配置。在官方教程中，我们使用的是glfw来进行窗体的创建

```c++
window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
```

```c++
void initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}
```

```c++
void cleanup() {
    glfwDestroyWindow(window);

    glfwTerminate();
}
```

当然在mainloop中，我们需要让窗口一直不停的运行

#### 初始化vulkan对象

接下来，我们就应该开始创建vulkan对象了，每当我们需要使用vkcreate相关的函数来进行某些创建的时候，我们都需要创建一个createInfo来承载它的属性。

```c++
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
```

Vulkan 中的很多信息是通过结构体而不是函数参数传递的，我们必须再填充一个结构体来为创建实例提供足够的信息。下一个结构体不是可选的，它告诉 Vulkan 驱动程序我们要使用哪些全局扩展和验证层。全局在这里意味着它们适用于整个程序而不是特定的设备，这将在接下来的几章中变得清晰。

```c++
VkInstanceCreateInfo createInfo{};
createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
createInfo.pApplicationInfo = &appInfo;
```

接下来，在配置好一切之后，我们会经常看到的一种类型的函数，vkcreate函数

```c++
 vkCreateInstance(&createInfo, nullptr, &instance);
```

这个函数的作用是生成一个vulkan实例，这种create函数，通常情况下会返回一个vk_success宏定义。

当然在创建了这个对象之后，我们需要将他打扫干净

```c++
vkDestroyInstance(instance, nullptr);
```

#### 验证层

创建完实例之后，教程的下一步就是验证层。验证层的主要主要是DEBUG使用。官方的说法如下

> Vulkan API 是围绕最小驱动程序开销的想法设计的，该目标的表现之一是默认情况下 API 中的错误检查非常有限。即使是将枚举设置为不正确的值或将空指针传递给所需参数这样简单的错误，通常也不会被明确处理，只会导致崩溃或未定义的行为。因为 Vulkan 要求您对所做的一切都非常明确，所以很容易犯许多小错误，例如使用新的 GPU 功能而忘记在逻辑设备创建时请求它。
>
> 但是，这并不意味着不能将这些检查添加到 API 中。Vulkan 为此引入了一个优雅的系统，称为*验证层*。验证层是可选组件，可与 Vulkan 函数调用挂钩以应用其他操作。验证层中的常见操作有：
>
> - 根据规范检查参数值以检测误用
> - 跟踪对象的创建和销毁以发现资源泄漏
> - 通过跟踪调用源自的线程来检查线程安全
> - 将每个调用及其参数记录到标准输出
> - 跟踪 Vulkan 调用以进行分析和重放
>
> 以下是诊断验证层中函数实现的示例：

但是实际上从我的实际体验来看，它的作用主要是告诉了你有错误，但是这些错误的解决并不是那么明确的，很多时候，例如说报了一个framebuffersize大小不匹配的错误，而实际的原因可能是你重新绘制图像的时候某一个参数填错了。

在教程中，我们使用的验证层也来自于一个sdk

> Vulkan 没有内置任何验证层，但 LunarG Vulkan SDK 提供了一组很好的层来检查常见错误。它们也是完全[开源的](https://github.com/KhronosGroup/Vulkan-ValidationLayers)，因此您可以检查它们检查和贡献的错误类型。使用验证层是避免您的应用程序因意外依赖未定义行为而在不同驱动程序上中断的最佳方法。
>
> 验证层只有在已安装到系统上时才能使用。例如，LunarG 验证层仅在安装了 Vulkan SDK 的 PC 上可用。
>
> Vulkan 中以前有两种不同类型的验证层：实例和设备特定。这个想法是实例层只会检查与全局 Vulkan 对象（如实例）相关的调用，而设备特定层只会检查与特定 GPU 相关的调用。设备特定层现已弃用，这意味着实例验证层适用于所有 Vulkan 调用。规范文档仍然建议您在设备级别启用验证层以及兼容性，这是某些实现所要求的。我们将只需指定相同的层在逻辑设备水平的情况下，我们会看到[后面](https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Logical_device_and_queues)

关于验证层，这里暂时不多做总结

#### 物理设备和队列系列

 vulkan的核心之一，就是要确定物理设备，所谓物理设备就是GPU。在选取设备之前，我们要先确定设备的可用性。当我们确定了设备后，我们就要建立vulkan的核心，队列。vulkan中的几乎所有操作，从绘制到上传纹理，都需要将命令提交到队列。有来自不同*队列族的*不同类型的队列， 每个*队列族*只允许命令的一个子集。例如，可能有一个队列系列只允许处理计算命令，或者一个只允许与内存传输相关的命令。我们需要检查设备支持哪些队列系列，哪些队列系列支持我们想要使用的命令。





