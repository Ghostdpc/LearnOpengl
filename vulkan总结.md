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

