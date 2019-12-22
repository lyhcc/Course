#include <iostream>
#include <string>
#include <WinSock2.h> //适用平台 Windows
#include <map>
using namespace std;
#pragma comment(lib, "ws2_32.lib") /*链接ws2_32.lib动态链接库*/
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
    cout<<"邮箱操作命令如下："<<endl;
    cout<<"     1. 发送邮件"<<endl;
    cout<<"     2. 接收邮件"<<endl;
    cout<<"     3. 退出"<<endl;
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
    //创建套接字
    SOCKET sockClient=socket(AF_INET, SOCK_STREAM, 0);
    //获取邮箱服务器域名相关信息
    HOSTENT* phostent=gethostbyname("pop3.163.com");

    //服务端地址
    SOCKADDR_IN addrServer;
    addrServer.sin_addr.S_un.S_addr=*((DWORD*)phostent->h_addr_list[0]);
    addrServer.sin_family=AF_INET;
    addrServer.sin_port=htons(110);

    connect(sockClient, (SOCKADDR*)&addrServer, sizeof(SOCKADDR));
    buff[recv(sockClient, buff, BUFFSIZE, 0)]='\0';
    cout<<"connect: "<<buff<<endl;

    string str;
    cout<<"请输入你要登陆的邮箱：";
    cin>>str;
    message = "user "+str+"\r\n";
    send(sockClient, message.c_str(), message.length(), 0);
    buff[recv(sockClient, buff, BUFFSIZE, 0)]='\0';
    cout<<"user: "<<buff<<endl;

    cout<<"请输入密码：";
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
    cout<<"请输入要输出的邮箱变化，在list中选择";
    cin>>num;

    message = "retr "+ num+"\r\n";
    send(sockClient, message.c_str(), message.length(), 0);
    buff[recv(sockClient, buff, BUFFSIZE, 0)]='\0';
    cout<<"序号num="<<num<<"邮件\n"<<buff<<endl;
    //memset(buff, 0, sizeof(buff));

    message = "QUIT\r\n";
    send(sockClient, message.c_str(), message.length(), 0);
    buff[recv(sockClient, buff, BUFFSIZE, 0)] = '\0';
    cout<<buff<<endl;
    //cout << "接收成功！"<<endl;
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

    //创建套接字
    SOCKET sockClient=socket(AF_INET, SOCK_STREAM, 0);
    //获取邮箱服务器域名相关信息
    HOSTENT* phostent=gethostbyname("smtp.163.com");

    //服务端地址
    SOCKADDR_IN addrServer;
    addrServer.sin_addr.S_un.S_addr=*((DWORD*)phostent->h_addr_list[0]);
    addrServer.sin_family=AF_INET;
    addrServer.sin_port=htons(25);


    //建立连接,并接收连接响应信息
    int err=connect(sockClient, (SOCKADDR*)&addrServer, sizeof(SOCKADDR));
    //cout<<err<<endl;
    buff[recv(sockClient, buff, BUFFSIZE, 0)]='\0';
    cout<<"connect: "<<buff<<endl;

    /*发送EHello报文，并登陆邮箱*/
    message="ehlo 163.com\r\n";
    send(sockClient, message.c_str(), message.length(), 0);
    buff[recv(sockClient, buff, BUFFSIZE, 0)]='\0';
    cout<<strlen(buff)<<endl;
    cout << "helo:" << buff << endl;


    message = "auth login \r\n";
    send(sockClient, message.c_str(), message.length(), 0);
    buff[recv(sockClient, buff, BUFFSIZE, 0)] = '\0';
    cout << "auth login:" << buff << endl;

    /*发送base64的用户名和密码*/
     message = "bHloY2NfdGVzdEAxNjMuY29t\r\n"; //base64 编码的用户名
     send(sockClient, message.c_str(), message.length(), 0);
     buff[recv(sockClient, buff, BUFFSIZE, 0)] = '\0';
     cout << "usrname:" << buff << endl;

      message = "cWF6d3N4MTI=\r\n";//base64 编码的密码
     send(sockClient, message.c_str(), message.length(), 0);
     buff[recv(sockClient, buff, BUFFSIZE, 0)] = '\0';
     cout << "password:" << buff << endl;

     string mail;
     cout << "输入收件人邮箱：";
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
     使用 DATA 命令告诉服务器要发送邮件内容
     */
     message = "DATA\r\n";
     send(sockClient, message.c_str(), message.length(), 0);
     buff[recv(sockClient, buff, 1024, 0)] = '\0';
     //cout << "data: " << buff << endl;

    message = "from:lyhcc_test<lyhcc_test@163.com>\r\nto:lyhcc_email<lyhcc_email@163.com>\r\nCc:lyhcc_test@163.com\r\nsubject:";

    cout<<"主题：";
    cin>>subject;

    message.append(subject);
    message.append("\r\n\r\n");
    cout<<"内容：";
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
            cout<<"请输入操作：";
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
