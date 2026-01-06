#include <iostream>
#include <unistd.h>  // getpid(), getppid()
using namespace std;

int main() {
    pid_t pid = getpid();     // 获取当前进程PID
    pid_t ppid = getppid();   // 获取父进程PID
    cout << "当前进程PID: " << pid << endl;
    cout << "父进程PPID: " << ppid << endl;
    return 0;
}
