#include <iostream>
#include <stdio.h>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

//IP���ݱ�
typedef struct{
    unsigned char hdr_len:4;        //4λͷ������
    unsigned char version:4;        //4λ�汾��
    unsigned char tos;              //8λ��������
    unsigned short total_len;       //16λ�ܳ���
    unsigned short indetifier;      //16λ��ʶ��
    unsigned short frag_and_flags;  //3λ��־��ȥ13λƬƫ��
    unsigned char ttl;              //8λ����ʱ��
    unsigned char protocol;         //8λ�ϴ�Э���
    unsigned short checksum;        //16λУ���
    unsigned long sourceIP;         //32λԴIP��ַ
    unsigned long destIP;           //32λĿ���ַ
} IP_HEADER;

//ICMP��ͷ
typedef struct{
    BYTE type;                      //8λ�����ֶ�
    BYTE code;                      //8λ�����ֶ�
    USHORT cksum;                   //16λУ���
    USHORT id;                      //16λ��ʶ��
    USHORT seq;                     //16λ���к�
}ICMP_HEADER;

//���Ľ���ṹ
typedef struct{
    USHORT usSeqNo;                 //���к�
    DWORD   dwRoundTripTime;        //����ʱ��
    in_addr dwIPaddr;               //���ر��ĵ�IP��ַ
}DECODE_RESULT;

/**
* ��������У��ͺ���
* �������͵ķ���������,������ʱ����ѭ����λ
*   - ÿ16λȡ�������ȡ��
*   - �ȶ�ÿ16�����ȡ��
* ����Ϊ�ڶ���
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
* �����ݽ��н���
*
*/
BOOL DecodeIcmpResponse(char* pBuf, int IPacketSize, DECODE_RESULT &DecodeResult, BYTE ICMP_ECHO_REPLY, BYTE ICMP_TIMEOUT){
    //������ݱ���С�ĺϷ���
    IP_HEADER *pIpHdr=(IP_HEADER*)pBuf;
    int iIpHdrLen=pIpHdr->hdr_len*4;

    if(IPacketSize<(int)(iIpHdrLen+sizeof(ICMP_HEADER))){
        return FALSE;
    }

    //����ICMP����������ȡID�ֶκ����к��ֶ�
    ICMP_HEADER *pIcmpHdr=(ICMP_HEADER *)(pBuf+iIpHdrLen);
    USHORT usID,usSquNO;
    if(pIcmpHdr->type==ICMP_ECHO_REPLY){    //ICMP����Ӧ����
        usID=pIcmpHdr->id;  //����ID
        usSquNO=pIcmpHdr->seq;  //�������к�
    }else if(pIcmpHdr->type==ICMP_TIMEOUT){ //ICMP��ʱ�����
        char* pInnerIpHdr=pBuf+iIpHdrLen+sizeof(ICMP_HEADER);   //�غ��е�IPͷ
        int iInnerHdrLen=((IP_HEADER *)pInnerIpHdr)->hdr_len*4; //�غ��е�IPͷ��
        ICMP_HEADER *pInnerIcmpHdr=(ICMP_HEADER*)(pInnerIpHdr+iInnerHdrLen);    //�غ��е�ICMPͷ
        usID=pInnerIcmpHdr->id;                                   //����ID
        usSquNO=pInnerIcmpHdr->seq;

    }else{
        return FALSE;
    }

    //���ID�����к���ȷ���յ��ڴ������ݱ�
    if(usID!=(USHORT)GetCurrentProcessId() || usSquNO!=DecodeResult.usSeqNo){
        return FALSE;
    }

    //��¼IP��ַ����������
    DecodeResult.dwIPaddr.s_addr=pIpHdr->sourceIP;
    DecodeResult.dwRoundTripTime=GetTickCount()-DecodeResult.dwRoundTripTime;

    //������ȷ�յ�ICMP���ݱ�
    if(pIcmpHdr->type==ICMP_ECHO_REPLY || pIcmpHdr->type==ICMP_TIMEOUT){
        //�������ʱ����Ϣ
        if(DecodeResult.dwRoundTripTime){
            cout<<"     "<<DecodeResult.dwRoundTripTime<<"ms\t";
        }else{
            cout<<"     "<<"<1ms"<<endl;
        }
    }

}
bool Ping(const char *ipAddress){

 //�õ�IP��ַ
    u_long ulDestIP=inet_addr(ipAddress);



    //cout<<"Tracing route to "<<ipAddress<<"with a maximum of 30 hops.\n\n";

    //���Ŀ��socket��ַ
    sockaddr_in destSockAddr;
    ZeroMemory(&destSockAddr, sizeof(sockaddr_in));
    destSockAddr.sin_family=AF_INET;
    destSockAddr.sin_addr.s_addr=ulDestIP;

    //����ԭʼ�׽���
    /*?*/
    SOCKET sockRaw=WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, NULL, 0, WSA_FLAG_OVERLAPPED);

    //��ʱʱ��
    int iTimeout=3000;
    //���ճ�ʱ
    setsockopt(sockRaw, SOL_SOCKET, SO_RCVTIMEO, (char*)&iTimeout, sizeof(iTimeout));
    //���ͳ�ʱ
    setsockopt(sockRaw, SOL_SOCKET, SO_SNDTIMEO, (char*)&iTimeout, sizeof(iTimeout));

    //����ICMP����������Ϣ������TTL������˳���ͱ���
    //ICMP�����ֶ�
    const BYTE ICMP_ECHO_REQUEST=8;     //�������
    const BYTE ICMP_ECHO_REPLY=0;       //��������
    const BYTE ICMP_TIMEOUT=11;         //���䳬ʱ



    //���������Ķ���
    const int DEF_ICMP_DATA_SIZE=32;    //ICMP����Ĭ�������ֶγ���
    const int MAX_ICMP_PACKET_SIZE=1024;//ICMP������󳤶ȣ�������ͷ��
    const DWORD MAX_ICMP_TIMEOUT=3000;  //������Ӧ��ʱʱ��
    const int DEF_MAX_HOP=30;           //�����ս��

    //���ICMP�������Ǹ�ÿ�η���ʱ����ĳ���
    char IcmpSendBuf[sizeof(ICMP_HEADER)+DEF_ICMP_DATA_SIZE];   //���ͻ�����
    memset(IcmpSendBuf, 0, sizeof(IcmpSendBuf));                //��ʼ��������
    char IcmpRecvBuf[MAX_ICMP_PACKET_SIZE];                     //���ջ�����
    memset(IcmpRecvBuf, 0, sizeof(IcmpRecvBuf));                //��ʼ�����ջ�����

    ICMP_HEADER* pIcmpHdr=(ICMP_HEADER*)IcmpSendBuf;
    pIcmpHdr->type=ICMP_ECHO_REQUEST;                           //����Ϊ�������
    pIcmpHdr->code=0;                                           //�����ֶ�Ϊ0
    pIcmpHdr->id=(USHORT)GetCurrentProcessId();                 //ID�ֶ�Ϊ��ǰ���̺�
    memset(IcmpSendBuf+sizeof(ICMP_HEADER), 'E', DEF_ICMP_DATA_SIZE);   //�����ֶ�

    USHORT usSeqNo=0;               //ICMP��������
    //int iTTL=1;                     //TTL��ʼΪ1
    BOOL bReachDestHost=FALSE;      //ѭ���˳���־
    int iMAXHot=DEF_MAX_HOP;       //ѭ����������
    DECODE_RESULT DecodeResult;     //���ݽ��뺯���Ļ���������
/*
    while(!bReachDestHost&&iMAXHot--){
        //����IP��ͷTTL�ֶ�
        //setsockopt(sockRaw, IPPROTO_IP, IP_TTL, (char*)&iTTL, sizeof(iTTL));
        //cout<<iTTL<<flush;              //�����ǰ���к�

        //���ICMP������ÿ�η��ͱ仯���ֶ�
        ((ICMP_HEADER*)IcmpSendBuf)->cksum=0;                   //У��λ����0
        ((ICMP_HEADER*)IcmpSendBuf)->seq=htons(usSeqNo++);      //������к�
        ((ICMP_HEADER*)IcmpSendBuf)->cksum=checksum((USHORT*)IcmpSendBuf, sizeof(ICMP_HEADER)+DEF_ICMP_DATA_SIZE);  //����У���

        //��¼���кź͵�ǰʱ��
        DecodeResult.usSeqNo=((ICMP_HEADER*)IcmpSendBuf)->seq;  //��ǰ���к�
        DecodeResult.dwRoundTripTime=GetTickCount();            //��ǰʱ��

        //����TCP����������Ϣ
        sendto(sockRaw, IcmpSendBuf, sizeof(IcmpSendBuf), 0, (sockaddr*)&destSockAddr, sizeof(destSockAddr));
        //����ICMP����Ĳ����н�������
        sockaddr_in from;               //�Զ�socket��ַ
        int iFromLen = sizeof(from);    //��ַ�ṹ����
        int iReadDataLen;               //�������ݳ���

        //while(true){
        //��������
        iReadDataLen=recvfrom(sockRaw, IcmpRecvBuf, MAX_ICMP_PACKET_SIZE, 0, (sockaddr*)&from, &iFromLen);
        if(iReadDataLen!=SOCKET_ERROR)
        {
            //�����ݵ���
            //�����ݱ����н���
            if(DecodeIcmpResponse(IcmpRecvBuf, iReadDataLen, DecodeResult, ICMP_ECHO_REPLY, ICMP_TIMEOUT))
            {
                //����Ŀ�ĵأ��˳�ѭ��
                if(DecodeResult.dwIPaddr.s_addr==destSockAddr.sin_addr.s_addr)
                {
                    bReachDestHost=true;
                }

                //���IP��ַ
                cout<<"\t"<<inet_ntoa(DecodeResult.dwIPaddr)<<endl;
                //break;
            }
        }
        else if(WSAGetLastError()==WSAETIMEDOUT)
        {
            //���ճ�ʱ���*
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
    //���ICMP������ÿ�η��ͱ仯���ֶ�
    ((ICMP_HEADER*)IcmpSendBuf)->cksum=0;                   //У��λ����0
    ((ICMP_HEADER*)IcmpSendBuf)->seq=htons(usSeqNo++);      //������к�
    ((ICMP_HEADER*)IcmpSendBuf)->cksum=checksum((USHORT*)IcmpSendBuf, sizeof(ICMP_HEADER)+DEF_ICMP_DATA_SIZE);  //����У���

    //��¼���кź͵�ǰʱ��
    DecodeResult.usSeqNo=((ICMP_HEADER*)IcmpSendBuf)->seq;  //��ǰ���к�
    DecodeResult.dwRoundTripTime=GetTickCount();            //��ǰʱ��



    //bool hasReach=FALSE;
    for(int i=1;i<=2;i++){
        //nt iTTL=255;
        //setsockopt(sockRaw, IPPROTO_IP, IP_TTL, (char*)&iTTL, sizeof(iTTL));
        //����TCP����������Ϣ
        sendto(sockRaw, IcmpSendBuf, sizeof(IcmpSendBuf), 0, (sockaddr*)&destSockAddr, sizeof(destSockAddr));
        //����ICMP����Ĳ����н�������
        sockaddr_in from;               //�Զ�socket��ַ
        int iFromLen = sizeof(from);    //��ַ�ṹ����
        int iReadDataLen;               //�������ݳ���

        //while(true){
        //��������
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
    //��ʼ��Windows sockets���绷��
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);


    char ipAddress[255];
    cout<<"Please Input a range, which format is [a,b], and a,b are Integer between 0 and 255:";

    int a,b;
    scanf("[%d,%d]",&a,&b);

    string networkID="192.168.37.";

    //�ж������Ƿ�Ϸ�
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
