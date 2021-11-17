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

### 初始化vulkan对象

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

在选择设备的时候，我们要先确认设备的可用性。而在vulkan中，我们可以通过一系列函数来获取设备的属性。举个例子

```c++
 	VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
           deviceFeatures.geometryShader;
```

在上面的例子中，我们通过getdeviceproperties和getdeivcefreature分别获取了两个属性，通过deviceProperties获取到devicetype，我们可以选择获取到我们最需要的类型，而deviceFeature这个属性，则可以获取到当前硬件所需要的feature。

而我们的device设备要如何获取呢，我们的例子中是这样的

``` c++
uint32_t deviceCount = 0;
vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
std::vector<VkPhysicalDevice> devices(deviceCount);
vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
```

选择好了一个比较合适的设备之后，我们就应该开始处理队列族

我们需要检查设备支持哪些队列系列，哪些队列系列支持我们想要使用的命令。为此，我们将添加一个新函数`findQueueFamilies`来查找我们需要的所有队列系列。

在这次的教程中，我们就只需要寻找一个支持图形命令的队列。而这些队列，我们都是通过一个值来确定的，这些值的范围是0到uin32_t的理论上任何值。因此，教程中采用了一个C++17的新的数据结构来区分值的存在与否的情况

```c++
#include <optional>
std::optional<uint32_t> graphicsFamily;
std::cout << std::boolalpha << graphicsFamily.has_value() << std::endl; //false
graphicsFamily = 0;
std::cout << std::boolalpha << graphicsFamily.has_value() << std::endl; //true
```

获取队列族的方式和上面获取硬件的方式一样

```c++
uint32_t queueFamilyCount = 0;
vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
```

`VkQueueFamilyProperties`结构包含有关队列系列的一些详细信息，包括支持的操作类型以及可以基于该系列创建的队列数量。我们需要找到至少一个支持`VK_QUEUE_GRAPHICS_BIT`,这种类型就是我们需要的图形命令。

```c++
int i = 0;
for (const auto& queueFamily : queueFamilies) {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        indices.graphicsFamily = i;
    }
    i++;
}
```

#### 创建逻辑设备

选择要使用的物理设备后，我们需要设置一个*逻辑设备*来与之交互。逻辑设备创建过程与实例创建过程类似，描述了我们想要使用的特性。我们还需要指定要创建的队列，因为我们已经查询了哪些队列系列可用。如果您有不同的需求，您甚至可以从同一物理设备创建多个逻辑设备。

要创建一个逻辑设备，我们需要指定相关的队列和队列数量，因此我们要先填充一个`vkDeviceQueueCreateInfo`

```c++
QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

VkDeviceQueueCreateInfo queueCreateInfo{};
queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();//上面的函数我们限定获取了图形队列
queueCreateInfo.queueCount = 1; //队列数量为1
float queuePriority = 1.0f;
queueCreateInfo.pQueuePriorities = &queuePriority;//队列优先级为1.0，范围是1.0~0.0
```

创建完这个之后，我们需要指定这个逻辑设备需要使用的设备功能集，这些就是上面说到的`vkGetPhysicalDeviceFeatures`。在画三角形这个部分里面，我们不需要额外的功能，因此我们只需要定义一个空白的结构体

```c++
VkPhysicalDeviceFeatures deviceFeatures{};
```

这两个结构体创建完毕之后，我们就需要填充最主要的`VkDeviceCreateInfo`结构体了

```C++
VkDeviceCreateInfo createInfo{};
createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
createInfo.pQueueCreateInfos = &queueCreateInfo;//指向上面创建的队列信息
createInfo.queueCreateInfoCount = 1;//只创建一个队列
createInfo.pEnabledFeatures = &deviceFeatures;//指向上面的硬件特性集
```

其他的信息和`VkInstanceCreateInfo`结构相似，并且我们需要指定扩展和验证层。不过这些和设备本身有关。

```c++
createInfo.enabledExtensionCount = 0;

if (enableValidationLayers) {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
} else {
    createInfo.enabledLayerCount = 0;
}
```

这里先列出来新写法和兼容性写法

接下来我们就是一如既往的创建和销毁了，这里也不多提

```c++
vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS;
vkDestroyDevice(device, nullptr);
```

这里需要提一嘴的是，由于设备被销毁时，设备队列也会被销毁，所以不需要单独cleanup，当我们需要获取一个队列的句柄的时候，我们可以调用下面的函数

```c++
vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);//0是队列index，graphicQueue是队列句柄
```

#### 窗口

创建完逻辑设备后，就应该开始创建窗口相关的代码了，不过这里的窗口不是我们创建应用程序的窗口，而是我们的代码和窗体交互的窗口。

> 由于 Vulkan 是一个与平台无关的 API，因此它自身无法直接与窗口系统交互。为了在 Vulkan 和窗口系统之间建立连接以将结果呈现到屏幕上，我们需要使用 WSI（窗口系统集成）扩展。在本章中，我们将讨论第一个，即`VK_KHR_surface`. 它公开一个`VkSurfaceKHR`代表抽象类型表面的对象，以向其呈现渲染图像。我们程序中的表面将由我们已经用 GLFW 打开的窗口支持。
>
> 该`VK_KHR_surface`扩展是一个实例级扩展，我们实际上已经启用了它，因为它包含在 `glfwGetRequiredInstanceExtensions`. 该列表还包括我们将在接下来的几章中使用的一些其他 WSI 扩展。
>
> 窗口表面需要在实例创建后立即创建，因为它实际上会影响物理设备的选择。我们推迟这样做的原因是因为窗口表面是渲染目标和演示的更大主题的一部分，对此的解释会使基本设置变得混乱。还应该注意的是，如果您只需要离屏渲染，窗口表面是 Vulkan 中完全可选的组件。Vulkan 允许您做到这一点，而无需像创建不可见窗口（OpenGL 所必需的）之类的技巧。

在vulkan的教程中，我们会使用`VkSurfaceKHR`来做句柄。窗口表面是一个Vulkan对象，我们就需要一个结构体

```c++
VkWin32SurfaceCreateInfoKHR createInfo{};
createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
createInfo.hwnd = glfwGetWin32Window(window);//窗口句柄
createInfo.hinstance = GetModuleHandle(nullptr);//进程
```

为了保证我们的设备可以支持窗口系统，所以我们需要确保设备可以将图像呈现到我们创建的表面上。由于呈现是队列特定的功能，问题实际上是关于找到支持呈现到我们创建的表面的队列系列。

```c++
VkBool32 presentSupport = false;
vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
```

支持绘图命令的队列族和支持演示的队列族实际上可能不重叠。因此，我们必须考虑到通过修改`QueueFamilyIndices`结构：

```c++
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};
```

接下来就是需要确定演示队列，在我们的例子中，我们使用`VkQueue presentQueue`这个变量来保存这个句柄，然后我们需要多个`VkDeviceQueueCreateInfo`结构体来创建来自两个不同的队列family的队列。

```c++
QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

float queuePriority = 1.0f;
for (uint32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
}
```

这样，我们就拥有了两个队列Family。当然，我们也需要修改上面提到的VkDeviceCreateInfo,

```c++
createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
createInfo.pQueueCreateInfos = queueCreateInfos.data();
```

#### SwapChian 交换链

交换链的定义原文如下

>Vulkan does not have the concept of a "default framebuffer", hence it requires an infrastructure that will own the buffers we will render to before we visualize them on the screen. This infrastructure is known as the *swap chain* and must be created explicitly in Vulkan. The swap chain is essentially a queue of images that are waiting to be presented to the screen. Our application will acquire such an image to draw to it, and then return it to the queue. How exactly the queue works and the conditions for presenting an image from the queue depend on how the swap chain is set up, but the general purpose of the swap chain is to synchronize the presentation of images with the refresh rate of the screen.

这部分定义的核心：交换链是一个等待呈现在屏幕上的图像队列。我们的应用程序将获取这样一个图像来绘制它，然后将其返回到队列中。队列的工作原理以及从队列中呈现图像的条件取决于交换链的设置方式，但交换链的一般目的是使图像的呈现与屏幕的刷新率同步。需要定义交换链的原因是Vulkan没有一个默认的帧缓冲区、

交换链并不是所有显卡都支持的，因此我们需要进行验证，然后通过逻辑设备启动这个拓展

```c++
createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
createInfo.ppEnabledExtensionNames = deviceExtensions.data();
```

而仅仅只检查交换链是否可以用是不够的，因为它和我们前面创建的surface可能并不兼容。所以我们还是需要查询更多的信息。

基本上我们需要检查三种属性：

- 基本表面功能（交换链中的最小/最大图像数，图像的最小/最大宽度和高度）
- 表面格式（像素格式、色彩空间）
- 可用的演示模式

> - Basic surface capabilities (min/max number of images in swap chain, min/max width and height of images)
> - Surface formats (pixel format, color space)
> - Available presentation modes

因此，我们先定义了结构体

```c++
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};
```

下面的示例代码展示了如何获取这些属性

```c++
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;
		//通过vkGetPhysicalDeviceSurfaceCapabilitiesKHR函数来获取capabilities
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            //如果存在有可用的，才获取format
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            //获取演示模式
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
  }
```

然后我们就可以在其他函数中通过获取到的这些属性进行判断。当我们确定了交换链可用之后，还会有一些其他的可优化操作。例如

- 表面格式（颜色深度）
- 演示模式（“交换”图像到屏幕的条件）
- 交换范围（交换链中图像的分辨率）

> - Surface format (color depth)
> - Presentation mode (conditions for "swapping" images to the screen)
> - Swap extent (resolution of images in swap chain)

##### SurfaceFormat 表面格式

选择表面格式的示例代码如下

```c++
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }
```

在我们的遍历中，每个`VkSurfaceFormatKHR`示例都包含一个`format`成员和一个`ColorSpace`成员，`format`成员的作用是指定颜色通道和类型，例如说`VK_FORMAT_B8G8R8A8_SRGB`的意思是我们用8位无符号整数的顺序存储B,G,R和alpha通道的值，每个像素总共32位，当然我们也可以扩大这个。另外一个成员通过这个`VK_COLOR_SPACE_SRGB_NONLINEAR_KHR`来判断是否支持SRPG颜色空间

##### Presentation mode 演示模式

演示模式是交换链最重要的设置，它控制了屏幕上显示图像的条件，Vulkan中有四种可能的模式

- `VK_PRESENT_MODE_IMMEDIATE_KHR`：您的应用程序提交的图像会立即传输到屏幕上，这可能会导致撕裂。
- `VK_PRESENT_MODE_FIFO_KHR`：交换链是一个队列，当显示器刷新时，显示器从队列前面获取图像，程序在队列后面插入渲染图像。如果队列已满，则程序必须等待。这与现代游戏中的垂直同步最为相似。显示刷新的那一刻被称为“垂直空白”。
- `VK_PRESENT_MODE_FIFO_RELAXED_KHR`：此模式仅在申请延迟且队列在最后一个垂直空白时为空时与前一种模式不同。图像最终到达时立即传输，而不是等待下一个垂直空白。这可能会导致画面撕裂。
- `VK_PRESENT_MODE_MAILBOX_KHR`：这是第二种模式的另一种变体。当队列已满时，不会阻塞应用程序，而是将已经排队的图像简单地替换为较新的图像。此模式可用于在避免画面撕裂的同时尽可能快地渲染帧，因此它和标准垂直同步相比延迟问题更少。这就是俗称的“三重缓冲”，不过单独存在三个缓冲并不一定意味着帧率已解锁。

> - `VK_PRESENT_MODE_IMMEDIATE_KHR`: Images submitted by your application are transferred to the screen right away, which may result in tearing.
> - `VK_PRESENT_MODE_FIFO_KHR`: The swap chain is a queue where the display takes an image from the front of the queue when the display is refreshed and the program inserts rendered images at the back of the queue. If the queue is full then the program has to wait. This is most similar to vertical sync as found in modern games. The moment that the display is refreshed is known as "vertical blank".
> - `VK_PRESENT_MODE_FIFO_RELAXED_KHR`: This mode only differs from the previous one if the application is late and the queue was empty at the last vertical blank. Instead of waiting for the next vertical blank, the image is transferred right away when it finally arrives. This may result in visible tearing.
> - `VK_PRESENT_MODE_MAILBOX_KHR`: This is another variation of the second mode. Instead of blocking the application when the queue is full, the images that are already queued are simply replaced with the newer ones. This mode can be used to render frames as fast as possible while still avoiding tearing, resulting in fewer latency issues than standard vertical sync. This is commonly known as "triple buffering", although the existence of three buffers alone does not necessarily mean that the framerate is unlocked.

和上面选择表面格式一样，我们也需要选择一个PresentMode，示例代码如下

```c++
 VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }
```

在教程的过程中，我们使用`VK_PRESENT_MODE_MAILBOX_KHR`,这个模式看上去会更好

##### 交换范围Swap extent

交换范围也是我们需要考虑的一个重要属性。交换范围是交换链图像的分辨率，它几乎总是完全等于我们要绘制的窗口的*分辨率*（*以像素*为*单位）*）。可能的分辨率范围在`VkSurfaceCapabilitiesKHR`结构中定义。Vulkan 告诉我们通过在`currentExtent`成员中设置宽度和高度来匹配窗口的分辨率 。不过在某些情况下会有所不同，这取决于我们使用的窗口的控制器（Vulkan并没有内置控制窗口的功能），这通过将宽度和高度设置`currentExtent`为特殊值来表示： 的最大值`uint32_t`。在这种情况下，我们将选择`minImageExtent`与 `maxImageExtent`边界内的窗口最匹配的分辨率。我们必须以正确的单位指定分辨率。

我们的例子中使用了GLFW来作为窗口管理器，它测量尺寸时使用了像素和屏幕坐标两种单位，我们之前在创建窗口时使用的width和height两个属性是通过屏幕坐标来测量的。在Vulkan中，使用的则是像素，因此交换链的范围也必须使用像素为单位来指定。在某些情况下，高DPI显示器屏幕坐标和像素并不对应，并且由于像素密度更高，以像素为单位的窗口分辨率要大于以屏幕坐标为单位的分辨率。因此我们必须使用`glfwGetFramebufferSize`来以像素为单位查询窗口分辨率，然后将其与最大和最小图像范围匹配。示例代码如下

``` C++
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };
			//使用clamp函数来限制width的值在width和minImageExtend的width之间
            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }
```

##### 创建交换链

准备好上面的各种配置之后，接下来应该做的就是就是创建我们的交换链。创建交换链应该在初始化逻辑设备之后执行

首先我们应该获取上面提交的属性

```c++
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
```

除了这些属性外，我们需要决定我们的交换链中有多少图像。通过属性我们可以知道运行所需的最小图像

```c++
uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
//比最小图像多1
```

同时指定完最小图像之后，我们应该指定不应该超过的最大图像

``` c++
if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
}
```

0是一个特殊值，表示没有最大值。

和创建其他Vulkan对象一样，我们也需要填充一个大型结构

``` C++
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;//指定表面
		//下面配置详细信息
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;//
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
```

`imageArrayLayers`指定层的每个图像包括的量。除非是在开发 3D 应用程序，否则始终应该是1。 `imageUsage`表示我们的图像使用的方式。在本教程中，我们将直接渲染它们，这意味着它们被用作颜色附件。您也可以先将图像渲染为单独的图像以执行后处理等操作。在这种情况下，您可以使用类似的值 `VK_IMAGE_USAGE_TRANSFER_DST_BIT`并使用内存操作将渲染图像传输到交换链图像。接下来，我们将会填写更多配置信息

```c++
        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }
		//上面的配置信息是为了判断在不同的队列族中怎么使用图片
        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;//指定对交换链中的图像应用某种变换，current就是保持当前
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;//指定是否应使用 Alpha 通道与窗口系统中的其他窗口混合，这个配置是忽略
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;//不关心被遮挡的像素的颜色

        createInfo.oldSwapchain = VK_NULL_HANDLE;//如果要重新绘制交换链，需要指定一个指向，这里是空
```

如果图形队列系列与呈现队列不同，我们将从图形队列中绘制交换链中的图像，然后将它们提交到演示队列中。有两种方法可以处理从多个队列访问的图像：

- `VK_SHARING_MODE_EXCLUSIVE`：一个图像一次由一个队列家族拥有，并且在另一个队列家族中使用它之前必须明确转移所有权。此选项可提供最佳性能。
- `VK_SHARING_MODE_CONCURRENT`：图像可以跨多个队列系列使用，无需明确的所有权转移。

```c++
vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain)//创建
vkDestroySwapchainKHR(device, swapChain, nullptr);//销毁
```

#### 图像视图（ImageView）

要使用我们的图像`VkImage`，我们需要定义一些`VkImageView`图，`ImageView`就是图像的视图。 它描述了如何访问图像以及访问图像的哪一部分，例如，如果可以被视为一个不需要任何mipmapping的2D纹理深度纹理。  

在这部分例子中，我们使用一个函数`createImageView`来为交换链中的每一个图像创建一个`ImageView`，我们之后就可以使用他

我们定义了一个属性来储存图像视图

```c++
std::vector<VkImageView> swapChainImageViews;
```

关于图像视图的创建属性如下

``` c++
VkImageViewCreateInfo createInfo{};
createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
createInfo.image = swapChainImages[i];
createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;//将图像视为1D纹理2D纹理3D纹理和立方体贴图
createInfo.format = swapChainImageFormat;
createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;//components字段允许混合颜色通道。例如，将所有通道映射到单色纹理的红色通道,这里是默认
createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
createInfo.subresourceRange.baseMipLevel = 0;//mipmap设置
createInfo.subresourceRange.levelCount = 1;
createInfo.subresourceRange.baseArrayLayer = 0;
createInfo.subresourceRange.layerCount = 1;//
```

接下来使用`vkCreateImage`来创建图像视图

```c++
vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i])
vkDestroyImageView(device, imageView, nullptr);
```

当然我们这里需要使用循环来创建多个图像视图

```c++
swapChainImageViews.resize(swapChainImages.size());
for (size_t i = 0; i < swapChainImages.size(); i++) {
    .../
}
```

### 着色器模块

Vulkan 中的着色器代码必须以字节码格式指定,这种字节码被称为`spir-v`,使用字节码格式的优势在于，GPU 供应商编写的将着色器代码转换为本地代码的编译器明显不那么复杂。Khronos 发布了他们自己的独立于供应商的编译器，可以将 GLSL 编译为 SPIR-V。该编译器旨在验证您的着色器代码是否完全符合标准，并生成一个可以随程序一起提供的 SPIR-V 二进制文件。您还可以将此编译器作为库包含在运行时生成 SPIR-V，在vulkan的教程中，我们将使用`glslc.exe`来完成这个工作

一个简单的vertexshader和fragmentshader的对比

```glsl
#version 450
//vertex
layout(location = 0) out vec3 fragColor;

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];
}
//fragment

#version 450
layout(location = 0) in vec3 fragColor;//和上面的location0部分绑定 

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}
```

输入变量不一定必须使用相同的名称，它们将使用`location`指令指定的索引链接在一起。

#### 固定功能阶段

较旧的图形 API 为图形管道的大多数阶段提供了默认状态。在 Vulkan 中，必须明确所有内容，从视口大小到颜色混合功能这些都需要我们使用一个结构体来填充

##### 顶点输入

顶点输入使用`VkPipelineVertexInputStateCreateInfo`结构描述了将传递给顶点着色器的顶点数据格式，它大致描述了两点

* 绑定：数据之间的间隔
* 属性描述，传递给顶点着色器的属性类型，绑定属性间的偏移量

由于教程中到这个阶段，我们的顶点着色器是硬编码的，因此没有数据

```c++
VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
vertexInputInfo.vertexBindingDescriptionCount = 0;
vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
vertexInputInfo.vertexAttributeDescriptionCount = 0;
vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional
```

`pVertexAttributeDescriptions`和`pVertexBindingDescriptions`成员指向用于加载顶点数据中的上述细节结构的数组。在教程的最后，我们拿到的顶点绑定如下

```js
VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
auto bindingDescription = Vertex::getBindingDescription();
auto attributeDescriptions = Vertex::getAttributeDescriptions();
vertexInputInfo.vertexBindingDescriptionCount = 1;
vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
```

##### 输入组件

输入组件``VkPipelineInputAssemblyStateCreateInfo`结构描述了两件事，从顶点绘制什么样的几何图形以及是否应该启用图元重启。前者在`topology`成员中指定，可以具有如下值：

- `VK_PRIMITIVE_TOPOLOGY_POINT_LIST`: 从顶点点
- `VK_PRIMITIVE_TOPOLOGY_LINE_LIST`：每 2 个顶点的线，无需重用
- `VK_PRIMITIVE_TOPOLOGY_LINE_STRIP`: 每行的结束顶点用作下一行的开始顶点
- `VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST`：每3个顶点的三角形，无需重用
- `VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP`: 每个三角形的第二个和第三个顶点用作下一个三角形的前两个顶点

通常情况下，这些 顶点是按照顺序从顶点缓冲区中读取的，但是当你有一个`element buffer`之后，你可以自己制定想要使用的缩影，这允许了类似于重用顶点之类的优化。当设置`primitiveRestartEnable`为`VK_TRUE`之后，就可以

##### Viewports and scissors(视口和剪刀)

视口基本上描述了输出将渲染到的帧缓冲区区域的大小，在我们的教程中，基本上就是(0,0)到(width，height)。

```c++
VkViewport viewport{};
viewport.x = 0.0f;
viewport.y = 0.0f;
viewport.width = (float) swapChainExtent.width;//交换链大小
viewport.height = (float) swapChainExtent.height;
viewport.minDepth = 0.0f;
viewport.maxDepth = 1.0f;
```

需要注意的是，交换链图片的大小和width和height可能不同，但是之后我们要将他作为帧缓冲，所以我们就使用这个大小

`minDepth`和`maxDepth`确定了帧缓冲中深度值的范围，这些值必须限定在`[0.0f,1.0f]`之间，不过`minDepth`可以比`maxDepth`要高。如果你没有做什么特别的操作的话，你应该让这些值设置为标准的0.0f和1.0f

视口定义了从图像到帧缓冲区的转换，而剪刀矩形定义了像素将实际存储在哪些区域中。剪刀矩形之外的任何像素都将被光栅化器丢弃。它们的功能类似于过滤器而不是转换。区别如下所示。请注意，左剪刀矩形只是产生该图像的众多可能性之一，只要它大于视口即可

在我们的例子中，我们会绘制整个缓冲区，因此我们将指定一个完全覆盖它的剪刀矩形

```c++
VkRect2D scissor{};
scissor.offset = {0, 0};
scissor.extent = swapChainExtent;
```

我们需要一个`VkPipelineViewportStateCreateInfo`组合这两个信息，从而得到一个视口状态。在某些显卡上可以使用多个视口和剪刀矩形，因此其成员引用它们的数组。使用多个需要启用对应的GPU功能，这需要在逻辑设备创建中使用。

```c++
VkPipelineViewportStateCreateInfo viewportState{};
viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
viewportState.viewportCount = 1;
viewportState.pViewports = &viewport;
viewportState.scissorCount = 1;
viewportState.pScissors = &scissor;
```

##### 光栅化器

光栅化器的输入是顶点着色器中输出的几何图形，然后它会将其转换为片段，交给片段着色器。它还会执行深度测试，表面剔除和剪刀测试（ scissor test,）并且可以将输出配置为填充整个多并行或者仅只有边缘的片段（线框渲染）。所有这些都是由`VkPipelineRasterizationStateCreateInfo`结构进行配置的。

```c++
VkPipelineRasterizationStateCreateInfo rasterizer{};
rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
rasterizer.depthClampEnable = VK_FALSE;
rasterizer.rasterizerDiscardEnable = VK_FALSE;
rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
rasterizer.lineWidth = 1.0f;
rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
rasterizer.depthBiasEnable = VK_FALSE;
rasterizer.depthBiasConstantFactor = 0.0f; // Optional
rasterizer.depthBiasClamp = 0.0f; // Optional
rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
```

`depthClampEnable`项需要启用对应的GPU功能才能使用，它的作用是将超出近平面和远平面的片段clamp到他们上面，而不是丢弃他们

`rasterizerDiscarEnable`项如果设为`vk_true`，则几何体永远不会通过光栅化阶段，并且这基本禁用了帧缓冲区的所有输出

`polygonMode`确定片段如何用于几何生成。可以使用以下模式：

- `VK_POLYGON_MODE_FILL`：用片段填充多边形区域
- `VK_POLYGON_MODE_LINE`: 多边形边被绘制为线
- `VK_POLYGON_MODE_POINT`: 多边形顶点绘制为点

使用除填充以外的任何模式都需要启用 GPU 功能。

`lineWidth`成员根据片段的数量描述线条的粗细，支持的最大线宽取决于硬件。

`cullmode`决定的是表面剔除模型，我们可以禁用，剔除正面，剔除背面或者两者都剔除，`frontFace`指定了要被视为正面的点的顶点顺序，可以使顺时针或者逆时针。

##### 多重采样

`VkPipelineMultisampleStateCreateInfo`结构体用于配置多重采样 ，这是抗锯齿的方法之一，他的工作原理是....这部分操作主要发生在边缘，这也是最明显的锯齿出现的地方。启用它需要启用GPU功能

```c++
VkPipelineMultisampleStateCreateInfo multisampling{};
multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
multisampling.sampleShadingEnable = VK_FALSE;
multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
multisampling.minSampleShading = 1.0f; // Optional
multisampling.pSampleMask = nullptr; // Optional
multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
multisampling.alphaToOneEnable = VK_FALSE; // 
```

##### 深度和模板测试

使用深度和/或模板缓冲区，那么还需要使用`VkPipelineDepthStencilStateCreateInfo`来定义，之后的内容里我们会提到，这里贴代码

```c++
VkPipelineDepthStencilStateCreateInfo depthStencil{};
depthStencil.sType =VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
depthStencil.depthTestEnable = VK_TRUE;
depthStencil.depthWriteEnable = VK_TRUE;
depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
depthStencil.depthBoundsTestEnable = VK_FALSE;
depthStencil.stencilTestEnable = VK_FALSE;
```

##### 颜色混合

片段着色器返回颜色后，需要将其与帧缓冲区中已有的颜色组合。这种转换称为颜色混合，颜色混合油两种处理方法：

- 混合旧值和新值以产生最终颜色
- 使用按位运算组合旧值和新值

颜色混合需要用两种类型的结构来配置颜色混合，第一个结构`VkPipelineColorBlendAttachmentState`包含了每个相关的帧缓存的配置，而另外一个结构体`VkPipelineColorBlendStateCreateInfo`则包含了全局的颜色混合设置。我们的案例中只有一个帧缓存。

```c++
VkPipelineColorBlendAttachmentState colorBlendAttachment{};
colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
colorBlendAttachment.blendEnable = VK_FALSE;
colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
```

第二个结构引用所有帧缓冲区的结构数组，并且设置混合参数。

```c++
VkPipelineColorBlendStateCreateInfo colorBlending{};
colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
colorBlending.logicOpEnable = VK_FALSE;
colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
colorBlending.attachmentCount = 1;
colorBlending.pAttachments = &colorBlendAttachment;
colorBlending.blendConstants[0] = 0.0f; // Optional
colorBlending.blendConstants[1] = 0.0f; // Optional
colorBlending.blendConstants[2] = 0.0f; // Optional
colorBlending.blendConstants[3] = 0.0f; // Optional
```

如果要使用按位混合方法的话，则应设置`loginOpEnable`的值为`VK_TRUE`。然后可以在`logicOp`字段中指定按位运算。需要注意的是，这将会自动禁用第一种方法，`colorWriteMask`也将在此模式下使用，以确认哪些帧缓冲通道会被影响到。

##### Dynamic state 动态状态

我们之前定义的一些状态是可以再不用重新创建管线就可以改变的，例如视口的大小，线宽还有混合常数，如果想要这么做的话，就需要填充一个`VkPipelineDynamicStateCreateInfo`

```c++
VkDynamicState dynamicStates[] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_LINE_WIDTH
};

VkPipelineDynamicStateCreateInfo dynamicState{};
dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
dynamicState.dynamicStateCount = 2;
dynamicState.pDynamicStates = dynamicStates;
```

##### Pipeline layout 管线布局

我们可以再着色器中使用uniform值，这些统一值需要在管道创建期间通过创建`VkPipelineLayout`对象来制定。而我们在上面创建的所有对象，之后我们也会一一使用上他们。

```c++
VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
pipelineLayoutInfo.setLayoutCount = 0; // Optional
pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
}
//清理管线
 vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
```

#### 渲染通道

在我们完成创建管线之前，我们需要告诉vulkan关于渲染时将会用到帧缓冲绑定。我们需要确定有多少颜色和深度缓冲将会被用到，每个缓冲区使用多少样本以及在整个渲染操作中应如何处理它们的内容。所有这些信息都包含在一个*渲染通道*对象中，我们将为其创建一个新`createRenderPass`函数。从`initVulkan`之前调用这个函数 `createGraphicsPipeline`。

在我们的例子中，我们将只有一个单一颜色缓冲区附件，由交换链中的一个图像表示

```c++
void createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
}
```

`format`应该和交换链图像的格式匹配，我们目前灭有使用多重采样，因此我们这里设置的sample count是1

```c++
colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
```

`loadOp`和`storeOp`两个参数确定了渲染前和渲染后，我们绑定buffer的数据的处理.`loadOp`的选择如下

- `VK_ATTACHMENT_LOAD_OP_LOAD`: 保留附件的现有内容
- `VK_ATTACHMENT_LOAD_OP_CLEAR`: 在开始时将值清除为常量
- `VK_ATTACHMENT_LOAD_OP_DONT_CARE`: 现有内容未定义；我们不在乎他们

在我们的例子中，我们将在绘制新帧之前使用清除操作将帧缓冲区清除为黑色，而另外一个参数的选择如下

- `VK_ATTACHMENT_STORE_OP_STORE`: 渲染的内容将存储在内存中，以后可以读取
- `VK_ATTACHMENT_STORE_OP_DONT_CARE`: 渲染操作后帧缓冲区的内容将是未定义的

下面的两个参数则是对于模板数据的设置

```c++
colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
```

因为我们没有任何需要模板缓冲做的事情，因此这里就是这样子了

```c++
colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
```

Vulkan中的纹理和帧缓冲区有具有特定像素格式的Vkimage对象所表示，但是内存中像素的布局可以根据我们的配置改变

- `VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL`：用作彩色附件的图像
- `VK_IMAGE_LAYOUT_PRESENT_SRC_KHR`：要在交换链中呈现的图像
- `VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL`: 用作内存复制操作目标的图像

上面提到的`initialLayout`它的布局图像将在渲染开始之前被指定，在`finalLayout`所指定的则会在渲染结束之后，`initialLayout`设置为`VK_IMAGE_LAYOUT_UNDEFINED`意味着我们不关心图像以前的布局，而且我们会清空它。我们希望图像在渲染后准备好使用交换链进行展示，这就是我们使用`VK_IMAGE_LAYOUT_PRESENT_SRC_KHR`来配置finalLayout的原因。

单个渲染通道可以包含多个子通道。子通道是后续渲染操作，它取决于之前通道中帧缓冲区的内容，例如一个接一个应用的后处理效果序列。如果您将这些渲染操作组合到一个渲染通道中，那么 Vulkan 能够重新排列这些操作并节省内存带宽，以获得更好的性能。然而，对于我们的第一个三角形，我们将坚持一个子通道。

每个子通道都引用一个或多个我们使用前面部分中的结构描述的附件。这些引用本身 `VkAttachmentReference`就是如下所示的结构：

```c++
VkAttachmentReference colorAttachmentRef{};
colorAttachmentRef.attachment = 0;
colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
```

使用`VkSubpassDescription`来描述子通道,我们必须明确说明这是一个图形子通道。接下来，我们指定对颜色附件的引用：

```c++
VkSubpassDescription subpass{};
subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
subpass.colorAttachmentCount = 1;
subpass.pColorAttachments = &colorAttachmentRef;
```

此数组中附件的索引直接从片段着色器中使用`layout(location = 0) out vec4 outColor`指令引用！

子通道可以引用以下其他类型的附件：

- `pInputAttachments`：从着色器读取的附件
- `pResolveAttachments`：用于多重采样颜色附件的附件
- `pDepthStencilAttachment`: 深度和模板数据的附件
- `pPreserveAttachments`：此子通道未使用但必须保留其数据的附件

##### 渲染通道

现在已经描述了附件和引用它的基本子通道，我们可以创建渲染通道本身。创建一个新的类成员变量以将`VkRenderPass`对象保存在`pipelineLayout`变量正上方：

```c++
VkRenderPass renderPass;
VkPipelineLayout pipelineLayout;
```

然后可以通过用`VkRenderPassCreateInfo`附件和子通道数组填充结构来创建渲染通道对象 。的`VkAttachmentReference`对象引用使用该阵列的索引附件。

```c++
VkRenderPassCreateInfo renderPassInfo{};
renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
renderPassInfo.attachmentCount = 1;
renderPassInfo.pAttachments = &colorAttachment;
renderPassInfo.subpassCount = 1;
renderPassInfo.pSubpasses = &subpass;

if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
    throw std::runtime_error("failed to create render pass!");
}
```

就像管道布局一样，整个程序都会引用渲染通道，所以应该只在最后清理它：

```c++
void cleanup() {
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);
    ...
}
```

#### 创建图形管道

我们现在可以结合前几章中的所有结构和对象来创建图形管道！这是我们现在拥有的对象类型，作为快速回顾：

- 着色器阶段：定义图形管道可编程阶段功能的着色器模块
- 固定功能状态：定义管道固定功能阶段的所有结构，如输入组装、光栅化、视口和颜色混合
- 管线布局：着色器引用的uniform和push值，可以在绘制时更新
- 渲染通道：管道阶段引用的附件及其用法

所有这些结合起来完全定义了图形管道的功能，我们结构`VkGraphicsPipelineCreateInfo`。

```c++
VkGraphicsPipelineCreateInfo pipelineInfo{};
pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
pipelineInfo.stageCount = 2;
pipelineInfo.pStages = shaderStages;
```

我们从引用`VkPipelineShaderStageCreateInfo`结构数组开始。

```c++
pipelineInfo.pVertexInputState = &vertexInputInfo;
pipelineInfo.pInputAssemblyState = &inputAssembly;
pipelineInfo.pViewportState = &viewportState;
pipelineInfo.pRasterizationState = &rasterizer;
pipelineInfo.pMultisampleState = &multisampling;
pipelineInfo.pDepthStencilState = nullptr; // Optional
pipelineInfo.pColorBlendState = &colorBlending;
pipelineInfo.pDynamicState = nullptr; // Optional
```

然后我们参考描述固定功能阶段的所有结构。

```c++
pipelineInfo.layout = pipelineLayout;
```

之后是管道布局

```c++
pipelineInfo.renderPass = renderPass;
pipelineInfo.subpass = 0;
```

最后，我们获得了渲染通道的引用以及将使用此图形管道的子通道的索引。也可以使用其他渲染通道与这条管线，而不是这种特定情况下，但他们必须要*兼容*与`renderPass`。[此处](https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#renderpass-compatibility)描述了兼容性要求，但我们不会在本教程中使用该功能。

```c++
pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
pipelineInfo.basePipelineIndex = -1; // Optional
```

实际上还有两个参数：`basePipelineHandle`和 `basePipelineIndex`。Vulkan 允许您通过从现有管道派生来创建新的图形管道。管道派生的想法是，当管道与现有管道具有许多共同功能时，设置管道的成本会更低，并且可以更快地在来自同一父管道的管道之间进行切换。您可以使用 指定现有管道的句柄，也可以使用`basePipelineHandle`引用即将由索引创建的另一个管道`basePipelineIndex`。现在只有一个管道，所以我们将简单地指定一个空句柄和一个无效的索引。仅当`VK_PIPELINE_CREATE_DERIVATIVE_BIT` 在`flags`字段中指定了标志时才使用这些值`VkGraphicsPipelineCreateInfo`。

现在通过创建一个类成员来保存`VkPipeline`对象来准备最后一步 ：

```c++
VkPipeline graphicsPipeline;
```

最后创建图形管道：

```c++
if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
    throw std::runtime_error("failed to create graphics pipeline!");
}
```

该`vkCreateGraphicsPipelines`函数实际上比 Vulkan 中通常的对象创建函数具有更多的参数。它旨在接受多个 `VkGraphicsPipelineCreateInfo`对象并`VkPipeline`在一次调用中创建多个对象。

我们为其传递了`VK_NULL_HANDLE`参数的第二个参数引用了一个可选`VkPipelineCache`对象。管道缓存可用于存储和重用与管道创建相关的数据，`vkCreateGraphicsPipelines`如果缓存存储到文件，则跨多次调用 甚至跨程序执行。这使得以后可以显着加快管道创建速度。我们将在管道缓存章节中讨论这一点。

所有常见的绘图操作都需要图形管道，因此它也应该只在程序结束时销毁：

```c++
void cleanup() {
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    ...
}
```

到这里，画出一个三角形的最关键步骤以及走完了，接下来就是绘制这个三角形了

### 绘制

##### 帧缓冲区

> 在渲染过程创建期间指定的附件通过将它们包装到一个`VkFramebuffer`对象中来绑定。帧缓冲对象引用所有VkImageView`表示附件的 对象。在我们的例子中，只有一个：颜色附件。然而，我们必须用于附件的图像取决于当我们检索一个图像进行展示时交换链返回的图像。这意味着我们必须为交换链中的所有图像创建一个帧缓冲区，并使用与绘制时检索到的图像对应的帧缓冲区。

因此，我们需要创建一个vector类成员来保存帧缓冲区

```c++
std::vector<VkFramebuffer> swapChainFramebuffers;
```

按照教程一路操作，我们最后需要遍历图像视图，并创建帧缓冲区

关键代码如下

```c++ 
for (size_t i = 0; i < swapChainImageViews.size(); i++) {
    VkImageView attachments[] = {
        swapChainImageViews[i]
    };

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;//绑定渲染通道
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;//这个attachment 就是imageview
    framebufferInfo.width = swapChainExtent.width;//确定大小
    framebufferInfo.height = swapChainExtent.height;
    framebufferInfo.layers = 1;//我们的交换链是单图的，因此我们的layer是1

    if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    }
}
```

##### 命令缓冲区

Vulkan 中的命令，如绘图操作和内存传输，不直接使用函数调用执行。必须在命令缓冲区对象中记录要执行的所有操作。这样做的好处是所有设置绘图命令的繁重工作都可以提前在多线程中完成。之后，您只需告诉 Vulkan 执行主循环中的命令。

要使用命令缓冲区，我们需要先建立一个命令池

关键代码如下

```c++
QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

VkCommandPoolCreateInfo poolInfo{};
poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
poolInfo.flags = 0; // Optional
```

命令缓冲区需要将命令提交到设备队列之一来执行，我们的例子中，有一个图形队列和一个演示队列，每个命令池只能分配在单一类型队列上提交的命令缓冲区，我们的目的是提交绘图命令，因此需要绑定图形队列。，

命令池有两个可能的标志：

- `VK_COMMAND_POOL_CREATE_TRANSIENT_BIT`：提示命令缓冲区经常用新命令重新记录（可能会改变内存分配行为）
- `VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT`: 允许单独重新记录命令缓冲区，如果没有这个标志，它们都必须一起重置

我们只会在程序开始时创建命令缓冲区，然后在主循环中多次执行它们，因此我们不会使用这些标志中的任何一个。

创建和销毁的例子如下

```c++
vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool)
vkDestroyCommandPool(device, commandPool, nullptr);
```

##### 渲染和演示

此章节的作用是画一个三角形，我们会定义一个drawFrame函数，这个函数将执行三个操作

- 从交换链中获取图像
- 使用该图像作为帧缓冲区中的附件执行命令缓冲区
- 将图像返回到交换链进行展示

这些事件中的每一个都使用单个函数调用来启动，但它们是异步执行的。函数调用将在操作实际完成之前返回，执行顺序也未定义。有两种同步交换链事件的方法：栅栏和信号量。它们都是可用于协调操作的对象，方法是让一个操作信号和另一个操作等待栅栏或信号量从无信号状态变为有信号状态。不同之处在于可以从您的程序中使用类似的调用访问栅栏的状态，`vkWaitForFences`而信号量则不能。栅栏主要用于同步应用程序本身与渲染操作，而信号量用于同步命令队列内或跨命令队列的操作。我们希望同步绘制命令和呈现的队列操作，这使得信号量最适合。

个人的理解的话，这个栅栏的效果就是await

##### 信号量

我们教程中使用了createSemaphores这个函数来进行信号量的创建

```c++
   VkSemaphoreCreateInfo semaphoreInfo{};
   semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

```

它实际上没有任何必填字段 ,除了stype

我们会创建两个信号量

```c++
vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS; 
vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS
```



当所有命令都完成并且不再需要同步时，应该在程序结束时清除信号量：

```c++
 	vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
    vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
```

##### 从交换链中获取图像

在绘制我们的图像之前，我们需要在`drawframe`函数中从我们的交换链中获取到需要绘制的图像，由于交换链是一个拓展特性，因此我们需要一个`vk...KHR`的接口来做这件事

```c++
void drawFrame() {
    uint32_t imageIndex;
    vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
}
```

`vkAcquireNextImageKHR`函数的前两个参数的意义是我们需要从逻辑设备的交换链中获取图像，第三个参数指定了图像可用的超时时间（纳秒），使用64位无符号整数的最大值可禁用超时

第三个参数用于指定当使用完这个图像后需要触发的信号，这就是我们可以开始绘制它的时间点，我们可以指定信号量，栅栏或者两者都指定，我们这里使用我们上面创建的`imageAvailableSemaphore`

最后一个参数指定一个变量来输出可以用的图像索引，这个索引指向的是我们的swapChainImages中的vkImage，我们可以使用这个索引来选择正确的命令缓冲区

##### 提交命令缓冲区

在提交命令缓冲区时，我们需要做的事情依旧是先配置一个结构体，这里我们通过`VkSubmitInfo`结构来配置。完整的配置代码如下

```c++
VkSubmitInfo submitInfo{};
submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
//下面三个参数的作用是指定在执行开始之前需要等待的信号量和需要等待的管道阶段
submitInfo.waitSemaphoreCount = 1;
submitInfo.pWaitSemaphores = waitSemaphores;
submitInfo.pWaitDstStageMask = waitStages;
//下面两个参数指定了实际提交哪些命令缓冲区一共执行
submitInfo.commandBufferCount = 1;
submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
//signalsemaphocount和psignalsemaphhores参数指定在命令缓冲区完成执行后发送哪些信号量
VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
submitInfo.signalSemaphoreCount = 1;
submitInfo.pSignalSemaphores = signalSemaphores;
```

我们希望等待将颜色写入图像直到它可用，因此我们指定了写入颜色附件的图形管道的阶段waitStages数组中的每个条目对应于pWaitSemaphores中具有相同索引的信号量。 

接下来我们就可以提交命令缓冲区到图形队列

```c++
vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE)//成功则返回vk_success
```



当整体负载更大时，我们可以使用数组来作为参数，提交命令缓冲区的函数`vkQueueSubmit`的最后一个参数引用一个可选的栅栏，当命令缓冲区执行完时，这个栅栏就会发出信号，不过我们使用信号量，因此这里就是`vk_null_handle`

##### subpass depedency(子通道依赖)

渲染通道中的子通道会自动处理图像布局转换，这些转换有子通道依赖项控制，子通道依赖项指定了子通道之间的内存和依赖。虽然我们现在只有一个子通道，但是这个子通道之前的操作和之后的操作也可以算作隐式的"子通道"

有两个内置的子通道依赖项来控制渲染通道开始和结束时的转换。但是前者的（默认）运行时间是错误的，它假设转换发生在管道的最开始，但是那时我们还没能获取图像。我们有两种办法能解决这个问题，第一个办法是我们可以将`imageAvailableSemaphore`的`waitStages`修改为`VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT`来确保渲染通道在图像可用之前不会开始，我们也可以让渲染通道等待`VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT`这个状态。在这个例子中，我们使用第二种方法。

子传递依赖是在VkSubpassDependency结构体中指定的。我们需要在createRenderPass中添加它代码如下

```c++
        VkSubpassDependency dependency{};
		//前两个字段指定依赖项和从属子通道的索引
//特殊值VK_SUBPASS_EXTERNAL指的是渲染传递之前或之后的隐式子传递，这取决于它是在srcSubpass还是dstSubpass中指定的。
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
//索引0指向我们的子通道。dstSubpass必须总是高于srcSubpass，以防止依赖图中的循环(除非其中一个子pass是VK_SUBPASS_EXTERNAL)。
        dependency.dstSubpass = 0;
//下来的两个字段指定要等待的操作以及这些操作发生的阶段。 我们需要等待交换链完成对图像的读取，然后才能访问它。 这可以通过等待颜色附件输出阶段本身来完成。
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
//我们应该等待的操作在颜色附件阶段，涉及颜色附件的写入。这些设置将阻止过渡发生，直到我们想要开始向其写入颜色时。
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

//在renderpassinfo中写入
renderPassInfo.dependencyCount = 1;
renderPassInfo.pDependencies = &dependency;
```

##### 显示

绘制的最后一步是将结果提交回交换链，使其最终显示在屏幕上。我们通过drawFrame函数末尾的`VkPresentInfoKHR`结构来配置,源码如下

```c++
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		//前两个参数和submitinfo一样，指定了需要等待的信号量
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
		//后面的这三个参数指定了需要显示图像的交换链和图像的索引
        VkSwapchainKHR swapChains[] = {swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;
		//提交图像给交换链
        vkQueuePresentKHR(presentQueue, &presentInfo);
```

由于`drawFrame`的所有操作都是一步的，所以我们退出mainloop的时候，绘图和演示操作可能还在，因此我们需要在退出mainloop之前等待逻辑设备完成操作

```c++
vkDeviceWaitIdle(device);
```

##### 帧控制

目前的代码还是存在一个问题，那就是CPU的提交速度可能会快于GPU的处理速度（drawFrame中的提交），而提交后并没有检查它是否会完成，并且我们之前的代码中，我们在多个帧中重用`imageAvailableSemaphore`和`renderFinishedSemaphore`信号量以及命令缓冲区。因此我们需要在提交后等待工作完成。我们可以使用

```c++
vkQueueWaitIdle(presentQueue);
```

函数来等待，但是这样的话，整个图形管道现在一次只用于一帧。 当前帧已经通过的阶段是空闲的，可以用于下一帧。 现在我们将扩展我们的应用程序，允许多个帧在运行的同时仍然限制堆积起来的工作量。  

首先在程序的顶部添加一个常量，它定义了并发处理多少帧:  

```c++
const int MAX_FRAMES_IN_FLIGHT = 2;
```

接下来，我们为每一帧都定义一组信号量，这样我们就需要一个容器来存放它

```c++
std::vector<VkSemaphore> imageAvailableSemaphores;
std::vector<VkSemaphore> renderFinishedSemaphores;
```

因此，在创建和清理时，我们也应该用一个循环进行创建

```c++
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS) {

            throw std::runtime_error("failed to create semaphores for a frame!");
    }
 // 清理
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
    }
        
```

同时，为了让每一帧都使用正确的信号量，我们需要追踪当前帧，所以我们需要一个新的变量

```c++
size_t currentFrame = 0;
```

在drawFrame函数中，我们也需要修改一下使其使用正确的对象

```c++
VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
```

我们上一章节的对应代码已经使用了数组的形式，就是这个原因。

在drawFrame函数中，我们也需要和循环一样处理当前帧的索引号

```c++
currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
```

做好了每帧所需要的准备后，我们需要一个栅栏（fences）来放置有大量的提交，这个栅栏的目的是CPU-GPU同步，目前的情况下，我们有可能正在使用第0帧，而第0帧正在处理中。我们将会为每一帧创建一个栅栏。同时当我们创建完栅栏后，内存泄漏依旧存在，这是由于如果如果`MAX_FRAMES_IN_FLIGHT`高于交换链图像的数量或`vkAcquireNextImageKHR`返回无序图像，那么我们可能会开始渲染一个在途的图像。因此我们需要一个新的列表来跟踪这个。综上，我们会将`createSemaphores`函数修改为`createSyncObjects`函数，源码如下

```c++
//信号列表
imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
//新的跟踪列表
//由于最开始没有任何帧使用图像，所以我们会初始化为no fence
imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);
VkSemaphoreCreateInfo semaphoreInfo{};
semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
//栅栏信息
VkFenceCreateInfo fenceInfo{};
//在有信号状态下初始化，效果类似于渲染完成了一个初始帧
fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
       	vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
        //这里创建栅栏
        vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
        throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
}

//清理信号和栅栏
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }
```

接下来，我们需要将`drawFrame`修改为使用栅栏进行同步，完整的函数较长，如下

```c++
//开始时等待帧已完成		
vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
		 // 检查前一帧是否使用此图像(即，有它的栅栏等
        if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
		//标记此帧正在使用的图像
        imagesInFlight[imageIndex] = inFlightFences[currentFrame];

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
		//调整了这个函数的位置
        vkResetFences(device, 1, &inFlightFences[currentFrame]);
// 我们使用栅栏进行同步，传入了一个inFlightFences的栅栏，用它来表示一帧完成。
        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;
		//提交命令
        vkQueuePresentKHR(presentQueue, &presentInfo);
		//控制帧
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
```

到这里，我们现在已经实现了所有需要的同步，以确保不超过两帧的工作队列，并且这些帧不会意外地使用相同的图像。 请注意，对于代码的其他部分(如最终的清理)，依赖于更粗糙的同步(如vkDeviceWaitIdle)是可以的。 您应该根据性能需求决定使用哪种方法。  

### 重建交换链

这部分内容比较简单，主要作用是可以让你改变窗口的大小，需要部分清理以及重建交换链。



