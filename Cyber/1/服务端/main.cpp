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
    /*1. �����׽��ֿ⣬�����׽���*/
    wVersionRequested=MAKEWORD(1,1);

    err=WSAStartup(wVersionRequested, &wsaData);
    if(err != 0){

        return 0;
    }

    /*���汾*/
    if(LOBYTE(wsaData.wVersion)!=1 || HIBYTE(wsaData.wVersion) !=1){
        WSACleanup();
        return 0;
    }

    /**
    * ��һ������ ʹ��IP��ַ��
    * �ڶ������� ��ʽ�׽��֣�SOCK_STREAM�������ݱ��׽��֣�SOCK_DGRAM����
    * ԭʼ�׽��֣�SOCK_RAW��
    */
    SOCKET sockSrv=socket(AF_INET,SOCK_STREAM,0);

    SOCKADDR_IN addrSrv;
    /*?*/
    addrSrv.sin_addr.S_un.S_addr=htonl(INADDR_ANY);
    addrSrv.sin_family=AF_INET;
    addrSrv.sin_port=htons(6210);

    /*��*/
    bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
    /*����*/
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
        printf("Kettle��%s\n", recvBuf);



        char sendBuf[200];
        printf("Allen: ");
        gets(sendBuf);

        send(sockConn, sendBuf, strlen(sendBuf)+1,0);




    }
    closesocket(sockConn);
}
