#include <iostream>
#include <string>
#include <WinSock2.h> //����ƽ̨ Windows
#include <map>
using namespace std;
#pragma comment(lib, "ws2_32.lib") /*����ws2_32.lib��̬���ӿ�*/
#define BUFFSIZE 2048

map<string,string>recvconf,sendconf;

/**

user lyhcc_email@163.com

pass qaz123

*/
bool execmd(map<string,string>conf){
    char buf[500];

}
void show(){
    cout<<"************************************************"<<endl;
    cout<<"��������������£�"<<endl;
    cout<<"     1. �����ʼ�"<<endl;
    cout<<"     2. �����ʼ�"<<endl;
    cout<<"     3. �˳�"<<endl;
    cout<<"************************************************"<<endl;
}


void recvmail(){

    char buff[BUFFSIZE];
    string message;
    WSADATA wsadata;
    if(WSAStartup(MAKEWORD(2, 2), &wsadata)){
        cout<<"error"<<endl;
        return ;
    }
    //�����׽���
    SOCKET sockClient=socket(AF_INET, SOCK_STREAM, 0);
    //��ȡ������������������Ϣ
    HOSTENT* phostent=gethostbyname("pop3.163.com");

    //����˵�ַ
    SOCKADDR_IN addrServer;
    addrServer.sin_addr.S_un.S_addr=*((DWORD*)phostent->h_addr_list[0]);
    addrServer.sin_family=AF_INET;
    addrServer.sin_port=htons(110);

    connect(sockClient, (SOCKADDR*)&addrServer, sizeof(SOCKADDR));
    buff[recv(sockClient, buff, BUFFSIZE, 0)]='\0';
    cout<<"connect: "<<buff<<endl;

    string str;
    cout<<"��������Ҫ��½�����䣺";
    cin>>str;
    message = "user "+str+"\r\n";
    send(sockClient, message.c_str(), message.length(), 0);
    buff[recv(sockClient, buff, BUFFSIZE, 0)]='\0';
    cout<<"user: "<<buff<<endl;

    cout<<"���������룺";
    cin>>str;

    message = "pass "+str+"\r\n";
    send(sockClient, message.c_str(), message.length(), 0);
    buff[recv(sockClient, buff, BUFFSIZE, 0)]='\0';
    cout<<"passwd: "<<buff<<endl;

    message = "list\r\n";
    send(sockClient, message.c_str(), message.length(), 0);
    buff[recv(sockClient, buff, BUFFSIZE, 0)]='\0';
    cout<<"list: \n"<<buff<<endl;
    //memset(buff, 0, sizeof(buff));

    string num;
    cout<<"������Ҫ���������仯����list��ѡ��";
    cin>>num;

    message = "retr "+ num+"\r\n";
    send(sockClient, message.c_str(), message.length(), 0);
    buff[recv(sockClient, buff, BUFFSIZE, 0)]='\0';
    cout<<"���num="<<num<<"�ʼ�\n"<<buff<<endl;
    //memset(buff, 0, sizeof(buff));

    message = "QUIT\r\n";
    send(sockClient, message.c_str(), message.length(), 0);
    buff[recv(sockClient, buff, BUFFSIZE, 0)] = '\0';
    cout<<buff<<endl;
    //cout << "���ճɹ���"<<endl;
    //memset(buff, 0, sizeof(buff));
    closesocket(sockClient);
}
bool sendmail(){
    char buff[BUFFSIZE];
    string message,info,subject;


    WSADATA wsadata;
    if(WSAStartup(MAKEWORD(2, 1), &wsadata)){
        cout<<"error"<<endl;
        return false;
    }

    //�����׽���
    SOCKET sockClient=socket(AF_INET, SOCK_STREAM, 0);
    //��ȡ������������������Ϣ
    HOSTENT* phostent=gethostbyname("smtp.163.com");

    //����˵�ַ
    SOCKADDR_IN addrServer;
    addrServer.sin_addr.S_un.S_addr=*((DWORD*)phostent->h_addr_list[0]);
    addrServer.sin_family=AF_INET;
    addrServer.sin_port=htons(25);


    //��������,������������Ӧ��Ϣ
    int err=connect(sockClient, (SOCKADDR*)&addrServer, sizeof(SOCKADDR));
    //cout<<err<<endl;
    buff[recv(sockClient, buff, BUFFSIZE, 0)]='\0';
    cout<<"connect: "<<buff<<endl;

    /*����EHello���ģ�����½����*/
    message="ehlo 163.com\r\n";
    send(sockClient, message.c_str(), message.length(), 0);
    buff[recv(sockClient, buff, BUFFSIZE, 0)]='\0';
    cout<<strlen(buff)<<endl;
    cout << "helo:" << buff << endl;


    message = "auth login \r\n";
    send(sockClient, message.c_str(), message.length(), 0);
    buff[recv(sockClient, buff, BUFFSIZE, 0)] = '\0';
    cout << "auth login:" << buff << endl;

    /*����base64���û���������*/
     message = "bHloY2NfdGVzdEAxNjMuY29t\r\n"; //base64 ������û���
     send(sockClient, message.c_str(), message.length(), 0);
     buff[recv(sockClient, buff, BUFFSIZE, 0)] = '\0';
     cout << "usrname:" << buff << endl;

      message = "cWF6d3N4MTI=\r\n";//base64 ���������
     send(sockClient, message.c_str(), message.length(), 0);
     buff[recv(sockClient, buff, BUFFSIZE, 0)] = '\0';
     cout << "password:" << buff << endl;

     string mail;
     cout << "�����ռ������䣺";
     cin >> mail;
     message = "MAIL FROM:<lyhcc_test@163.com> \r\nRCPT TO:<";
    // message = "MAIL FROM:<XXX@qq.com> \r\nRCPT TO:<XXX@163.com> \r\n";

     message.append(mail);
     message.append("> \r\n");
     //cout << "message=" << message;

     send(sockClient, message.c_str(), message.length(), 0);

     buff[recv(sockClient, buff, BUFFSIZE, 0)] = '\0';
     cout << "mail from: " << buff << endl;
     buff[recv(sockClient, buff, BUFFSIZE, 0)] = '\0';
     cout << "rcpt to: " << buff << endl;
     /*
     ʹ�� DATA ������߷�����Ҫ�����ʼ�����
     */
     message = "DATA\r\n";
     send(sockClient, message.c_str(), message.length(), 0);
     buff[recv(sockClient, buff, 1024, 0)] = '\0';
     //cout << "data: " << buff << endl;

    message = "from:lyhcc_test<lyhcc_test@163.com>\r\nto:lyhcc_email<lyhcc_email@163.com>\r\nCc:lyhcc_test@163.com\r\nsubject:";

    cout<<"���⣺";
    cin>>subject;

    message.append(subject);
    message.append("\r\n\r\n");
    cout<<"���ݣ�";
    cin>>info;
    message.append(info);
    message.append("\r\n.\r\n");
    send(sockClient, message.c_str(), message.length(), 0);
    buff[recv(sockClient, buff, 1024, 0)] = '\0';
    cout << "data: " << buff << endl;
    message = "QUIT\r\n";
    send(sockClient, message.c_str(), message.length(), 0);
    buff[recv(sockClient, buff, 1024, 0)] = '\0';

    closesocket(sockClient);
}

int main()
{
    int op;
    while(true){
        op=0;
        show();
        while(!(op>=1&&op<=3)){
            cout<<"�����������";
            scanf("%d",&op);
        }


        switch(op){

        case 1:
            sendmail();
            break;
        case 2:
            recvmail();
            break;
        case 3:
            return 0;
        default:
            break;
        }

    }


    return 0;
}
