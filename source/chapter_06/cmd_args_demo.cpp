#include <iostream>
#include <cstdlib>  // exit()
using namespace std;

int main(int argc, char* argv[]) {
    cout << "参数个数 argc = " << argc << endl;
    cout << "命令行参数列表：" << endl;
    // 方式1：通过argc遍历
    for (int i = 0; i < argc; ++i) {
        cout << "argv[" << i << "] = " << argv[i] << endl;
    }

    cout << "\n方式2：通过NULL终止遍历：" << endl;
    // 方式2：通过argv末尾的NULL遍历
    char** p = argv;
    while (*p != NULL) {
        cout << *p << endl;
        ++p;
    }

    return EXIT_SUCCESS;
}
