#include <iostream>
#include <cstdlib>  // malloc()
using namespace std;

// 初始化数据段：显式初始化的全局变量
int g_init = 10;
// 未初始化数据段（BSS）：未显式初始化的全局变量
int g_uninit;
// 初始化数据段：显式初始化的静态全局变量
static int g_static_init = 20;
// 未初始化数据段（BSS）：未显式初始化的静态全局变量
static int g_static_uninit;

int main() {
    // 初始化数据段：显式初始化的静态局部变量
    static int l_static_init = 30;
    // 未初始化数据段（BSS）：未显式初始化的静态局部变量
    static int l_static_uninit;
    // 栈：局部变量（自动变量）
    int l_stack = 40;
    // 堆：动态分配内存
    int* l_heap = (int*)malloc(sizeof(int));
    *l_heap = 50;

    cout << "各段变量地址与值：" << endl;
    cout << "初始化数据段（全局）：g_init = " << g_init << ", 地址 = " << &g_init << endl;
    cout << "未初始化数据段（全局）：g_uninit = " << g_uninit << ", 地址 = " << &g_uninit << endl;
    cout << "初始化数据段（静态全局）：g_static_init = " << g_static_init << ", 地址 = " << &g_static_init << endl;
    cout << "未初始化数据段（静态全局）：g_static_uninit = " << g_static_uninit << ", 地址 = " << &g_static_uninit << endl;
    cout << "初始化数据段（静态局部）：l_static_init = " << l_static_init << ", 地址 = " << &l_static_init << endl;
    cout << "未初始化数据段（静态局部）：l_static_uninit = " << l_static_uninit << ", 地址 = " << &l_static_uninit << endl;
    cout << "栈（局部变量）：l_stack = " << l_stack << ", 地址 = " << &l_stack << endl;
    cout << "堆（动态分配）：*l_heap = " << *l_heap << ", 地址 = " << l_heap << endl;

    free(l_heap);  // 释放堆内存
    return 0;
}
