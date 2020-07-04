#pragma once
#include <Windows.h>
class ClientControl {
protected:
    HWND ParentWindow=NULL;
    HINSTANCE hInst=NULL;
    int index=0;
    const int margin = 10;
public:
    HWND hwnd1_ClientName=NULL;
    HWND hwnd2_Status = NULL;
    HWND hwnd3_IP = NULL;
    HWND hwnd4_Port = NULL;
    HWND hwnd5_RecvMessage = NULL;
    HWND hwnd6_SendMessage = NULL;
    HWND hwnd7_SendButton = NULL;

    RECT rect0_Control;

    RECT rect1_ClientName;
    RECT rect2_Status;
    RECT rect3_IP;
    RECT rect4_Port;
    RECT rect5_RecvMessage;
    RECT rect6_SendMessage;
    RECT rect7_SendButton;

protected:void Ini(HWND parentHwnd, HINSTANCE hInst, int top, int index) {
        this->ParentWindow = parentHwnd;
        this->hInst = hInst;
        this->index = index;

        rect0_Control.top = top;
        rect0_Control.bottom = 20;//Height
        rect0_Control.left = margin;

        rect1_ClientName.top = rect0_Control.top;
        rect2_Status.top = rect0_Control.top;
        rect3_IP.top = rect0_Control.top;
        rect4_Port.top = rect0_Control.top;
        rect5_RecvMessage.top = rect0_Control.top;
        rect6_SendMessage.top = rect0_Control.top;
        rect7_SendButton.top = rect0_Control.top;

        rect1_ClientName.bottom = rect0_Control.bottom;
        rect2_Status.bottom = rect0_Control.bottom;
        rect3_IP.bottom = rect0_Control.bottom;
        rect4_Port.bottom = rect0_Control.bottom;
        rect5_RecvMessage.bottom = rect0_Control.bottom;
        rect6_SendMessage.bottom = rect0_Control.bottom;
        rect7_SendButton.bottom = rect0_Control.bottom;

        rect1_ClientName.left = rect0_Control.left;
        rect1_ClientName.right = 60;//width
        rect2_Status.left = rect1_ClientName.left + rect1_ClientName.right + margin;
        rect2_Status.right = 70;//width
        rect3_IP.left = rect2_Status.left + rect2_Status.right + margin;
        rect3_IP.right = 70;//width
        rect4_Port.left = rect3_IP.left + rect3_IP.right + margin;
        rect4_Port.right = 70;//width
        rect5_RecvMessage.left = rect4_Port.left + rect4_Port.right + margin;
        rect5_RecvMessage.right = 220;//width
        rect6_SendMessage.left = rect5_RecvMessage.left + rect5_RecvMessage.right + margin;
        rect6_SendMessage.right = 220;//width
        rect7_SendButton.left = rect6_SendMessage.left + rect6_SendMessage.right + margin;
        rect7_SendButton.right = 60;//width
}
public: 
    ClientControl(HWND parentHwnd, HINSTANCE hInst) {

        Ini( parentHwnd,  hInst, 0, 0);
    }
    ClientControl(HWND parentHwnd, HINSTANCE hInst, int top, int index)
    {
        Ini( parentHwnd,  hInst, top, index);
    }
    ~ClientControl() {
        DestroyWindow(hwnd1_ClientName);
        DestroyWindow(hwnd2_Status);
        DestroyWindow(hwnd3_IP);
        DestroyWindow(hwnd4_Port);
        DestroyWindow(hwnd5_RecvMessage);
        DestroyWindow(hwnd6_SendMessage);
        DestroyWindow(hwnd7_SendButton);
    }
void CreateControlWindows() {
    wchar_t buffer[20],bufferNumber[20];
    _itow_s(this->index, bufferNumber, 10);
    wcscpy_s(buffer, L"Client[");
    wcscat_s(buffer, bufferNumber);
    wcscat_s(buffer, L"]");
    hwnd1_ClientName = CreateWindowClient(L"STATIC", buffer, 0, &rect1_ClientName);
    hwnd2_Status = CreateWindowClient(L"STATIC", L"UNKNOW", 0, &rect2_Status);
    hwnd3_IP = CreateWindowClient(L"STATIC", L"UNKNOW", 0, &rect3_IP);
    hwnd4_Port = CreateWindowClient(L"STATIC", L"UNKNOW", 0, &rect4_Port);
    hwnd5_RecvMessage = CreateWindowClient(L"STATIC", L"-NO MESSAGE-", 0, &rect5_RecvMessage);
    hwnd6_SendMessage = CreateWindowClient(L"EDIT", NULL, 0, &rect6_SendMessage);
    hwnd7_SendButton = CreateWindowClient(L"BUTTON", L"SEND", ES_LEFT, &rect7_SendButton);

}
protected:HWND CreateWindowClient(const wchar_t* predClass,const wchar_t* tittle, long style,RECT* dim) {
    return CreateWindowEx(
        0, predClass,   // predefined class 
        tittle,         // no window title 
        WS_CHILD | WS_BORDER | style | WS_VISIBLE,
        dim->left,
        dim->top,
        dim->right,      // width
        dim->bottom,     // height
        ParentWindow,   // parent window 
        NULL,           // No menu. 
        hInst,
        NULL);          // pointer not needed 
}
};
