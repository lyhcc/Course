#include <iostream>
#include <stdio.h>
#include <Winsock2.h>
#include <string.h>

using namespace std;

int main()
{
    char name[100]="Allen";
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested=MAKEWORD(1, 1);


    err=WSAStartup(wVersionRequested, &wsaData);
    if(err!=0){
        return 0;
    }
    if(LOBYTE(wsaData.wVersion)!=1 || HIBYTE(wsaData.wVersion)!=1){
        WSACleanup();
        printf("ASDDS");
        return 0;
    }

    SOCKET sockClient=socket(AF_INET, SOCK_STREAM, 0);

    SOCKADDR_IN addrSrv;
    addrSrv.sin_addr.S_un.S_addr=inet_addr("127.0.0.1");
    addrSrv.sin_family=AF_INET;
    addrSrv.sin_port=htons(6210);

    connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
    //while(true){
    //char sendBuf[100]="I don't  like C/C++, and I can't achieve the socket task.";
    while(true){
        char sendBuf[200];
        printf("Kettle: ");
        gets(sendBuf);
        send(sockClient, sendBuf, strlen(sendBuf)+1, 0);

        char recvBuf[200]="\0";

        while(true){
            recv(sockClient, recvBuf, 200, 0);
            if(strlen(recvBuf)>0)break;
        }
        printf("%s: %s\n", name, recvBuf);

    }

    //}

    closesocket(sockClient);
    WSACleanup();
    return 0;
}
