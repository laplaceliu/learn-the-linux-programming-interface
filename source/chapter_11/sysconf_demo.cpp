#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <cstring>
using namespace std;

// 打印sysconf()获取的限制
void printSysConf(const char* desc, int name) {
    errno = 0;
    long lim = sysconf(name);
    cout << desc << ": ";
    if (lim == -1) {
        if (errno == 0) {
            cout << "（不确定）" << endl;
        } else {
            cout << "获取失败：" << strerror(errno) << endl;
        }
    } else {
        cout << lim << endl;
    }
}

int main() {
    cout << "=== 系统级限制 ===" << endl;
    printSysConf("ARG_MAX（参数+环境变量最大字节数）", _SC_ARG_MAX);
    printSysConf("OPEN_MAX（最大打开文件描述符数）", _SC_OPEN_MAX);
    printSysConf("LOGIN_NAME_MAX（登录名最大长度）", _SC_LOGIN_NAME_MAX);
    printSysConf("NGROUPS_MAX（最大补充组数量）", _SC_NGROUPS_MAX);
    printSysConf("PAGESIZE（虚拟内存页大小）", _SC_PAGESIZE);
    printSysConf("RTSIG_MAX（最大实时信号数）", _SC_RTSIG_MAX);

    return 0;
}
