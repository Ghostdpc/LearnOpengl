### webassembly是什么

webassembly是什么这个问题，根据MDN的说法:

> WebAssembly是一种运行在现代网络浏览器中的新型代码，并且提供新的性能特性和效果。它设计的目的不是为了手写代码,而是为诸如C、C++和Rust等低级源语言提供一个高效的编译目标。
>
> 对于网络平台而言，这具有巨大的意义——这为客户端app提供了一种在网络平台以接近本地速度的方式运行多种语言编写的代码的方式；在这之前，客户端app是不可能做到的。
>
> 而且，你在不知道如何编写WebAssembly代码的情况下就可以使用它。WebAssembly的模块可以被导入的到一个网络app（或Node.js）中，并且暴露出供JavaScript使用的WebAssembly函数。JavaScript框架不但可以使用WebAssembly获得巨大性能优势和新特性，而且还能使得各种功能保持对网络开发者的易用性。

简单的概括一下，我们可以看到两个要点

1. 跨平台，为C++,C,Rust等语言提供一个高效的编译目标，可以通过js使用。
2. 高性能，在web上可以接近本地速度的方式进行运行。

而他的标准如下

> - 快速、高效、可移植——通过利用[常见的硬件能力](http://webassembly.org/docs/portability/#assumptions-for-efficient-execution)，WebAssembly代码在不同平台上能够以接近本地速度运行。
> - 可读、可调试——WebAssembly是一门低阶语言，但是它有确实有一种人类可读的文本格式（其标准即将得到最终版本），这允许通过手工来写代码，看代码以及调试代码。
> - 保持安全——WebAssembly被限制运行在一个安全的沙箱执行环境中。像其他网络代码一样，它遵循浏览器的同源策略和授权策略。
> - 不破坏网络——WebAssembly的设计原则是与其他网络技术和谐共处并保持向后兼容。

因此我们得到最终的结论，总结一下就是，WebAssembly（wasm）就是一个可移植、体积小、加载快并且兼容 Web 的全新格式。

假设我们现在有一个段C++源码，然后我们将其编译成了wasm的形式，那么，在浏览器中，它运行时就是以二进制源码运行的。

```js
 00 61 73 6d  01 00 00 00  01 0c 02 60  02 7f 7f 01
  7f 60 01 7f  01 7f 03 03  02 00 01 07  10 02 03 61
  64 64 00 00  06 73 71 75  61 72 65 00  01 0a 13 02
  08 00 20 00  20 01 6a 0f  0b 08 00 20  00 20 00 6c
  0f 0b
```

而在我们的分析中，它的代码可能就是长这样的

```js
(func $encodeScript (;2;) (export "encodeScript") (param $var0 i32) (param $var1 i32) (result i32)
    (local $var2 i32) (local $var3 i32)
    local.get $var1
    call $func3
    local.set $var2
    block $label0
      local.get $var0
      i32.load8_u
      i32.eqz
      br_if $label0
      i32.const 0
      local.set $var1
      loop $label1
        local.get $var0
        local.get $var1
        i32.add
        local.tee $var3
        local.get $var3
        i32.load8_u
        local.get $var2
        i32.xor
        i32.store8
        local.get $var1
        i32.const 1
        i32.add
        local.tee $var1
        local.get $var0
        call $func3
        i32.ne
        br_if $label1
      end $label1
    end $label0
    local.get $var0
  )
```

这种代码可以说，基本上是难以阅读和编辑的，这也变相的导致了代码的安全性大大的提高了。同时，相较于直接写的JavaScript,一般情况下，webassembly的运行速度会更快。（**当然，运用一些高超的编程技巧，直接使用JavaScript编写的代码的性能是可以达到甚至高于wasm的运行速度的**）

### Wasm为什么会更快

WebAssembly 比 JavaScript 执行更快主要是因为下面的几个点：

- 文件抓取阶段，WebAssembly 比 JavaScript 抓取文件更快。即使 JavaScript 进行了压缩，WebAssembly 文件的体积也比 JavaScript 更小，即便通过压缩算法可以显著地减小 JavaScript 的包大小，但是压缩后的 WebAssembly 的二进制代码依然更小。；
- 解析阶段，WebAssembly 的解码时间比 JavaScript 的解析时间更短，到达浏览器时，JavaScript 源代码就被解析成了抽象语法树，而 WebAssembly 则不需要这种转换，因为它本身就是中间代码。它要做的只是解码并且检查确认代码没有错误就可以了；
- 编译和优化阶段，WebAssembly 更具优势，因为 WebAssembly 的代码更接近机器码，而 JavaScript 要先通过服务器端进行代码优化。
- 重优化阶段，WebAssembly 不会发生重优化现象。而 JS 引擎的优化假设则可能会发生“抛弃优化代码<->重优化”现象（重优化阶段主要是JIT优化时，会对warm的代码进行一个优化）。
- 执行阶段，WebAssembly 更快是因为开发人员不需要懂太多的编译器技巧，而这在 JavaScript 中是需要的，但是为了可读性，开发人员不一定会编写这种代码。WebAssembly 代码则是编译后代码，也更适合生成机器执行效率更高的指令。
- 垃圾回收阶段，WebAssembly 垃圾回收都是手动控制的，效率比自动回收更高。

这就是为什么在大多数情况下，同一个任务 WebAssembly 比 JavaScript 表现更好的原因。

想要理解上面的点，我们需要理解JavaScript的引擎的执行的机制。这里不展开讲，推荐6篇文章

[1.生动形象的介绍Webassembly](https://zhuanlan.zhihu.com/p/25800318)

[2.JavaScript Just-in-time (JIT) 工作原理](https://zhuanlan.zhihu.com/p/25669120)

[3.编译器如何生成汇编](https://zhuanlan.zhihu.com/p/25718411)

[4.WebAssembly 工作原理](https://zhuanlan.zhihu.com/p/25754084)

[5.为什么WebAssembly更快](https://zhuanlan.zhihu.com/p/25773367)

[6.现在和未来](https://zhuanlan.zhihu.com/p/25799683)

### Wasm的缺点

引用我阅读过的文章的一段话

> 第一个问题是 WebAssembly 的使用场景，但是不能为了用新技术而用新技术，得找到最适合使用 WebAssembly 而且具备不可替代性的场景，目前来看，客户端上用在视频、游戏、AR、AI 等领域比较合适。
>
> 第二个问题是促进 WebAssembly 的发展，解决实践中的问题帮助它落地。要实现 WebAssembly 在真实业务场景中落地，还需要继续完善基础设施，我很期待社区能够有人解决下面几个问题：
>
> 在工程层面解决 WebAssembly 研发链路的问题。现在无论是开发、编译还是调试都会遇到很多问题，开发体验和开发效率都比较低。目前我个人觉得比较靠谱的三种开发语言是 C++、Rust 和 AssemblyScript，分别面向不同类型的开发者。
> 在平台侧解决 WebAssembly 模块的管理问题。解决 wasm 在真正使用时的加载、分发、依赖管理、复用等问题，要是能构建出 npm 这样丰富的生态就好了。
> 在客户端/服务端解决 WebAssembly 独立运行时的问题。能够把丰富的平台原生能力，高效的、标准的透出到 wasm 模块中，并且解决性能、稳定性、安全性等问题。
> 性能优化！性能优化！性能优化！ 无需多说，性能优化永无止境。
> 等上面的基础设施建设完成后，可以为 WebAssembly 的落地扫清大部分障碍。

从目前来看，wasm的运用门槛相对于其他JavaScript模块要更加复杂，需要开发者具有另外一门语言运用能力，理解语言的编译流程，同时在开发过程中，开发者所做的工作主要是在两门不同的语言之间的相互调用，总体复杂度较高。而目前而言，对于整体的链路，无论是开发、编译还是调试都会遇到很多问题，开发体验和开发效率都比较低（从我个人的经验和体验来看），总体而言是一个好用却难用的工具。

### Wasm实际使用

#### 技术准备

1.  **一门熟悉的语言：C/C++,JAVA,GO,TYPESCRIPT,RUST....**
2. 搭建编译环境，参考文档[emscripten](https://emscripten.org/docs/getting_started/downloads.html)

在环境搭建完成后，在windows平台下，需要将emscripten的环境变量配置到用户环境变量中，才能比较方便的使用，而需要配置的变量，可以通过emsdk下的emcmdprompt.bat 运行来看具体需要设置哪些环境变量

#### 一个简单的示例

将一个C++文件编译为wasm只需要三步

1. 编写C++代码
2. 使用emscripten编译为wasm
3. 编写js使用代码

以我们的`hello world.c为例`

``` c
#include <stdio.h>
int main() {
  printf("Hello World!\n");
  return 0;
}
```

我们可以执行如下代码就可以生成 wasm 的包：

```javascript
emcc hello.c -O3 -o out/hello-emcc.wasm
```

但是，上面这个命令隐含了 -s STANDALONE_WASM 的配置 ，实际上触发的是 WebAssembly Standalone build ⑩，只生成了一个 wasm 的包，需要自己写 loader 加载和执行。如果不想费这个劲，就可以使用如下命令直接生成 wasm + js 文件：

```javascript
emcc hello.c -O3 -o out/hello-emcc.js
```

该命令除了生成 js 文件以外，还会生成同名的 hello-emcc.wasm 文件，可以使用 WABT ⑪ (WebAssembly Binary Toolkit) 提供的小工具把 wasm 文件转成对等的文本格式，方便阅读。

```javascript
wasm2wat out/hello-emcc.wasm -o out/hello-emcc.wat
```

>  代码比较短，但是生成出来的 wasm 文件有 2.1KB，js 文件 16KB，主要是因为 stdio.h 头文件里有很多依赖，在运行时是由 js 代码来实现的。用 wasm 做 io 本身也不是个好的用法。 

最后，直接在 Node.js 环境里执行这个 js 文件就行了，可以看到控制台输出了 Hello World! 。

```javascript
node out/hello-emcc.js
```

#### 了解编译参数

#### 常见的坑点总结







