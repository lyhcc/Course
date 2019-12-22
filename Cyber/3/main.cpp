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

#define MCASTADDR "233.0.0.1"   //����ʹ�õĶಥ���ַ
#define MCASTPORT 5551          //���ض˿�
#define BUFSIZE 1024            //�������ݻ�����
#define NUMLINES 100



static TCHAR text[BUFSIZE*3]; // ���ڴ洢����ʾ�����ݻ򼴽�Ҫ��ʾ������

/*��Sender��Reciver���еĲ���*/
static int len;                 //
static WSADATA wsd;
static struct sockaddr_in local,remote, from;
SOCKET sock,sockM;

//���ջ������ͷ��ͻ�����
static TCHAR recvBuf[BUFSIZE],buf[BUFSIZE];
/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
TCHAR szClassName[ ] = _T("CodeBlocksWindowsApp");


/*�������ݵĺ����������̵߳���������*/
void* rcptMsg(void* args){
    //SOCKET sockM=*((SOCKET*)args);
    int ret;
    //���նಥ���ݣ����û�����̨�����QUIT��ʱ�˳�
    while(true){


        ret=recvfrom(sockM, recvBuf, BUFSIZE, 0, (struct sockaddr*)&from, &len);
        if(ret==SOCKET_ERROR){
            printf("recv failed whit:%d\n", WSAGetLastError());

            closesocket(sockM);
            closesocket(sock);
            WSACleanup();

        }
        /*�˳�����*/
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


    /*����������*/
    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           _T("IP�ಥ"),       /* Title Text */
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
    //��ʼ��Winsock2.2
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
    //��sock�������˿�
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

    //����ಥ
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

    /*ʹ���߳���������*/
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
    //�Ƿ���Ҫ����
    bool update=false;

    static HFONT hFont;     //����
    static HWND hBtnSend;   //���Ͱ�ť
    static HWND hEditSend;  //�༭��
    static HWND hLabMessage;//������Ϣ��

    /**�������������*/
    //TEXTMETRIC  tm ;
    //SCROLLINFO  si ;
    //static int  cxChar, cxCaps, cyChar, cxClient, cyClient, iMaxWidth ;
    //HDC    hdc ;
    //int    i, x, y, iVertPos, iHorzPos, iPaintBeg, iPaintEnd ;

    /*��text�ÿ�*/
    memset(text, 0, sizeof(text));
    GetWindowText(hLabMessage, text, 4096);

    /*�ж��Ƿ���������
        ����ʵ����
            ���������ݷŵ���һ���黺�屨�������������ÿգ��������жϽ��պ󱸷ݵ�����buf�Ƿ������ݣ�
            �����ݽ���ƾ�赽text��ָ���������Ҫ�����޸�
            ͬʱ�����buf����ֹ�´��ٴν��У�
            WindowProcedure��һֱ���ڣ��������̣߳����п��ܣ�����ѭ��������Ͳ���Ҫ�������

    */
    if(strlen(buf)>0){
        sprintf(text+strlen(text), "RECV:'%s' From <%s>\n", buf, inet_ntoa(from.sin_addr));
        update=true;
        memset(buf, '\0', sizeof(buf));
    }





    switch (message)                  /* handle the messages */
    {
        case WM_CREATE:             //��һ�ν��������ʱ�����У���������Ԫ��
            hFont = CreateFont(-25/*��*/, -7/*��*/, 0, 0, 400 /*һ�����ֵ��Ϊ400*/,
                FALSE/*б��?*/, FALSE/*�»���?*/, FALSE/*ɾ����?*/,DEFAULT_CHARSET,
                OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY,
                FF_DONTCARE, TEXT("΢���ź�")
            );


            hEditSend = CreateWindow(TEXT("edit"), TEXT("ABC"),
                WS_CHILD | WS_VISIBLE | WS_BORDER /*�߿�*/ | ES_AUTOHSCROLL /*ˮƽ����*/,
                5, 290, 400, 35,
                hwnd, (HMENU)3, NULL, NULL
            );

            hBtnSend = CreateWindow(TEXT("button"), TEXT("Send"),
                WS_CHILD | WS_VISIBLE | WS_BORDER | BS_FLAT/*��ƽ��ʽ*/,
                406, 290, 110, 35,
                hwnd, (HMENU)5, NULL, NULL
            );



            hLabMessage =  CreateWindow(TEXT("static"), TEXT(""),
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | SS_LEFT /*ˮƽ����*/,
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
            /*�����������������ֻ��Ҫ������ť�¼��������5��ʾ�ÿռ�ı�ţ��ڴ�����ť�ؼ���ָ��*/
            switch(LOWORD(wParam)){
                case 5:
                    char strText[BUFSIZE]={0};

                    //GetWindowText(hwnd, strText, 2048);
                    //GetWindowText())
                    //SendMessage(hLabMessage, EM_SETSEL, -2, -1);

                    /*��ȡ�ؼ�������ı��������Ǳ༭�򣬱༭��������ÿ�*/
                    GetWindowText(hEditSend, strText, 2048);
                    SetWindowText(hEditSend, "");

                    /*�������жϵ����ť���Ƿ������ݣ�û�������*/
                    if(strcmp(strText, "")!=0){
                        update=true;
                        /*ƴ�����ݵ�text*/
                        sprintf(text+strlen(text), "Sender: %s\n", strText);

                        //���Ͷಥ��Ϣ
                        if((sendto(sockM, (char*)strText, strlen(strText), 0, (sockaddr*)&remote, sizeof(remote)))==SOCKET_ERROR){
                            printf("sendto failed whit:%d\n", WSAGetLastError());

                            closesocket(sockM);
                            closesocket(sock);
                            WSACleanup();
                            return -1;
                        }

                        //�˳�
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
    /*������ʾ*/
    if(update)SetWindowText(hLabMessage, text);
    return 0;
}
