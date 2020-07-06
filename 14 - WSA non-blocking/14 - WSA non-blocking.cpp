// 14 - WSA non-blocking.cpp : Define el punto de entrada de la aplicación.
//

#include "framework.h"
#ifndef IDT_TIMER1
#define IDT_TIMER1 1
#endif // !IDT_TIMER1
#ifndef IDT_TIMER1_MILIS
#define IDT_TIMER1_MILIS 100
#endif // !IDT_TIMER1_MILIS

#include "14 - WSA non-blocking.h"
#include "ClientControl.h"
#include <winsock2.h>

#define MAX_LOADSTRING  100

// Global Variables
HINSTANCE hInst;                                // instancia actual
WCHAR szTitle[MAX_LOADSTRING];                  // Texto de la barra de título
WCHAR szWindowClass[MAX_LOADSTRING];            // nombre de clase de la ventana principal

//********************WSA_NON_BLOKING initialization **************************************
WSA_non_blocking WSAnb;

wchar_t IPString[20] = L"127.0.0.1";
int portNumber = 27015;
/*************************************COMMON CONTROLS ***************************************/
#define IDC_MAINFRM_BTN_1               40501//RESTART button ID
HWND hwndIP, hwndPort,hwndButton;
RECT ComboIPRect1 = { 200,10,150,200 };
RECT EditPortRect1 = { ComboIPRect1.left ,ComboIPRect1.top + 30,ComboIPRect1.right,20 };
static const int size = EditPortRect1.top -ComboIPRect1.top+ EditPortRect1.bottom;
RECT btnRestartRect1 = { ComboIPRect1.left+ ComboIPRect1.right+10 ,ComboIPRect1.top ,size*2,size };
ClientControl* clientControl[WSA_MAXIMUM_WAIT_EVENTS];
unsigned int lastClientControl = 1;

// Function declarations
ATOM                MyRegisterClass(HINSTANCE);
HWND                InitInstance( int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM); 
void                UpdateUI(HWND , HDC , const wchar_t* );
int Ini_WSA_non_blocking(HWND);
void XTrace(LPCTSTR lpszFormat, ...);
void XTrace0(LPCTSTR lpszText);

/// <summary>
/// App entry point
/// </summary>
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    hInst = hInstance; 
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Global strings initialization
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MY14WSANONBLOCKING, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Window Initialization
    HWND hwnd = InitInstance( nCmdShow);
    if (!hwnd) return FALSE;

    //SOCKET initialization
    if (!Ini_WSA_non_blocking(hwnd))return FALSE;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY14WSANONBLOCKING));

    MSG msg;

    // Bucle principal de mensajes:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

/// <summary>
/// Inicialize server class: WSAnb.
/// Inicialize timer IDT_TIMER1 to watch SOCKET the events
/// </summary>
/// <param name="hwnd">Main Window handle</param>
/// <returns>TRUE if succesfull</returns>
BOOL Ini_WSA_non_blocking(HWND hwnd)
{
    if(!WSAnb.CreateServerSocket(IPString, portNumber))return FALSE;
    if (SetTimer(hwnd,           // handle to main window 
        IDT_TIMER1,             // timer identifier 
        IDT_TIMER1_MILIS,       // interval in miliseconds
        (TIMERPROC)NULL)==0     // no timer callback 
        )  return FALSE;
    return TRUE;
}

/// <summary>
/// REgister the window's class
/// </summary>
/// <param name="hInstance">App's Instance handle</param>
/// <returns>Returns a class atom that uniquely identifies the class. Zero if fails</returns>
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MY14WSANONBLOCKING));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MY14WSANONBLOCKING);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

/// <summary>
/// Create and shows the main window.
/// </summary>
/// <param name="nCmdShow">ShowWindow() parameter</param>
/// <returns>Main window handle</returns>
HWND InitInstance( int nCmdShow)
{
   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 900, 300, nullptr, nullptr, hInst, nullptr);

   if (!hWnd)return FALSE;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return hWnd;
}

/// <summary>
/// Initialize the User Interface
/// </summary>
/// <param name="hwnd">main Window Handle</param>
void Ini_UI_server(HWND hwnd) {

    //*******************************IP COMBO BOX + PORT EDIT*************************
    hwndIP = CreateWindowEx(
        0, L"COMBOBOX",         // predefined class 
        L"Recieve:",            // no window title 
        CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
        ComboIPRect1.left,      // x position 
        ComboIPRect1.top,       // y position 
        ComboIPRect1.right,     // Combo width
        ComboIPRect1.bottom,    // Combo height
        hwnd,                   // parent window 
        NULL,                   // No menu. 
        hInst,
        NULL);                  // pointer not needed 

    hwndPort = CreateWindowEx(
        0, L"EDIT",             // predefined class 
        NULL,                   // no window title 
        WS_CHILD | WS_BORDER | WS_VISIBLE | ES_LEFT,
        EditPortRect1.left,
        EditPortRect1.top,
        EditPortRect1.right,
        EditPortRect1.bottom,   // set size in WM_SIZE message 
        hwnd,                   // parent window 
        NULL,                   // No menu. 
        hInst,
        NULL);                  // pointer not needed 
    hwndButton = CreateWindow(
        L"BUTTON",              // Predefined class; Unicode assumed 
        L"RESTART",             // Button text 
        WS_TABSTOP | WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,  // Styles 
        btnRestartRect1.left,   // x position 
        btnRestartRect1.top,    // y position 
        btnRestartRect1.right,  // Button width
        btnRestartRect1.bottom, // Button height
        hwnd,                   // Parent window
        (HMENU)IDC_MAINFRM_BTN_1,// For buttons, hMenu is used to send the WM_BUTTON identifier
        hInst,
        NULL);                  // Pointer to identify the button in WndProc()

}
/// <summary>
/// Main window procedure
/// </summary>
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int control = 0;
    INT iRetval;
    ADDRINFOW* ptr = NULL;
    wchar_t ipstringbuffer[46];
    DWORD ipbufferlength = 46;
    const int textSize = 1000;
    wchar_t text[textSize];
    int ItemIndex;
    switch (message)
    {
    case WM_CREATE:
    {
        Ini_UI_server(hWnd);
        // Add the Port Number to editBox
        wchar_t text2[10];
        _itow_s(portNumber, text2, 10);
        SendMessage(hwndPort, WM_SETTEXT, 0, (LPARAM)text2);
        // Add string to combobox.
        //First add the default localhost IP
        SendMessage(hwndIP, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)IPString);
        //Then. list the avaliable IP's on device
        ADDRINFOW* addrResult;
        WSAnb.GetIPList(&addrResult);
        // Retrieve each address and print out the hex bytes
        for (ptr = addrResult; ptr != NULL; ptr = ptr->ai_next)
        {
            switch (ptr->ai_family) {
            case AF_UNSPEC:
                break;
            case AF_INET:
            {
                // The buffer length is changed by each call to WSAAddresstoString
                // So we need to set it for each iteration through the loop for safety
                ipbufferlength = 46;
                iRetval = WSAAddressToStringW((LPSOCKADDR)ptr->ai_addr, (DWORD)ptr->ai_addrlen, NULL,
                    ipstringbuffer, &ipbufferlength);
                if (iRetval)
                    XTrace(L"WSAAddressToString failed with ", WSAGetLastError());
                else
                {
                    SendMessage(hwndIP, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)ipstringbuffer);
                    // Send the CB_SETCURSEL message to display an initial item 
                    //  in the selection field  
                    SendMessage(hwndIP, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
                }
                break;
            }
            }
        }
        return 0;
    }

    case WM_TIMER:
    {
        switch (wParam)
        {
        case IDT_TIMER1:
            XTrace(L"WM_TIMER\n");
            if (WSAnb.status == WSA_non_blocking::STATE::LISTENING) WSAnb.testForEvents();
            InvalidateRect(hWnd, NULL, TRUE);
            return 0;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    case WM_COMMAND:
    {
        unsigned int control = 1;
        for (; control < lastClientControl; control++) {
            if (lParam == (LPARAM)clientControl[control]->hwnd7_SendButton)
            {
                const int textSize = 1000;
                char text[textSize];
                if (GetWindowTextA(clientControl[control]->hwnd6_SendMessage, text, textSize) != 0)
                    WSAnb.SendText(control, text, strlen(text));
                return 0;
            }
        }
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDC_MAINFRM_BTN_1:
            //Get the IP from combo box
            ItemIndex = SendMessage(hwndIP, (UINT)CB_GETCURSEL,
                (WPARAM)0, (LPARAM)0);
            (TCHAR)SendMessage(hwndIP, (UINT)CB_GETLBTEXT,
                (WPARAM)ItemIndex, (LPARAM)IPString);
            //Get the Port from textbox

            if (GetWindowText(hwndPort, text, textSize) != 0)
            {
                //If the number is correct, we use it.
                //If not, the older is used
                unsigned int number = _wtoi(text);
                if ((number < 65535) && (number > 0)) {
                    portNumber = number;
                }
            }
            WSAnb.CreateServerSocket(IPString, portNumber);
            break;
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        RECT TextRect = { 10,10,300,100 };
        switch (WSAnb.status)
        {
        case WSA_non_blocking::STATE::NONE:
            DrawText(hdc, L"Estado:None", -1, &TextRect, DT_VCENTER | DT_LEFT);
            break;
        case WSA_non_blocking::STATE::CONNECTED:
            UpdateUI(hWnd, hdc, L"Estado:Conectado");
            break;
        case WSA_non_blocking::STATE::LISTENING:
            UpdateUI(hWnd, hdc, L"Estado:Escuchando");
            break;
        default:
            DrawText(hdc, L"Estado:Desconocido", -1, &TextRect, DT_VCENTER | DT_LEFT);
            break;
        }
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
/// <summary>
/// Show the Server's basic parametres ( Status, Ip and Port )  and the client's textboxes
/// </summary>
/// <param name="hwnd">Manejador de la ventana principal</param>
/// <param name="hdc">Device context handler received by BeginPaint () on WM_PAINT</param>
/// <param name="message">Status text</param>
void UpdateUI(HWND hwnd, HDC hdc, const wchar_t* message) {
    //First, we use the basic GDI to show the basic parameters of the Server:
    //Conected/disconected,
    //IP and Port
    RECT TextRect = { 10,10,300,100 };
    DrawText(hdc, message, -1, &TextRect, DT_VCENTER | DT_LEFT);
    TextRect.top += 20;
    TextRect.bottom += 20;
    DrawText(hdc, WSAnb.IPString, -1, &TextRect, DT_VCENTER | DT_LEFT);
    TextRect.top += 20;
    TextRect.bottom += 20;
    DrawText(hdc, WSAnb.PortString, -1, &TextRect, DT_VCENTER | DT_LEFT);

    //************Prepare TexBoxes lines for each conected Client show basic parameters and data ********************************

    // English:
    // Please note that WSAnb.ClientIndex starts at 1 so lastClientControl
     // starts at 1. This is because WSAnb.ClientIndex = 0 would correspond to the SERVER SOCKET
     // Hard to implement "correctly" since WSAWaitForMultipleEvents () must receive
     // a single fix with all the events of all the sockets (server [0] + client [1] + client [2] + client [3] + etc ...)
     if (lastClientControl > WSAnb.ClientIndex)
    {
         // If there are more TexBoxes lines than Clients accepted by the Server,
         // the leftovers are eliminated.
        for (unsigned int index = WSAnb.ClientIndex; index < lastClientControl; index++) {
            delete clientControl[index];
        }
        lastClientControl = WSAnb.ClientIndex;
    }
    if (lastClientControl < WSAnb.ClientIndex) {
        //If there are more accepted Clients than TexBoxes lines, lines are added.
        for (unsigned int index = lastClientControl; index < WSAnb.ClientIndex; index++) {

            clientControl[index] = new ClientControl(hwnd, hInst, 70 + (index * 30), index);

            clientControl[index]->CreateControlWindows();

        }

        lastClientControl = WSAnb.ClientIndex;
    }
    //******************************Update TexBoxes lines content******************************

    wchar_t text[10];
    wchar_t ip_wchar[20];
    wchar_t port_wchar[20];
    for (unsigned int index = 1; index < lastClientControl; index++) {
        switch (WSAnb.StateArray[index])
        {
        case WSA_non_blocking::STATE::NONE:
            wcscpy_s(text, L"Standby");
            break;
        case WSA_non_blocking::STATE::CONNECTED:
            wcscpy_s(text, L"Conectado");
            break;
        default:
            wcscpy_s(text, L"UNKNOW");
            break;
        }
        SetWindowText(clientControl[index]->hwnd2_Status, text);
        WSA_non_blocking::socketaddress_to_string(&WSAnb.AddressArray[index], ip_wchar, port_wchar);

        SetWindowText(clientControl[index]->hwnd3_IP, ip_wchar);
        SetWindowText(clientControl[index]->hwnd4_Port, port_wchar);
        SetWindowTextA(clientControl[index]->hwnd5_RecvMessage, (LPCSTR)WSAnb.BufferRecieved[index]);
    }
}

// Controlador de mensajes del cuadro Acerca de.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void XTrace0(LPCTSTR lpszText)
{
    ::OutputDebugString(lpszText);
}

void XTrace(LPCTSTR lpszFormat, ...)
{
    va_list args;
    va_start(args, lpszFormat);
    int nBuf;
    TCHAR szBuffer[512]; 
    nBuf = _vstprintf_s(szBuffer, 511, lpszFormat, args);
    ::OutputDebugString(szBuffer);
    va_end(args);
}