# 1. 代码重构

简单的对lept_parse_***函数做了一下重构：

```c
static int lept_parse_key_type(lept_context *c, lept_value*v, const char* key) {
    EXPECT(c, key[0]);
    if (strlen(c->json) < strlen(key))
        return LEPT_PARSE_INVALID_VALUE;
    for (int i = 0; i < strlen(key);++i) {
        if (key[i] != c->json[0])
            return LEPT_PARSE_INVALID_VALUE;
        c->json++;
    }
    switch (key[0]) {
    case 'n': v->type = LEPT_NULL; break;
    case 't':v->type = LEPT_TRUE; break;
    case 'f':v->type = LEPT_FALSE; break;
    }
    return LEPT_PARSE_OK;
}


static int lept_parse_true(lept_context* c, lept_value* v) {
    return lept_parse_key_type(c, v, "true");
}

static int lept_parse_false(lept_context* c, lept_value* v) {
    return lept_parse_key_type(c, v, "false");
}

static int lept_parse_null(lept_context* c, lept_value* v) {
    return lept_parse_key_type(c, v, "null");
}
```





# 2. 解析数字

## 2.1 数字的json表示

json中数字语法图示如下：

![number](images/number.png)

```
number = [ "-" ] int [ frac ] [ exp ]
int = "0" / digit1-9 *digit
frac = "." 1*digit
exp = ("e" / "E") ["-" / "+"] 1*digits
```

数字包括： 符号、整数部分、小数部分、指数部分

数字部分可以是0，也可以是正整数(首字符不为0)

小数部分可选，以`.`开头，后跟着1或多个数字

指数部分以`E`或者`e`开头，可以以正负号，之后是一或多个数字。

## 2.2 数字的程序内部表示

数字与`true`、`false`、`null`不同，不仅需要保存类型，还需要保存值。

简单起见，直接把数字保存为double类型。



```c
typedef struct {
    double n;
    lept_type type;
}lept_value;
```

获取数字值的函数如下：

```c
double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
```

## 2.3 转换函数

将json中的数字转换为程序内的数字难度其实非常大，主要问题是json中数字为10进制，而编程语言中一般采用2进制，两者有时不存在精确的对应关系。

我写了一篇笔记讨论这个问题

[笔记：计算机中小数的存储](double float.md)

严格来讲，需要自己定义特殊的数字形式才能将json中的数字精确保存，但该项目我们简单起见，使用double来近似存储小数。



这里使用c内置的函数`strtod`来转换。

- strtod介绍：

```
#include<stdlib.h>
double strtod(cosnt char *nptr, char **endptr);

功能：将数字从字符串的形式转换为double的形式。
参数：
	nptr: 字符串地址。该数字的开头可以有空格，之后可以是一个'+'或者'-'号，再之后应当是以下3者之一：
		- 一个十进制数字，该十进制数字可以有小数点，也允许有指数。指数以E或者e结尾
		- 一个十六进制数字。
		- 无穷大，以INF或者INFINITY表示（不区分大小写）
		- NAN 表示不是数字。
	nptr: 传出参数。转换完成后，endptr会指向数字末尾的后一个字符。假如没有完成转换，endptr指向nptr。如果不需要使用endptr可以将其设为NULL
```

可以看出，strtod可以直接用来解析json中的数字。

但是需要考虑以下情况：

- 当strtod出错时，需要正确地捕捉错误。这个可以有时可以由函数的返回值来判断。
- 即使strtod正确运行，输入数字也有可能不符合规定。比如strtod能解析十六进制数字，但这不符合json标准，有时候这不符合程序功能要求。

在使用strtod之前，需要提前做好必要的合法性判断。

```c
static int lept_parse_number(lept_context* c, lept_value* v) {
    v->type = LEPT_INVALID;
    char* end;
    int itr=0;
    //符号位
    if (c->json[0] == '-') itr++;

    if (!IS_DIGIT(c->json[itr])) {//首字符必须为数字
        return LEPT_PARSE_INVALID_VALUE;
    }
    //特判0
    if (c->json[itr] == '0' && c->json[itr+1]!='.'){//排除以0为开头的正小数,剩下的肯定只能是数字0

        if (c->json[itr+1] != ' ' && c->json[itr + 1] != '\n' && c->json[itr + 1] != '\t' && c->json[itr + 1] != '\r' && c->json[itr + 1] != 0) {//数字0之后只允许有空白字符
            return LEPT_PARSE_INVALID_VALUE;
        }
    c->json += (itr + 1);
    v->n = 0;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
    }
    if (! IS_DIGIT1_9(c->json[itr]) && c->json[itr+1]!='.') {//除非是以0开头的小数，否则首字符只能是1-9
        return LEPT_PARSE_INVALID_VALUE;
    }

    int count_dot = 0;
    int count_e=0;
    while (IS_DIGIT(c->json[itr])||c->json[itr]=='.'||c->json[itr]=='E'||c->json[itr]=='e') {
        if (c->json[itr] == '.') {
            if (count_dot || count_e||!IS_DIGIT(c->json[itr+1])) {//小数点只能出现一次，E/e必须在小数点之前，小数点之后必然是数字
                return LEPT_PARSE_INVALID_VALUE;
            }
            ++count_dot;
            ++itr;
            continue;
        }
        if (c->json[itr] == 'e' || c->json[itr] == 'E') {//E/e只能出现一次，E/e之后是一个可以有符号的数字
            if (c->json[itr + 1] == '-' || c->json[itr + 1] == '+') {
                ++itr;
            }
            if (count_e||(!IS_DIGIT(c->json[itr+1]))){
                return LEPT_PARSE_INVALID_VALUE;
            }
            ++count_e;
            ++itr;
            continue;
        }
        ++itr;
    }

    v->n = strtod(c->json, &end);
    if ((v->n == HUGE_VAL || v->n == -HUGE_VAL)&&errno==ERANGE) {//溢出错误处理
        return LEPT_PARSE_NUMBER_TOO_BIG;
    }
    c->json = end;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}
```



相比原作者的方法相对复杂一些。原作者的方法利用了strtod的一些返回值，而我基本没有使用。



