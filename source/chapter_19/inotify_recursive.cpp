#define _XOPEN_SOURCE 500
#include <iostream>
#include <sys/inotify.h>
#include <ftw.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <limits.h>
#include <unordered_map>
#include <vector>

using namespace std;

// 全局变量（简化nftw回调访问）
int g_inotify_fd;
unordered_map<int, string> g_wd_to_path; // wd→目录路径
unordered_map<string, int> g_path_to_wd; // 路径→wd（避免重复添加）

// 事件掩码转字符串（同示例1，省略重复代码，实际使用需包含）
string eventMaskToString(uint32_t mask);

// nftw回调函数：遍历目录时添加监控
static int addWatchForDir(const char* pathname, const struct stat* st, int typeflag, struct FTW* ftwbuf) {
    if (typeflag == FTW_D || typeflag == FTW_DP) { // 仅处理目录
        if (g_path_to_wd.count(pathname)) {
            return 0; // 已添加监控，跳过
        }
        // 监控目录的创建、删除、重命名、子文件变更等事件
        uint32_t mask = IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO | IN_DELETE_SELF | IN_MOVE_SELF;
        int wd = inotify_add_watch(g_inotify_fd, pathname, mask);
        if (wd == -1) {
            cerr << "添加目录监控失败（" << pathname << "）：" << strerror(errno) << endl;
            return 0;
        }
        g_wd_to_path[wd] = pathname;
        g_path_to_wd[pathname] = wd;
        cout << "已添加目录监控：" << pathname << "（wd=" << wd << "）" << endl;
    }
    return 0;
}

// 递归遍历目录树，添加监控
bool recursiveWatchDir(const char* root_dir) {
    // nftw参数：目录路径、回调、最大文件描述符数、遍历标志（FTW_PHYS不跟随符号链接）
    int ret = nftw(root_dir, addWatchForDir, 10, FTW_PHYS);
    if (ret == -1) {
        cerr << "遍历目录树失败：" << strerror(errno) << endl;
        return false;
    }
    return true;
}

// 处理新创建的子目录，添加监控
void handleNewDir(const string& parent_dir, const string& dir_name) {
    string new_dir = parent_dir + "/" + dir_name;
    if (g_path_to_wd.count(new_dir)) {
        return;
    }
    uint32_t mask = IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO | IN_DELETE_SELF | IN_MOVE_SELF;
    int wd = inotify_add_watch(g_inotify_fd, new_dir.c_str(), mask);
    if (wd != -1) {
        g_wd_to_path[wd] = new_dir;
        g_path_to_wd[new_dir] = wd;
        cout << "动态添加子目录监控：" << new_dir << "（wd=" << wd << "）" << endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "用法：" << argv[0] << " <根目录路径>" << endl;
        return EXIT_FAILURE;
    }
    const char* root_dir = argv[1];

    // 1. 创建inotify实例
    g_inotify_fd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    if (g_inotify_fd == -1) {
        cerr << "创建inotify实例失败：" << strerror(errno) << endl;
        return EXIT_FAILURE;
    }

    // 2. 递归遍历目录树，添加初始监控
    cout << "开始递归遍历目录树：" << root_dir << endl;
    if (!recursiveWatchDir(root_dir)) {
        close(g_inotify_fd);
        return EXIT_FAILURE;
    }

    // 3. 循环读取事件
    const size_t BUF_LEN = 20 * (sizeof(struct inotify_event) + NAME_MAX + 1);
    vector<char> buf(BUF_LEN);
    cout << "\n开始监控目录树事件（按Ctrl+C退出）..." << endl;

    while (true) {
        ssize_t bytes_read = read(g_inotify_fd, buf.data(), buf.size());
        if (bytes_read == -1) {
            if (errno == EAGAIN) {
                usleep(100000);
                continue;
            } else {
                cerr << "读取事件失败：" << strerror(errno) << endl;
                break;
            }
        }

        // 解析事件
        for (size_t pos = 0; pos < bytes_read; ) {
            struct inotify_event* event = reinterpret_cast<struct inotify_event*>(buf.data() + pos);
            auto it = g_wd_to_path.find(event->wd);
            if (it == g_wd_to_path.end()) {
                pos += sizeof(struct inotify_event) + event->len;
                continue;
            }
            string dir_path = it->second;

            // 输出事件信息
            cout << "\n=== 目录事件 ===" << endl;
            cout << "目录：" << dir_path << "（wd=" << event->wd << "）" << endl;
            cout << "事件类型：" << eventMaskToString(event->mask) << endl;
            if (event->len > 0) {
                cout << "关联文件名：" << event->name << endl;
            }

            // 处理子目录创建：动态添加监控
            if ((event->mask & IN_CREATE) && (event->mask & IN_ISDIR)) {
                handleNewDir(dir_path, event->name);
            }

            // 处理目录删除/移除监控
            if (event->mask & IN_DELETE_SELF || event->mask & IN_IGNORED) {
                g_path_to_wd.erase(dir_path);
                g_wd_to_path.erase(event->wd);
                cout << "目录监控已移除：" << dir_path << endl;
            }

            pos += sizeof(struct inotify_event) + event->len;
        }
    }

    // 4. 清理资源
    close(g_inotify_fd);
    cout << "\n监控结束" << endl;
    return EXIT_SUCCESS;
}

// 事件掩码转字符串实现（需添加到代码中）
string eventMaskToString(uint32_t mask) {
    vector<string> events;
    if (mask & IN_CREATE) events.push_back("IN_CREATE");
    if (mask & IN_DELETE) events.push_back("IN_DELETE");
    if (mask & IN_MOVED_FROM) events.push_back("IN_MOVED_FROM");
    if (mask & IN_MOVED_TO) events.push_back("IN_MOVED_TO");
    if (mask & IN_DELETE_SELF) events.push_back("IN_DELETE_SELF");
    if (mask & IN_MOVE_SELF) events.push_back("IN_MOVE_SELF");
    if (mask & IN_ISDIR) events.push_back("IN_ISDIR");
    if (mask & IN_IGNORED) events.push_back("IN_IGNORED");

    string result;
    for (size_t i = 0; i < events.size(); ++i) {
        if (i > 0) result += " | ";
        result += events[i];
    }
    return result.empty() ? "UNKNOWN" : result;
}
