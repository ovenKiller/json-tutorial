# realloc





```
void *realloc(void *ptr, size_t size);
```

功能： 改变地址在ptr的动态分配内存的大小为size。返回值是改变后的内存地址，该地址未必是ptr。假如ptr=NULL,则函数作用相当于malloc，假如size=0,函数作用相当于free。调整后地址未必相等，但是内存区的内容会被保存下来。

- 参数
  - ptr： 原始内存地址
  - size: 想要调整后的内存大小
- 返回值：新内存的地址



注意一个问题：realloc的功能太多就可能导致错误。

[(805条消息) realloc 可能导致的内存泄露_wangzhang001的博客-CSDN博客_realloc 内存泄露](https://blog.csdn.net/wangzhang001/article/details/23257397)