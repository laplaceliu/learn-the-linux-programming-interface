#include <iostream>
#include <string>
#include <dirent.h>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <cerrno>
using namespace std;

const int MAX_LINK_LEN = 1024;

// 读取符号链接目标（用于/proc/PID/fd/*）
string readLink(const string& path) {
    char buf[MAX_LINK_LEN];
    ssize_t len = readlink(path.c_str(), buf, MAX_LINK_LEN - 1);
    if (len == -1) {
        return "读取失败：" + string(strerror(errno));
    }
    buf[len] = '\0';
    return buf;
}

// 列出进程打开的文件描述符
void listProcessFds(pid_t pid) {
    string fdDirPath = "/proc/" + to_string(pid) + "/fd";
    DIR* dir = opendir(fdDirPath.c_str());
    if (!dir) {
        cerr << "打开" << fdDirPath << "失败：" << strerror(errno) << endl;
        return;
    }

    cout << "=== 进程" << pid << "打开的文件描述符 ===" << endl;
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        string fdName = entry->d_name;
        if (fdName == "." || fdName == "..") continue;
        string fdPath = fdDirPath + "/" + fdName;
        string target = readLink(fdPath);
        cout << "fd=" << fdName << " -> " << target << endl;
    }

    closedir(dir);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "用法：" << argv[0] << " <PID>" << endl;
        return 1;
    }

    pid_t pid = stoi(argv[1]);
    listProcessFds(pid);
    return 0;
}
