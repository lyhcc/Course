#include <iostream>
#include <stdio.h>
#include <Winsock2.h>
#include <string.h>


using namespace std;

int main()
{


    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    /*1. 加载套接字库，创建套接字*/
    wVersionRequested=MAKEWORD(1,1);

    err=WSAStartup(wVersionRequested, &wsaData);
    if(err != 0){

        return 0;
    }

    /*检查版本*/
    if(LOBYTE(wsaData.wVersion)!=1 || HIBYTE(wsaData.wVersion) !=1){
        WSACleanup();
        return 0;
    }

    /**
    * 第一个参数 使用IP地址族
    * 第二个参数 流式套接字（SOCK_STREAM），数据报套接字（SOCK_DGRAM），
    * 原始套接字（SOCK_RAW）
    */
    SOCKET sockSrv=socket(AF_INET,SOCK_STREAM,0);

    SOCKADDR_IN addrSrv;
    /*?*/
    addrSrv.sin_addr.S_un.S_addr=htonl(INADDR_ANY);
    addrSrv.sin_family=AF_INET;
    addrSrv.sin_port=htons(6210);

    /*绑定*/
    bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
    /*监听*/
    listen(sockSrv, 5);

    SOCKADDR_IN addrClient;
    int len=sizeof(SOCKADDR);


    printf("Welcome %s to here!\n", inet_ntoa(addrClient.sin_addr));
    SOCKET sockConn=accept(sockSrv, (SOCKADDR*)&addrClient, &len);;
    while(true){


        char recvBuf[200]={0};
        while(true){
            recv(sockConn, recvBuf, 200, 0);
            if(strlen(recvBuf)>0)break;
        }
        printf("Kettle：%s\n", recvBuf);



        char sendBuf[200];
        printf("Allen: ");
        gets(sendBuf);

        send(sockConn, sendBuf, strlen(sendBuf)+1,0);




    }
    closesocket(sockConn);
}
