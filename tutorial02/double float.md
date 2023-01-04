# 计算机中小数的存储



本文修改自知乎文章：[《浮点数的底层原理和精度损失问题 》](https://zhuanlan.zhihu.com/p/269619376)，作者[TOMOCAT](https://www.zhihu.com/people/mian-bei-juan-da-cong)



## 1. 问题引入：浮点数丢失精度的问题



众所周知，浮点数对数字的存储仅仅是近似的，所以会有一些比较奇怪的问题出现：

```python
>>> 0.1+0.2==0.3
False
>>> 0.1+0.2
0.30000000000000004
```

而另外一些场合，我们不能接受这种近似，比如想写一个计算器程序，我们希望0.1+0.2结果是0.3，而不是0.30000000000000004



这篇文章回顾一下浮点数如何存储的，以及如何解决小数近似的问题



## 2. 小数的存储

### 2.1 定点数存储

也就是将整数部分和小数部分分开存储。比如说一个32位变量，可以是：

- 第1位为符号位，0为正数，1为负数。
- 第2-9位为整数数值部分
- 10-32位为小数的数值部分

以存储78.375为例，

![img](D:\projectTodo\json\json-tutorial\tutorial02\images\double float\v2-cb2ae52724d3d8c4b2d52934ca495f18_r.jpg)

这种存储方式：

- 表示范围为（-256，256）
- 精度为2^(-23)



有两个比较严重的问题

1. 表示范围太小。当然，可以把整数位设的更长，但是表示范围仍然有限。2
2. 精度并非是十进制。日常习惯使用的都是十进制，一些在十进制中的有限小数转换为2进制可能变成循环小数。

第2点举个例子： 比如0.1，转换成二进制是0.0(0011)， 括号内为循环小数循环体。 这个循环小数不能在有限位中精确存储。



以上的小数可以认为是先把该数字当成整数。然后乘以$2^{-23}$。 如果想要解决问题2，可以将小数设定为在整数的基础上乘以$10^{-5}$。 



### 2.2 浮点数存储

#### 2.2.1   IEEE 754x浮点数

以32位浮点数为例，最高一位是符号位s，接着的8位是指数位E，最后的23位是有效数字M。double64最高一位是符号位，有11个指数位和52个有效数字位。下图展示了float32类型的底层表示：

![img](D:\projectTodo\json\json-tutorial\tutorial02\images\double float\v2-21a0b7e3c9a690838f1015d0420d3cef_1440w-1672744570329.webp)

其中IEEE 754的规定为：

- 因为M的取值一定是1<=M<2，因此规定M在存储时舍弃第一个1，只存储小数点之后的数字，这样可以节省存储空间（以float32为例，可以保存23位小数信息）
- 指数E是一个无符号整数，因此它的取值范围为0~255，但是指数可以是负的，所以规定在存入E时在它原本的值加上127（使用时减去中间数127），这样E的取值范围为-127~128
- E不全为0，不全为1：正常的计算规则，E的真实值就是E的字面值减去127（中间值），M加回1
- E全为0：指数E等于1-127为真实值，M不加回1（此时M为0.xxxxxx），这样是为了表示0和一些很小的数
- E全为1：M全为0表示正负无穷大（取决于S符号位）；M不全为0时表示不是一个数（NaN）

![img](D:\projectTodo\json\json-tutorial\tutorial02\images\double float\v2-39e14cdd4197560b52e88e0ba7f2a30a_1440w.webp)

### 2.2.2 具体例子

以78.375为例，它的整数和小数部分可以表示为： $(78)_{10}=(1001110)_2(0.375)_{10}=38=14+18=2−2+2−3=(0.01)_2+(0.001)_2=(0.011)_2 $因此二进制的科学计数法为：

$(78.375)_{10}=(1001110.011)_2=1.001110011×2^6$

一般而言转换过程包括如下几步：

- 改写整数部分：将整数部分的十进制改写成二进制
- 改写小数部分：拆成 2−1 到 2−N 的和
- 规格化：保证小数点前只有一位是1，改写成二进制的科学计数法表示
- 填充：指数部分加上127（中间数）填充E；有效数字部分去掉1后填充M

按照前面IEEE 754的要求，它的底层存储为：

![img](D:\projectTodo\json\json-tutorial\tutorial02\images\double float\v2-8b7e99663fb0c5e80dc897a157519734_1440w.webp)

### 2.2.3 特性分析

- 表示范围

  - 单精度浮点数float占四个字节，表示范围-3.40E+38 ~ +3.40E+38

  - 双精度浮点数double占八个字节，表示范围-1.7E+308 ~ +1.79E+308

- 精度

和指数有关。以32位浮点数为例， M的精度是$2^{-23}$，但考虑指数的话精度还需要乘以指数，范围会在$2^{-150}\sim2^{105}$之间。

- 仍然不能精确存储一些简单的十进制数。





# 3. 解决近似问题

由于十进制小数存储为2进制时不可避免地会出现无限循环小数，导致存储时是近似的。



个人思考： 如果想消除掉二进制数的影响，就不能使用二进制小数来逼近十进制小数。



考虑方案是采用8421BCD编码，这样更贴合十进制数字。

比如采用定点存储

32位数字，1位符号位，16位整数部分，12位小数部分，另外3位不使用。

存储130.71,转换位bcd编码是0 0001 0011 0000. 0111 0001

![image-20230103191244872](D:\projectTodo\json\json-tutorial\tutorial02\images\double float\image-20230103191244872.png)

这种方法可以精确保存-9999~+9999之间的任意一个3位小数。

当然，浮点数也完全可以使用这个方案，不过相对复杂一些。