#include <iostream>
#include <stdio.h>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

//IP数据报
typedef struct{
    unsigned char hdr_len:4;        //4位头部长度
    unsigned char version:4;        //4位版本号
    unsigned char tos;              //8位服务类型
    unsigned short total_len;       //16位总长度
    unsigned short indetifier;      //16位标识符
    unsigned short frag_and_flags;  //3位标志加去13位片偏移
    unsigned char ttl;              //8位生存时间
    unsigned char protocol;         //8位上次协议号
    unsigned short checksum;        //16位校验和
    unsigned long sourceIP;         //32位源IP地址
    unsigned long destIP;           //32位目标地址
} IP_HEADER;

//ICMP报头
typedef struct{
    BYTE type;                      //8位类型字段
    BYTE code;                      //8位代码字段
    USHORT cksum;                   //16位校验和
    USHORT id;                      //16位标识符
    USHORT seq;                     //16位序列号
}ICMP_HEADER;

//报文解码结构
typedef struct{
    USHORT usSeqNo;                 //序列号
    DWORD   dwRoundTripTime;        //往返时间
    in_addr dwIPaddr;               //返回报文的IP地址
}DECODE_RESULT;

/**
* 计算网际校验和函数
* 计算检验和的方法有两个,加运算时，需循环进位
*   - 每16位取反求和再取反
*   - 先对每16求和再取反
* 以下为第二种
*/
USHORT checksum(USHORT *pBuf, int iSize){
    unsigned long cksum=0;
    while(iSize>1){
        cksum+=*pBuf++;
        iSize-=sizeof(USHORT);
    }
    if(iSize){
        cksum+=*(UCHAR*)pBuf;
    }
    cksum=(cksum>>16)+(cksum&0xffff);
    cksum+=(cksum>>16);
    return (USHORT)(~cksum);
}

/**
* 对数据进行解码
*
*/
BOOL DecodeIcmpResponse(char* pBuf, int IPacketSize, DECODE_RESULT &DecodeResult, BYTE ICMP_ECHO_REPLY, BYTE ICMP_TIMEOUT){
    //检查数据报大小的合法性
    IP_HEADER *pIpHdr=(IP_HEADER*)pBuf;
    int iIpHdrLen=pIpHdr->hdr_len*4;

    if(IPacketSize<(int)(iIpHdrLen+sizeof(ICMP_HEADER))){
        return FALSE;
    }

    //根据ICMP报文类型提取ID字段和序列号字段
    ICMP_HEADER *pIcmpHdr=(ICMP_HEADER *)(pBuf+iIpHdrLen);
    USHORT usID,usSquNO;
    if(pIcmpHdr->type==ICMP_ECHO_REPLY){    //ICMP回显应答报文
        usID=pIcmpHdr->id;  //报文ID
        usSquNO=pIcmpHdr->seq;  //报文序列号
    }else if(pIcmpHdr->type==ICMP_TIMEOUT){ //ICMP超时差错报文
        char* pInnerIpHdr=pBuf+iIpHdrLen+sizeof(ICMP_HEADER);   //载荷中的IP头
        int iInnerHdrLen=((IP_HEADER *)pInnerIpHdr)->hdr_len*4; //载荷中的IP头长
        ICMP_HEADER *pInnerIcmpHdr=(ICMP_HEADER*)(pInnerIpHdr+iInnerHdrLen);    //载荷中的ICMP头
        usID=pInnerIcmpHdr->id;                                   //报文ID
        usSquNO=pInnerIcmpHdr->seq;

    }else{
        return FALSE;
    }

    //检查ID和序列号以确定收到期待的数据报
    if(usID!=(USHORT)GetCurrentProcessId() || usSquNO!=DecodeResult.usSeqNo){
        return FALSE;
    }

    //记录IP地址并计算往返
    DecodeResult.dwIPaddr.s_addr=pIpHdr->sourceIP;
    DecodeResult.dwRoundTripTime=GetTickCount()-DecodeResult.dwRoundTripTime;

    //处理正确收到ICMP数据报
    if(pIcmpHdr->type==ICMP_ECHO_REPLY || pIcmpHdr->type==ICMP_TIMEOUT){
        //输出往返时间信息
        if(DecodeResult.dwRoundTripTime){
            cout<<"     "<<DecodeResult.dwRoundTripTime<<"ms\t";
        }else{
            cout<<"     "<<"<1ms"<<endl;
        }
    }

}
bool Ping(const char *ipAddress){

 //得到IP地址
    u_long ulDestIP=inet_addr(ipAddress);



    //cout<<"Tracing route to "<<ipAddress<<"with a maximum of 30 hops.\n\n";

    //填充目的socket地址
    sockaddr_in destSockAddr;
    ZeroMemory(&destSockAddr, sizeof(sockaddr_in));
    destSockAddr.sin_family=AF_INET;
    destSockAddr.sin_addr.s_addr=ulDestIP;

    //创建原始套接字
    /*?*/
    SOCKET sockRaw=WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, NULL, 0, WSA_FLAG_OVERLAPPED);

    //超时时间
    int iTimeout=3000;
    //接收超时
    setsockopt(sockRaw, SOL_SOCKET, SO_RCVTIMEO, (char*)&iTimeout, sizeof(iTimeout));
    //发送超时
    setsockopt(sockRaw, SOL_SOCKET, SO_SNDTIMEO, (char*)&iTimeout, sizeof(iTimeout));

    //构造ICMP回显请求消息，并以TTL递增的顺序发送报文
    //ICMP类型字段
    const BYTE ICMP_ECHO_REQUEST=8;     //请求回显
    const BYTE ICMP_ECHO_REPLY=0;       //回显请求
    const BYTE ICMP_TIMEOUT=11;         //传输超时



    //其他常量的定义
    const int DEF_ICMP_DATA_SIZE=32;    //ICMP报文默认数据字段长度
    const int MAX_ICMP_PACKET_SIZE=1024;//ICMP报文最大长度（包括报头）
    const DWORD MAX_ICMP_TIMEOUT=3000;  //航意险应答超时时间
    const int DEF_MAX_HOP=30;           //最大挑战数

    //填充ICMP报文找那个每次发送时不变的长度
    char IcmpSendBuf[sizeof(ICMP_HEADER)+DEF_ICMP_DATA_SIZE];   //发送缓冲区
    memset(IcmpSendBuf, 0, sizeof(IcmpSendBuf));                //初始化缓冲区
    char IcmpRecvBuf[MAX_ICMP_PACKET_SIZE];                     //接收缓冲区
    memset(IcmpRecvBuf, 0, sizeof(IcmpRecvBuf));                //初始化接收缓冲区

    ICMP_HEADER* pIcmpHdr=(ICMP_HEADER*)IcmpSendBuf;
    pIcmpHdr->type=ICMP_ECHO_REQUEST;                           //类型为请求回显
    pIcmpHdr->code=0;                                           //代码字段为0
    pIcmpHdr->id=(USHORT)GetCurrentProcessId();                 //ID字段为当前进程号
    memset(IcmpSendBuf+sizeof(ICMP_HEADER), 'E', DEF_ICMP_DATA_SIZE);   //数据字段

    USHORT usSeqNo=0;               //ICMP报文序列
    //int iTTL=1;                     //TTL初始为1
    BOOL bReachDestHost=FALSE;      //循环退出标志
    int iMAXHot=DEF_MAX_HOP;       //循环的最大次数
    DECODE_RESULT DecodeResult;     //传递解码函数的机构化参数
/*
    while(!bReachDestHost&&iMAXHot--){
        //设置IP报头TTL字段
        //setsockopt(sockRaw, IPPROTO_IP, IP_TTL, (char*)&iTTL, sizeof(iTTL));
        //cout<<iTTL<<flush;              //输出当前序列号

        //填充ICMP报文中每次发送变化的字段
        ((ICMP_HEADER*)IcmpSendBuf)->cksum=0;                   //校验位先置0
        ((ICMP_HEADER*)IcmpSendBuf)->seq=htons(usSeqNo++);      //填充序列号
        ((ICMP_HEADER*)IcmpSendBuf)->cksum=checksum((USHORT*)IcmpSendBuf, sizeof(ICMP_HEADER)+DEF_ICMP_DATA_SIZE);  //计算校验和

        //记录序列号和当前时间
        DecodeResult.usSeqNo=((ICMP_HEADER*)IcmpSendBuf)->seq;  //当前序列号
        DecodeResult.dwRoundTripTime=GetTickCount();            //当前时间

        //发送TCP回显请求信息
        sendto(sockRaw, IcmpSendBuf, sizeof(IcmpSendBuf), 0, (sockaddr*)&destSockAddr, sizeof(destSockAddr));
        //接收ICMP差错报文并进行解析处理
        sockaddr_in from;               //对端socket地址
        int iFromLen = sizeof(from);    //地址结构长度
        int iReadDataLen;               //接收数据长度

        //while(true){
        //接收数据
        iReadDataLen=recvfrom(sockRaw, IcmpRecvBuf, MAX_ICMP_PACKET_SIZE, 0, (sockaddr*)&from, &iFromLen);
        if(iReadDataLen!=SOCKET_ERROR)
        {
            //有数据到达
            //对数据报进行解码
            if(DecodeIcmpResponse(IcmpRecvBuf, iReadDataLen, DecodeResult, ICMP_ECHO_REPLY, ICMP_TIMEOUT))
            {
                //到达目的地，退出循环
                if(DecodeResult.dwIPaddr.s_addr==destSockAddr.sin_addr.s_addr)
                {
                    bReachDestHost=true;
                }

                //输出IP地址
                cout<<"\t"<<inet_ntoa(DecodeResult.dwIPaddr)<<endl;
                //break;
            }
        }
        else if(WSAGetLastError()==WSAETIMEDOUT)
        {
            //接收超时输出*
            cout<<"     *"<<"\t"<<"Request Time Out."<<endl;
            //break;
        }
        else
        {
            //break;
        }

        //}
        iTTL++;
    }
*/
    //填充ICMP报文中每次发送变化的字段
    ((ICMP_HEADER*)IcmpSendBuf)->cksum=0;                   //校验位先置0
    ((ICMP_HEADER*)IcmpSendBuf)->seq=htons(usSeqNo++);      //填充序列号
    ((ICMP_HEADER*)IcmpSendBuf)->cksum=checksum((USHORT*)IcmpSendBuf, sizeof(ICMP_HEADER)+DEF_ICMP_DATA_SIZE);  //计算校验和

    //记录序列号和当前时间
    DecodeResult.usSeqNo=((ICMP_HEADER*)IcmpSendBuf)->seq;  //当前序列号
    DecodeResult.dwRoundTripTime=GetTickCount();            //当前时间



    //bool hasReach=FALSE;
    for(int i=1;i<=2;i++){
        //nt iTTL=255;
        //setsockopt(sockRaw, IPPROTO_IP, IP_TTL, (char*)&iTTL, sizeof(iTTL));
        //发送TCP回显请求信息
        sendto(sockRaw, IcmpSendBuf, sizeof(IcmpSendBuf), 0, (sockaddr*)&destSockAddr, sizeof(destSockAddr));
        //接收ICMP差错报文并进行解析处理
        sockaddr_in from;               //对端socket地址
        int iFromLen = sizeof(from);    //地址结构长度
        int iReadDataLen;               //接收数据长度

        //while(true){
        //接收数据
        iReadDataLen=recvfrom(sockRaw, IcmpRecvBuf, MAX_ICMP_PACKET_SIZE, 0, (sockaddr*)&from, &iFromLen);
        if(iReadDataLen!=SOCKET_ERROR){
            //64 bytes from 220.181.38.148: icmp_seq=1 ttl=250 time=36.2 ms
            if(DecodeIcmpResponse(IcmpRecvBuf, iReadDataLen, DecodeResult, ICMP_ECHO_REPLY, ICMP_TIMEOUT)){
                 //cout<<"32 bytes from "<<inet_ntoa(DecodeResult.dwIPaddr)<<endl;
                 return true;
            }

        }else if(WSAGetLastError()==WSAETIMEDOUT){
            //cout<<"Request Time Out."<<endl;
        }else{
            //cout<<"Data Diagram Lost!"<<endl;
        }
    }
    return false;
}


int main()
{
    //初始化Windows sockets网络环境
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);


    char ipAddress[255];
    cout<<"Please Input a range, which format is [a,b], and a,b are Integer between 0 and 255:";

    int a,b;
    scanf("[%d,%d]",&a,&b);

    string networkID="192.168.37.";

    //判断输入是否合法
    while(a<0||a>255||b<0||b>255){
        cout<< "Your Input is illegal. Please Input again:";
        scanf("[%d,%d]",&a,&b);
    }



    for(int i=a;i<=b;i++){
        string ip=networkID+to_string(i);
        if(Ping(ip.c_str())){
            cout<<ip<<" is OnLine..."<<endl;
        }else{
            cout<<ip<<" is OffLine..."<<endl;
        }
    }


    return 0;
}
