#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#include <tchar.h>
#include <windows.h>
#include <string>
#include <iostream>
#include <string.h>
#include <pthread.h>
#include <queue>
using namespace std;

#define MCASTADDR "233.0.0.1"   //本例使用的多播组地址
#define MCASTPORT 5551          //本地端口
#define BUFSIZE 1024            //发送数据缓冲区
#define NUMLINES 100



static TCHAR text[BUFSIZE*3]; // 用于存储已显示的数据或即将要显示的数据

/*在Sender和Reciver都有的参数*/
static int len;                 //
static WSADATA wsd;
static struct sockaddr_in local,remote, from;
SOCKET sock,sockM;

//接收缓冲区和发送缓冲区
static TCHAR recvBuf[BUFSIZE],buf[BUFSIZE];
/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
TCHAR szClassName[ ] = _T("CodeBlocksWindowsApp");


/*接收数据的函数，将于线程的启动函数*/
void* rcptMsg(void* args){
    //SOCKET sockM=*((SOCKET*)args);
    int ret;
    //接收多播数据，当用户控制台输出“QUIT”时退出
    while(true){


        ret=recvfrom(sockM, recvBuf, BUFSIZE, 0, (struct sockaddr*)&from, &len);
        if(ret==SOCKET_ERROR){
            printf("recv failed whit:%d\n", WSAGetLastError());

            closesocket(sockM);
            closesocket(sock);
            WSACleanup();

        }
        /*退出接收*/
        if(strcmp(recvBuf, "QUIT")==0)break;
        else {
            recvBuf[ret]='\0';
            printf("RECV:'%s' From <%s>\n", recvBuf, inet_ntoa(from.sin_addr));
            memcpy(buf, recvBuf, sizeof(recvBuf));
            //memcpy(recvBuf, 0, sizeof(recvBuf));
        }
    }
}


int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{







    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;


    /*创建主界面*/
    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           _T("IP多播"),       /* Title Text */
           WS_OVERLAPPEDWINDOW, /* default window */
           CW_USEDEFAULT,       /* Windows decides the position */
           CW_USEDEFAULT,       /* where the window ends up on the screen */
           544,                 /* The programs width */
           375,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           hThisInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );


    len = sizeof(struct sockaddr_in);
    //初始化Winsock2.2
    if(WSAStartup(MAKEWORD(2, 2), &wsd)!=0)
    {
        printf("WSAStartup() failed\n");

        return -1;
    }
    sock=WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_MULTIPOINT_C_LEAF|WSA_FLAG_MULTIPOINT_D_LEAF|WSA_FLAG_OVERLAPPED);
    if(sock==INVALID_SOCKET){
        printf("socket failed with:%d\n", WSAGetLastError());
        return -1;
    }
    //绑定sock到本机端口
    local.sin_family=AF_INET;
    local.sin_port=htons(MCASTPORT);
    local.sin_addr.s_addr=INADDR_ANY;

    int code=bind(sock, (sockaddr*)&local, sizeof(local));
    if(code==SOCKET_ERROR)
    {

        printf("socket failed with:%d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    //加入多播
    remote.sin_family=AF_INET;
    remote.sin_port=htons(MCASTPORT);
    remote.sin_addr.s_addr=inet_addr(MCASTADDR);

    sockM=WSAJoinLeaf(sock, (SOCKADDR*)&remote, sizeof(remote), NULL, NULL, NULL, NULL, JL_BOTH);
    if(sockM==INVALID_SOCKET)
    {
        printf("WSAJoinLeaf() failed:%d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    /*使用线程启动接收*/
    pthread_t tid;
    int c=pthread_create(&tid, NULL, rcptMsg, NULL);
    if (c != 0)
    {
        cout << "pthread_create error: error_code=" << c << endl;
    }

     //SendMessage(hEditUsername, WM_SETFONT, (WPARAM)hFont, NULL);
    /* Make the window visible on the screen */
    ShowWindow (hwnd, nCmdShow);


    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);

        //printf("HKJKJKJKJKJ");

    }

    closesocket(sockM);
    closesocket(sock);
    WSACleanup();
    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}


/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //是否需要更新
    bool update=false;

    static HFONT hFont;     //字体
    static HWND hBtnSend;   //发送按钮
    static HWND hEditSend;  //编辑框
    static HWND hLabMessage;//发送信息框

    /**滚动条所需变量*/
    //TEXTMETRIC  tm ;
    //SCROLLINFO  si ;
    //static int  cxChar, cxCaps, cyChar, cxClient, cyClient, iMaxWidth ;
    //HDC    hdc ;
    //int    i, x, y, iVertPos, iHorzPos, iPaintBeg, iPaintEnd ;

    /*将text置空*/
    memset(text, 0, sizeof(text));
    GetWindowText(hLabMessage, text, 4096);

    /*判断是否有数据来
        具体实现是
            将接收数据放到另一数组缓冲报错起来将接收置空，在这里判断接收后备份的数据buf是否有数据，
            有数据将其凭借到text，指明在最后需要进行修改
            同时，清空buf，防止下次再次进行，
            WindowProcedure会一直存在，可能以线程（更有可能）或者循环，这里就不需要进行深究了

    */
    if(strlen(buf)>0){
        sprintf(text+strlen(text), "RECV:'%s' From <%s>\n", buf, inet_ntoa(from.sin_addr));
        update=true;
        memset(buf, '\0', sizeof(buf));
    }





    switch (message)                  /* handle the messages */
    {
        case WM_CREATE:             //第一次进入这里的时候运行，创建窗口元素
            hFont = CreateFont(-25/*高*/, -7/*宽*/, 0, 0, 400 /*一般这个值设为400*/,
                FALSE/*斜体?*/, FALSE/*下划线?*/, FALSE/*删除线?*/,DEFAULT_CHARSET,
                OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY,
                FF_DONTCARE, TEXT("微软雅黑")
            );


            hEditSend = CreateWindow(TEXT("edit"), TEXT("ABC"),
                WS_CHILD | WS_VISIBLE | WS_BORDER /*边框*/ | ES_AUTOHSCROLL /*水平滚动*/,
                5, 290, 400, 35,
                hwnd, (HMENU)3, NULL, NULL
            );

            hBtnSend = CreateWindow(TEXT("button"), TEXT("Send"),
                WS_CHILD | WS_VISIBLE | WS_BORDER | BS_FLAT/*扁平样式*/,
                406, 290, 110, 35,
                hwnd, (HMENU)5, NULL, NULL
            );



            hLabMessage =  CreateWindow(TEXT("static"), TEXT(""),
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | SS_LEFT /*水平居左*/,
                5, 5, 510, 280,
                hwnd, (HMENU)2, NULL, NULL
            );

            SendMessage(hEditSend, WM_SETFONT, (WPARAM)hFont, NULL);
            SendMessage(hBtnSend, WM_SETFONT, (WPARAM)hFont, NULL);
            SendMessage(hLabMessage, WM_SETFONT, (WPARAM)hFont, NULL);

            //SetScrollRange(hLabMessage, SB_VERT, 0, 500, FALSE);


        /*hdc = GetDC (hLabMessage) ;
        GetTextMetrics (hdc, &tm) ;
        cxChar = tm.tmAveCharWidth ;
        cxCaps = (tm.tmPitchAndFamily & 1 ? 3 : 2) * cxChar / 2 ;
        cyChar = tm.tmHeight + tm.tmExternalLeading ;
        ReleaseDC (hwnd, hdc) ;
        // Save the width of the three columns
        iMaxWidth = 40 * cxChar + 22 * cxCaps ;*/
            /*?*/
            break;

        case WM_COMMAND:
            /*有命令，即操作，这里只需要监听按钮事件，下面的5表示该空间的编号，在创建按钮控件是指定*/
            switch(LOWORD(wParam)){
                case 5:
                    char strText[BUFSIZE]={0};

                    //GetWindowText(hwnd, strText, 2048);
                    //GetWindowText())
                    //SendMessage(hLabMessage, EM_SETSEL, -2, -1);

                    /*获取控件里面的文本，这里是编辑框，编辑框的内容置空*/
                    GetWindowText(hEditSend, strText, 2048);
                    SetWindowText(hEditSend, "");

                    /*在这里判断点击按钮后是否有数据，没有则忽略*/
                    if(strcmp(strText, "")!=0){
                        update=true;
                        /*拼接数据到text*/
                        sprintf(text+strlen(text), "Sender: %s\n", strText);

                        //发送多播信息
                        if((sendto(sockM, (char*)strText, strlen(strText), 0, (sockaddr*)&remote, sizeof(remote)))==SOCKET_ERROR){
                            printf("sendto failed whit:%d\n", WSAGetLastError());

                            closesocket(sockM);
                            closesocket(sock);
                            WSACleanup();
                            return -1;
                        }

                        //退出
                        if(strcmp(strText, "QUIT")==0){
                            closesocket(sockM);
                            closesocket(sock);
                            WSACleanup();
                            return 0;
                        }

                    }
                    break;


            }

            break;
        /****/


        /****/
        case WM_DESTROY:
            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
            break;

        default:                      /* for messages that we don't deal with */
            if(update)SetWindowText(hLabMessage, text);
            return DefWindowProc (hwnd, message, wParam, lParam);
    }
    /*更新显示*/
    if(update)SetWindowText(hLabMessage, text);
    return 0;
}
