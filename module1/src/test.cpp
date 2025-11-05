#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <iostream>
#include <string>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

// 函数声明
void sendHead(const char* filename, const char* extname, SOCKET s);
void sendData(const char* filename, SOCKET s);

int main() {
    WSADATA wsaData;
    
    // 初始化Winsock
    int nRc = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (nRc != 0) {
        printf("Winsock初始化失败! 错误代码: %d\n", nRc);
        return 1;
    }
    
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        printf("Winsock版本不正确!\n");
        WSACleanup();
        return 1;
    }
    
    printf("Winsock 2.2 初始化成功!\n");

    // 创建服务器socket
    SOCKET srvSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (srvSocket == INVALID_SOCKET) {
        printf("Socket创建失败! 错误代码: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    printf("Socket创建成功!\n");
    printf("----------------------------------------------\n");

    // 获取服务器配置
    int srvPort;
    char srvIP[20];
    char fileAddr[50];
    
    printf("请输入监听端口号：");
    scanf("%d", &srvPort);
    printf("请输入监听IP地址：");
    scanf("%19s", srvIP);
    printf("请输入主目录：");
    scanf("%49s", fileAddr);
    printf("----------------------------------------------\n");

    // 设置服务器地址
    sockaddr_in srvAddr;
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(srvPort);
    srvAddr.sin_addr.s_addr = inet_addr(srvIP);

    // 绑定socket
    nRc = bind(srvSocket, (sockaddr*)&srvAddr, sizeof(srvAddr));
    if (nRc == SOCKET_ERROR) {
        printf("Socket绑定失败! 错误代码: %d\n", WSAGetLastError());
        closesocket(srvSocket);
        WSACleanup();
        return 1;
    }
    printf("Socket绑定成功!\n");

    // 开始监听
    nRc = listen(srvSocket, 5);
    if (nRc == SOCKET_ERROR) {
        printf("设置监听失败! 错误代码: %d\n", WSAGetLastError());
        closesocket(srvSocket);
        WSACleanup();
        return 1;
    }
    printf("服务器开始监听 %s:%d\n", srvIP, srvPort);
    printf("主目录: %s\n", fileAddr);
    printf("----------------------------------------------\n");

    // 主服务循环
    while (true) {
        sockaddr_in cltAddr;
        int addrLen = sizeof(cltAddr);
        
        // 接受客户端连接
        SOCKET cltSocket = accept(srvSocket, (sockaddr*)&cltAddr, &addrLen);
        if (cltSocket == INVALID_SOCKET) {
            printf("接受客户端连接失败! 错误代码: %d\n", WSAGetLastError());
            continue;
        }
        
        printf("客户端连接成功! IP: %s, 端口: %d\n", 
               inet_ntoa(cltAddr.sin_addr), ntohs(cltAddr.sin_port));

        // 接收HTTP请求
        char rcvdata[2048] = "";
        nRc = recv(cltSocket, rcvdata, sizeof(rcvdata) - 1, 0);
        if (nRc == SOCKET_ERROR) {
            printf("接收数据失败! 错误代码: %d\n", WSAGetLastError());
            closesocket(cltSocket);
            continue;
        }
        
        rcvdata[nRc] = '\0'; // 确保字符串结束
        printf("收到 %d 字节数据:\n%.*s\n", nRc, nRc, rcvdata);

        // 解析HTTP请求
        char rqtname[100] = "";
        char extname[20] = "";
        
        // 简单的请求解析（实际项目应该使用更健壮的解析器）
        sscanf(rcvdata, "GET %s", rqtname);
        
        // 提取文件扩展名
        const char* dot = strrchr(rqtname, '.');
        if (dot != NULL) {
            strcpy(extname, dot + 1);
        }

        printf("请求文件: %s, 扩展名: %s\n", rqtname, extname);

        // 构建完整文件路径
        char filename[200];
        snprintf(filename, sizeof(filename), "%s%s", fileAddr, rqtname);
        printf("完整路径: %s\n", filename);
        printf("----------------------------------------------\n");

        // 发送HTTP响应
        sendHead(filename, extname, cltSocket);
        sendData(filename, cltSocket);
        printf("----------------------------------------------\n\n");

        closesocket(cltSocket);
    }

    // 清理资源（实际上循环不会退出）
    closesocket(srvSocket);
    WSACleanup();
    return 0;
}

void sendHead(const char* filename, const char* extname, SOCKET s) {
    // 设置Content-Type
    const char* content_type = "text/plain"; // 默认类型
    
    if (strcmp(extname, "html") == 0) content_type = "text/html";
    else if (strcmp(extname, "htm") == 0) content_type = "text/html";
    else if (strcmp(extname, "css") == 0) content_type = "text/css";
    else if (strcmp(extname, "js") == 0) content_type = "application/javascript";
    else if (strcmp(extname, "gif") == 0) content_type = "image/gif";
    else if (strcmp(extname, "jpg") == 0) content_type = "image/jpeg";
    else if (strcmp(extname, "jpeg") == 0) content_type = "image/jpeg";
    else if (strcmp(extname, "png") == 0) content_type = "image/png";
    else if (strcmp(extname, "txt") == 0) content_type = "text/plain";

    // 检查是否为禁止访问的文件
    if (strstr(filename, "private.png") != NULL) {
        const char* response = 
            "HTTP/1.1 403 FORBIDDEN\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 13\r\n"
            "\r\n"
            "Access Denied";
        send(s, response, strlen(response), 0);
        printf("403 禁止访问: %s\n", filename);
        return;
    }

    // 检查文件是否存在
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) {
        const char* response = 
            "HTTP/1.1 404 NOT FOUND\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 9\r\n"
            "\r\n"
            "Not Found";
        send(s, response, strlen(response), 0);
        printf("404 文件未找到: %s\n", filename);
        return;
    }
    fclose(fp);

    // 发送200 OK响应头
    char header[512];
    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "\r\n",
             content_type);
    
    if (send(s, header, strlen(header), 0) == SOCKET_ERROR) {
        printf("发送响应头失败! 错误代码: %d\n", WSAGetLastError());
    } else {
        printf("200 文件查找成功: %s\n", filename);
    }
}

void sendData(const char* filename, SOCKET s) {
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) {
        return;
    }

    // 获取文件大小
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // 读取并发送文件内容
    char* buffer = (char*)malloc(file_size);
    if (buffer == NULL) {
        fclose(fp);
        return;
    }

    fread(buffer, 1, file_size, fp);
    int bytes_sent = send(s, buffer, file_size, 0);
    
    if (bytes_sent == SOCKET_ERROR) {
        printf("发送文件数据失败! 错误代码: %d\n", WSAGetLastError());
    } else {
        printf("成功发送 %d/%ld 字节文件数据\n", bytes_sent, file_size);
    }

    free(buffer);
    fclose(fp);
}