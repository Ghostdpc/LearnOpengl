#### shadowmap

优点：

图像空间的算法，不需要场景的几何信息

缺点：

自遮挡和走样

算法概述

1. pass1,输出一副深度图（能看到的最浅的深度）
2. pass2，从眼睛出发，确定是否能被light照到。



精度导致的自遮挡：记录的深度不连续

处理方式：bias，可以是变动的，不一定要死定一个值

其他处理方式：级联阴影

斜角大的时候容易出现

bias会导致一个截断，detached shadow。

处理方式，合适的bias



second depth shadow mapping

此处是记录最小深度和次小深度，然后取中间值

性能较差，而且需要是有正面和反面的模型



实时渲染只相信绝对的速度



pcss

s1 blocker search

计算遮挡物的平均深度

s2 使用平均深度确定filter size

s3 pcf

1,3都是开销大



