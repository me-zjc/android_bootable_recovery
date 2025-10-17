#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <string>

#define SCREENSHOT_SOCKET_PATH "/tmp/twrp_screencap.sock"

int main(int argc, char **argv)
{
    std::string cmd = "screencap";
    bool output_png = false;
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            output_png = true;
            // 如果-p后面有参数，则将其作为路径传递
            if (i + 1 < argc && argv[i+1][0] != '-') {
                cmd = "screencap:" + std::string(argv[i+1]);
                i++; // 跳过下一个参数
            } else {
                // 只有-p参数，没有指定路径
                cmd = "screencap:-p";
            }
        }
    }
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return 1;
    }
    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SCREENSHOT_SOCKET_PATH);
    if (connect(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(fd);
        return 1;
    }
    write(fd, cmd.c_str(), cmd.length());
    // 如果是输出PNG数据模式，直接将收到的数据写到stdout
    if (output_png) {
        char buffer[1024];
        ssize_t n;
        while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
            write(STDOUT_FILENO, buffer, n);
        }
    } else {
        // 否则显示服务器回复消息
        char buffer[128] = {0};
        ssize_t n = read(fd, buffer, sizeof(buffer)-1);
        if (n > 0) {
            buffer[n] = '\0';
            std::cout << "Server reply: " << buffer << std::endl;
        }
    }
    close(fd);
    return 0;
}