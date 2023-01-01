# 一、项目概述

本项目目标是制作一个json解析、操作的模块。

## 1. json是什么

json是一种用于交换信息的文本格式，一个典型的json信息是这样的：

```json
{
    "title": "Design Patterns",
    "subtitle": "Elements of Reusable Object-Oriented Software",
    "author": [
        "Erich Gamma",
        "Richard Helm",
        "Ralph Johnson",
        "John Vlissides"
    ],
    "year": 2009,
    "weight": 1.8,
    "hardcover": true,
    "publisher": {
        "Company": "Pearson Education",
        "Country": "India"
    },
    "website": null
}
```

可以看出，json有6种数据类型：

- null: 表示为 null
- boolean: 表示为 true 或 false
- number: 一般的浮点数表示方式，在下一单元详细说明
- string: 表示为 "..."
- array: 表示为 [ ... ]
- object: 表示为 { ... }





## 2. 项目目标

如图，我们的项目包括如下3个功能：

1. parse: 解析JSON文本，并将其保存为我们设计的数据结构（tree）
2. access:提供对tree进行操作和访问的接口
3. stringify: 将tree解析为json文本

![image-20221231103049858](images\1\image-20221231103049858.png)



这些数据类型中，可以看出null,boolean、number、string都是比较容易处理的。而array和object都是一个集合，可以包含其他数据类型，需要解析为一个树形结构。



# 二、搭建项目环境

项目使用cmake来构建项目文件

在Cmake官网下载适合自己的版本：[Download | CMake](https://cmake.org/download/)

本人下载了windows x64版本。

## 

安装后打开cmake gui:

- 配置路径：

![image-20221231125040443](images\1\image-20221231125040443.png)

- 设置生成器。我电脑安装了VS2019,所以选择VS2019

![image-20221231125250003](images\1\image-20221231125250003.png)

在cmake-gui底部可以看到提示信息`Configuring done`

- 点击generate按钮

![image-20221231130312114](images\1\image-20221231130312114.png)

- 从vs2019中打开项目文件

点击open Project可以从VS2019打开文件（也可以打开build文件夹中的sln文件）

![image-20221231130546522](images\1\image-20221231130546522.png)

现在就可以运行项目了。

运行结果：

		xxx\tutorial01\test.c:56: expect: 3 actual: 0
		11/12 (91.67%) passed

# 三、 头文件与API设计

现在可以打开项目文件，查看代码了

## 1、ifndef

为防止头文件重复定义，可以使用

```c
#ifndef xxx
#define xxx

/*头文件内容*/

#endif
```

这里的宏`xxx`在语法角度可以随意定义，但必须是唯一的。通常习惯以 `_H__` 作为后缀。由于 leptjson 只有一个头文件，可以简单命名为 `LEPTJSON_H__`。如果项目有多个文件或目录结构，可以用 `项目名称_目录_文件名称_H__` 这种命名方式。

## 2. 类型定义

先将各种数据类型定义为枚举：

```c
typedef enum { LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT } lept_type;
```

这里的boolean型被定义为false和true

这7种类型可以分为3类：

- 只有类型，不需要保存值。包括TRUE,FALSE,NULL
- 需要保存值,但是值不是集合。包括 NUMBER, STRING
- 需要保存值，值是一个集合。包括ARRAY, OBJECT





<font color=red size=5>目前先从最基本的第一类做起。只需要解析TRUE,FALSE,NULL</font>

```c
typedef struct {
    lept_type type;
}lept_value;
```

因为第一类数据(TRUE,FALSE,NULL)不需要保存值，所以结构体left_value中仅包含一个left_type就可以存储数据了。



## 3. API定义

### 解析json：

```c
int lept_parse(lept_value* v, const char* json);
```

- 函数功能： 将json文本解析为lept_value类型
- json: 传入参数，一个以\0结尾的字符串
- v：传出参数，解析结果
- 返回值也定义为枚举：

```
enum {
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR
};
```

### 获取类型

```c
lept_type lept_get_type(const lept_value* v);
```





# 四、 json语法子集

下面是此单元的 JSON 语法子集，使用 [RFC7159](https://tools.ietf.org/html/rfc7159) 中的 [ABNF](https://tools.ietf.org/html/rfc5234) 表示：

```
JSON-text = ws value ws
ws = *(%x20 / %x09 / %x0A / %x0D)
value = null / false / true 
null  = "null"
false = "false"
true  = "true"
```

当中 `%xhh` 表示以 16 进制表示的字符，`/` 是多选一，`*` 是零或多个，`()` 用于分组。

那么第一行的意思是，JSON 文本由 3 部分组成，首先是空白（whitespace），接着是一个值，最后是空白。

第二行告诉我们，所谓空白，是由零或多个空格符（space U+0020）、制表符（tab U+0009）、换行符（LF U+000A）、回车符（CR U+000D）所组成。

第三行是说，我们现时的值只可以是 null、false 或 true，它们分别有对应的字面值（literal）。

我们的解析器应能判断输入是否一个合法的 JSON。如果输入的 JSON 不合符这个语法，我们要产生对应的错误码，方便使用者追查问题。

在这个 JSON 语法子集下，我们定义 3 种错误码：

* 若一个 JSON 只含有空白，传回 `LEPT_PARSE_EXPECT_VALUE`。
* 若一个值之后，在空白之后还有其他字符，传回 `LEPT_PARSE_ROOT_NOT_SINGULAR`。
* 若值不是那三种字面值，传回 `LEPT_PARSE_INVALID_VALUE`。

# 五、测试驱动开发

有一种软件开发方法论，称为测试驱动开发（test-driven development, TDD），它的主要循环步骤是：

1. 加入一个测试。
2. 运行所有测试，新的测试应该会失败。
3. 编写实现代码。
4. 运行所有测试，若有测试失败回到3。
5. 重构代码。
6. 回到 1。



这里就使用测试驱动开发。首先要写好一个有足够覆盖率的单元测试。

项目文件中test.c就是需要的测试模块



