#include <cstdio>
#include <string>
#include <iostream>

#include <winsock2.h>
#include <ws2tcpip.h>

using std::string;

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif

void sendHead(const char *filename, const char *extname, SOCKET s);
void sendData(const char *filename, SOCKET s);

int main() {
    WSADATA wsaData;

    int nRc = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (nRc != 0) {
        // initialization failed
        printf("[ERROR] Winsock initialization failed! Error code: %d\n", nRc);
        return 1;
    }
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        // version check, should be Winsock 2.2
        printf("[ERROR] Winsock version is incorrect!\n");
        WSACleanup();
        return 1;
    }
    printf("Winsock 2.2 initialized successfully!\n");

    SOCKET srvSocket;  // server listen socket

    // create server socket, use TCP protocol
    srvSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (srvSocket == INVALID_SOCKET) {
        printf("[ERROR] Socket creation failed!\nError code: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    printf("Socket created successfully!\n");
    printf("----------------------------------------------\n");

    int srvPort;
    char srvIP[20];
    char fileAddr[50];

    printf("Enter listening port: ");
    scanf("%d", &srvPort);
    printf("Enter listening IP address: ");
    scanf("%19s", srvIP);
    printf("Enter root directory: ");
    scanf("%49s", fileAddr);
    printf("----------------------------------------------\n");

    // set server address structure (zero it first)
    sockaddr_in srvAddr;
    memset(&srvAddr, 0, sizeof(srvAddr));
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(srvPort);
    srvAddr.sin_addr.s_addr = inet_addr(srvIP);

    nRc = bind(srvSocket, (sockaddr*)&srvAddr, sizeof(srvAddr));
    if (nRc == SOCKET_ERROR) {
        printf("[ERROR] Socket binding failed! Error code: %d\n", WSAGetLastError());
        closesocket(srvSocket);
        WSACleanup();
        return 1;
    }
    printf("Socket bound successfully!\n");
    // Start listening
    nRc = listen(srvSocket, 5);
    if (nRc == SOCKET_ERROR) {
        printf("Failed to set listening! Error code: %d\n", WSAGetLastError());
        closesocket(srvSocket);
        WSACleanup();
        return 1;
    }
    printf("Server is listening on %s:%d\n", srvIP, srvPort);
    printf("Root directory: %s\n", fileAddr);
    printf("----------------------------------------------\n");

    while (true) {
        sockaddr_in cltAddr;
        int addrLen = sizeof(cltAddr);

        // accept client connection
        SOCKET cltSocket = accept(srvSocket, (sockaddr*)&cltAddr, &addrLen);
        if (cltSocket == INVALID_SOCKET) {
            printf("[ERROR] Accepting client connection failed! Error code: %d\n", WSAGetLastError());
            continue;
        }
        printf("Client connected! IP: %s, Port: %d\n",
               inet_ntoa(cltAddr.sin_addr), ntohs(cltAddr.sin_port));
        
        // receive HTTP request
        char rcvdata[2048] = {};
        nRc = recv(cltSocket, rcvdata, sizeof(rcvdata) - 1, 0);
        if (nRc == SOCKET_ERROR) {
            printf("[ERROR] Receiving data failed! Error code: %d\n", WSAGetLastError());
            closesocket(cltSocket);
            continue;
        }
        rcvdata[nRc] = '\0'; // ensure string termination
        printf("Received %d bytes of data:\n%.*s\n", nRc, nRc, rcvdata);

        char reqname[100] = {};
        char extname[20] = {};
        sscanf(rcvdata, "GET %s", reqname);

        const char *dotptr = strrchr(reqname, '.');
        if (dotptr != NULL)
            strcpy(extname, dotptr + 1);
        printf("Requested file: %s, Extension: %s\n", reqname, extname);

        char filename[200];
        snprintf(filename, sizeof(filename), "%s%s", fileAddr, reqname);
        printf("Full path: %s\n", filename);
        printf("----------------------------------------------\n");

        sendHead(filename, extname, cltSocket);
        sendData(filename, cltSocket);
        closesocket(cltSocket);
        printf("Connection closed.\n");
    }

    closesocket(srvSocket);
    WSACleanup();
    return 0;
}

void sendHead(const char *filename, const char *extname, SOCKET s) {
    const char *content_type = "text/plain";

    if (strcmp(extname, "html") == 0) content_type = "text/html";
    else if (strcmp(extname, "htm") == 0) content_type = "text/html";
    else if (strcmp(extname, "css") == 0) content_type = "text/css";
    else if (strcmp(extname, "js") == 0) content_type = "application/javascript";
    else if (strcmp(extname, "gif") == 0) content_type = "image/gif";
    else if (strcmp(extname, "jpg") == 0) content_type = "image/jpeg";
    else if (strcmp(extname, "jpeg") == 0) content_type = "image/jpeg";
    else if (strcmp(extname, "png") == 0) content_type = "image/png";
    else if (strcmp(extname, "txt") == 0) content_type = "text/plain";

    if (strstr(filename, "anon.jpg") != NULL) {
        // invalid file access
        const char *resp =
            "HTTP/1.1 403 FORBIDDEN\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 13\r\n"
            "\r\n"
            "Access Denied";
        send(s, resp, strlen(resp), 0);
        printf("403 Forbidden: %s\n", filename);
        return;
    }

    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        // file not found
        const char *resp =
            "HTTP/1.1 404 NOT FOUND\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 9\r\n"
            "\r\n"
            "Not Found";
        send(s, resp, strlen(resp), 0);
        printf("404 Not Found: %s\n", filename);
        return;
    }
    fclose(fp);

    char header[512];
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "\r\n",
        content_type);
    int nRc = send(s, header, strlen(header), 0);
    if (nRc == SOCKET_ERROR)
        printf("[ERROR] Sending header failed! Error code: %d\n", WSAGetLastError());
    else
        printf("Sent %d bytes of header data.\n", nRc);
}

void sendData(const char *filename, SOCKET s) {
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        // 404 not found already handled in sendHead
        return ;
    }

    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *buffer = new char[fileSize];
    if (buffer == NULL) {
        fclose(fp);
        return ;
    }

    fread(buffer, 1, fileSize, fp);
    int nRc = send(s, buffer, fileSize, 0);
    if (nRc == SOCKET_ERROR)
        printf("[ERROR] Sending data failed!"
               "Error code: %d\n", WSAGetLastError());
    else
        printf("Sent %d/%ld bytes of file data.\n", nRc, fileSize);
    delete[] buffer;
    fclose(fp);
}