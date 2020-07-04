#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include "resource.h"
#pragma comment (lib,"Ws2_32.lib")

#define DATA_BUFSIZE 512
#define TIME_OUT_FOR_EVENTS 50 //50 milliseconds



class WSA_non_blocking {
//********************************* STATE *************************************************
public: enum class STATE { NONE,CONNECTED,LISTENING, REQUESTING};
public:STATE status = STATE::NONE;
public:BOOL bConnected = FALSE;
//****************************** SOCKET ****************************************************
public:static const int IPString_Lenght = 50;
public:      wchar_t IPString[IPString_Lenght] = { 0 };
public:      wchar_t PortString[IPString_Lenght] = { 0 };
protected: WSADATA wsaData = { 0 };
protected: const int ServerIndex = 0;
public: unsigned int ClientIndex = 1;
//***************************** EVENTS *****************************************************
public:int TimeOutForEvents = TIME_OUT_FOR_EVENTS;
protected: DWORD EventTotal = 1;
//***************************** Various ******************************************************
protected: int iResult = 0;
public: int lastWSAError = 0;
protected: static const int ErrorBufferLen = 1000;
protected: wchar_t lpBuffer[ErrorBufferLen];
//*******************************  CLIENTS ARRAYS********************************************
protected: WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
protected: SOCKET SocketArray[WSA_MAXIMUM_WAIT_EVENTS];
public: sockaddr AddressArray[WSA_MAXIMUM_WAIT_EVENTS];
public: CHAR BufferRecieved[DATA_BUFSIZE+1][WSA_MAXIMUM_WAIT_EVENTS] = { 0 };
public:int ReceivedBytes[WSA_MAXIMUM_WAIT_EVENTS] = { 0 };
public: BOOL OverflowAlert[WSA_MAXIMUM_WAIT_EVENTS] = { FALSE };
public: STATE StateArray[WSA_MAXIMUM_WAIT_EVENTS] = { STATE::NONE };
//******************************* SEND -RECIEVE data ****************************************
protected: static const int SendBufferSize = 512;
protected: char SendBuffer[SendBufferSize];
protected: int SendBytes = 0;

/// <summary>
/// printf() style debugging
/// https://stackoverflow.com/questions/15240/
/// </summary>
/// <param name="lpszFormat">Debugging text</param>
void XTrace0(LPCTSTR lpszText)
{
    ::OutputDebugString(lpszText);
}

/// <summary>
/// printf() style debugging
/// https://stackoverflow.com/questions/15240/
/// </summary>
/// <param name="lpszFormat">-Debugging text</param>
/// <param name="">.... parameters in _vstprintf_s() style</param>
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

/// <summary>
/// Initialize SERVER.
/// Initialize WSA subsystem.
/// Initializes Server Socket and listens to requesting clients
/// Initializes Server Events
/// </summary>
/// <param name="IPString">Server IP</param>
/// <param name="port">Server Port</param>
/// <returns>True if succeed. FALSE if fails, lastWSAError saves the WSAGelLastError() value</returns>
public: BOOL CreateServerSocket(wchar_t* IPString, int port) {
    lastWSAError = 0;
    // socket() data --------------------
    int iFamily = AF_INET;
    int iType = SOCK_STREAM;
    int iProtocol = IPPROTO_TCP;    
    //  bind() data ---------------------
    sockaddr_in service = { 0 };
    IN_ADDR in_addr = { 0 };
    //  Listen() data -------------------
    int    backlog = 0;
    // Initialize Winsock*****************************************************************
    
    lastWSAError= WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (lastWSAError != 0) {
        XTrace(L"WSAStartup failed: %d. %s\n", lastWSAError, WindowsErrorToString(lastWSAError));
        return FALSE;
    }
    // Create socket****************************************************************
    SocketArray[ServerIndex] = socket(iFamily, iType, iProtocol);
    if (SocketArray[ServerIndex] == INVALID_SOCKET)
    {
        lastWSAError = WSAGetLastError();
        XTrace(L"socket failed: %d. %s\n", lastWSAError, WindowsErrorToString(lastWSAError));
        return FALSE;
    }
    // Set non-blocking mode****************************************************************
    //-------------------------
    // Set the socket I/O mode: In this case FIONBIO
    // enables or disables the blocking mode for the 
    // socket based on the numerical value of iMode.
    // If iMode = 0, blocking is enabled; 
    // If iMode != 0, non-blocking mode is enabled.

    u_long iMode = 1;
    iResult = ioctlsocket(SocketArray[ServerIndex], FIONBIO, &iMode);
    if (iResult == SOCKET_ERROR)
    {
        lastWSAError = WSAGetLastError();
        XTrace(L"ioctlsocket failed: %d. %s\n", lastWSAError, WindowsErrorToString(lastWSAError));
        return FALSE;
    }
    // Bind the socket to a specific port of an IP adress***********************************
    if ((iResult = InetPton(AF_INET, IPString, &in_addr)) != 1) {
        if (iResult == 0) {
            XTrace(L"ioctlsocket failed: IPString is not a valid IP");
            closesocket(SocketArray[ServerIndex]);
            WSACleanup();
            return FALSE;
        }
        lastWSAError = WSAGetLastError();
        XTrace(L"ioctlsocket failed: %d. %s\n", lastWSAError, WindowsErrorToString(lastWSAError));
        closesocket(SocketArray[ServerIndex]);
        WSACleanup();
        return FALSE;

    }
    service.sin_family = AF_INET;
    service.sin_addr = in_addr;
    service.sin_port = htons(port);
    SOCKADDR* prtSOCKADDR = (SOCKADDR*)&service;
    wchar_t IPString2[20];
    wchar_t PortString2[20];
    socketadress_to_string(prtSOCKADDR, IPString2, PortString2);
    iResult = bind(SocketArray[ServerIndex], (SOCKADDR*)&service, sizeof(service));
    if (iResult == SOCKET_ERROR) {
        lastWSAError = WSAGetLastError();
        XTrace(L"bind failed: %d. %s\n", lastWSAError, WindowsErrorToString(lastWSAError));
        closesocket(SocketArray[ServerIndex]);
        WSACleanup();
        return FALSE;
    }
    //The IP and port received are saved in variables of the WSA_non_blocking object
    wcscpy_s(this->IPString, this->IPString_Lenght, IPString);
    _itow_s(port, PortString, 10);
    //-------------------------
    // Associate event types FD_ACCEPT and FD_CLOSE*****************************************
    // with the listening socket and NewEvent
    // Create new event
    EventArray[0] = WSACreateEvent();
    iResult = WSAEventSelect(SocketArray[ServerIndex], EventArray[0], FD_ACCEPT |FD_CONNECT| FD_CLOSE|FD_READ|FD_WRITE);
    if (iResult == SOCKET_ERROR) {
        lastWSAError = WSAGetLastError();
        XTrace(L"WSAEventSelect failed: %d. %s\n", lastWSAError, WindowsErrorToString(lastWSAError));
        closesocket(SocketArray[ServerIndex]);
        WSACleanup();
        return FALSE;
    }
    // Listen *********************************************************************
    iResult =  listen(SocketArray[ServerIndex],1);
    if (iResult == SOCKET_ERROR) {
        lastWSAError = WSAGetLastError();
        XTrace(L"listen failed: %d. %s\n", lastWSAError, WindowsErrorToString(lastWSAError));
        closesocket(SocketArray[ServerIndex]);
        WSACleanup();
        return FALSE;
    }
    XTrace(L"listen succeed\n");
    status = STATE::LISTENING;
    StateArray[ServerIndex] = STATE::LISTENING;
    return TRUE;
}

/// <summary>
/// Check if WSA has triggered an event on the server or on a client.
/// The function waits for TimeOutForEvents milliseconds and returns
/// with a value of 0 if there are no events waiting.
/// If it detects an event, the system launches the corresponding 
/// internal function to accept new clients, close unnecessary sockets, 
/// and load incoming messages into buffers.
/// </summary>
/// <returns>Returns zero if no events have been detected.
/// Returns one, if any event has been processed.
/// Returns SOCKET_ERROR if there has been an error. 
/// In case of error, lastWSAError stores the value returned by WSAGetLastError
///</returns>
public:int testForEvents() {
    lastWSAError = 0;
    WSANETWORKEVENTS NetworkEvents = { 0 };
    // Wait for one of the sockets to receive I/O notification and
    DWORD Event = WSAWaitForMultipleEvents(
        ClientIndex,             //The number of event object handles in the array pointed to by lphEvents. 
        EventArray,             //A pointer to an array of event object handles.              
        FALSE,                  // If FALSE, the function returns when any of the event objects is signaled.
        TimeOutForEvents,       //The time-out interval, in milliseconds.
        FALSE                   //If FALSE, the thread is not placed in an alertable wait state and I/O completion routines are not executed.
    );
    switch (Event)
    {
    case WSA_WAIT_FAILED:
    {
        lastWSAError = WSAGetLastError();
        XTrace(L"WSAWaitForMultipleEvents() failed with error %u: %s\n", lastWSAError, WindowsErrorToString(lastWSAError));
        return SOCKET_ERROR;
    }
    case WSA_WAIT_IO_COMPLETION:
        XTrace(L"WSAWaitForMultipleEvents() WSA_WAIT_IO_COMPLETION\n");
        return 0;
    case WSA_WAIT_TIMEOUT:
        XTrace(L"WSAWaitForMultipleEvents() WSA_WAIT_TIMEOUT\n");
        return 0;
    default:
        break;
    }
    XTrace(L"WSAWaitForMultipleEvents() OK! Numero de Evento en Array:%u\n",Event - WSA_WAIT_EVENT_0);
    iResult = WSAEnumNetworkEvents(
        SocketArray[Event - WSA_WAIT_EVENT_0],          //A descriptor identifying the socket.
        EventArray[Event - WSA_WAIT_EVENT_0],           //An optional handle identifying an associated event object to be reset.
        &NetworkEvents);                                //A structure that is filled with a record of network events that occurred and any associated error codes.
    if (iResult== SOCKET_ERROR)
    {
        lastWSAError = WSAGetLastError();
        XTrace(L"WSAEnumNetworkEvents() failed with error %u: %s\n", lastWSAError, WindowsErrorToString(lastWSAError));
        return SOCKET_ERROR;
    }
    if (NetworkEvents.lNetworkEvents & FD_ACCEPT) {
        XTrace(L"SocketArray[%u]FD_ACCEPT\n", Event - WSA_WAIT_EVENT_0);
        return FD_ACCEPT_response();
    }
    if (NetworkEvents.lNetworkEvents & FD_CLOSE) {
    XTrace(L"SocketArray[%u]FD_CLOSE\n", Event - WSA_WAIT_EVENT_0);
    FD_CLOSE_response(Event - WSA_WAIT_EVENT_0);
    return TRUE;
    }
    if (NetworkEvents.lNetworkEvents & FD_CONNECT) {
        XTrace(L"SocketArray[%u]FD_CONNECT\n", Event - WSA_WAIT_EVENT_0);
        return TRUE;
    }
    if (NetworkEvents.lNetworkEvents & FD_READ) {
        XTrace(L"SocketArray[%u]FD_READ\n", Event - WSA_WAIT_EVENT_0);
        FD_READ_response(Event - WSA_WAIT_EVENT_0);
        return TRUE;
    }
    if (NetworkEvents.lNetworkEvents & FD_WRITE) {
        XTrace(L"SocketArray[%u]FD_WRITE\n", Event - WSA_WAIT_EVENT_0);
        FD_WRITE_response(Event - WSA_WAIT_EVENT_0);
        return TRUE;
    }
    XTrace(L"SocketArray[%u]FD_XXXX WSAEnumNetworkEvents() ha devuelto:%u\n", Event - WSA_WAIT_EVENT_0, NetworkEvents.lNetworkEvents);
    return TRUE;
}
/// <summary>
/// FD_ACCEPT event response.
/// Accept incoming client. Inicialize events, update status arrays and clear Buffers for send/recv data
/// </summary>
/// <returns>TRUE if succeed. FALSE if fails.
/// In case of error, lastWSAError stores the value returned by WSAGetLastError().
///</returns>
protected: BOOL FD_ACCEPT_response() {

    lastWSAError = 0;
    //Create a new ClientSocket to accept the requested conection
    if (ClientIndex >= WSA_MAXIMUM_WAIT_EVENTS)
    {
        XTrace(L"FD_ACCEPT: Top Client conections researched %u:\n", ClientIndex);
        return FALSE;
    }
    int sizeAddress = sizeof(AddressArray[ClientIndex]);
    SocketArray[ClientIndex] = accept(SocketArray[ServerIndex], &AddressArray[ClientIndex], &sizeAddress);
    if (SocketArray[ClientIndex] == INVALID_SOCKET) {
        lastWSAError = WSAGetLastError();
        XTrace(L"accept() failed with error %u: %s\n", lastWSAError, WindowsErrorToString(lastWSAError));
        return FALSE;
    }
    // Associate event types FD_CONNECT | FD_CLOSE | FD_READ | FD_WRITE*****************************************
    // with the listening socket and NewEvent
    // Create new event
    EventArray[ClientIndex] = WSACreateEvent();
    iResult = WSAEventSelect(SocketArray[ClientIndex], EventArray[ClientIndex], FD_CONNECT | FD_CLOSE | FD_READ | FD_WRITE);
    if (iResult == SOCKET_ERROR) {

        lastWSAError = WSAGetLastError();
        XTrace(L"WSAEventSelect failed in SocketArray[%u] with error %u: %s\n", ClientIndex, lastWSAError, WindowsErrorToString(lastWSAError));
        closesocket(SocketArray[ClientIndex]);
        return FALSE;
    }
    //Update Arrays to new client
    StateArray[ServerIndex] = STATE::CONNECTED;
    StateArray[ClientIndex] = STATE::CONNECTED;
    BufferRecieved[ClientIndex][0] = 0;
    ReceivedBytes[ClientIndex] = 0;
    OverflowAlert[ClientIndex] = FALSE;
    ClientIndex++;
    XTrace(L"accept succeed\n");
    return TRUE;
}
/// <summary>
/// FD_CLOSES event response.
/// Closes the client socket. And update the internal Arrays to fill in the gap.
/// </summary>
/// <param name="SocketArrayIndex">Index of the closing socket in the internal SocketArray[]</param>
protected:void FD_CLOSE_response(int SocketArrayIndex) {

    closesocket(SocketArray[SocketArrayIndex]);
    WSACloseEvent(EventArray[SocketArrayIndex]);

    for (unsigned int index = SocketArrayIndex; index < (ClientIndex - 1); index++)
    {
        SocketArray[index] = SocketArray[index + 1];
        EventArray[index] = EventArray[index + 1];
        AddressArray[index] = AddressArray[index + 1];
        StateArray[index] = StateArray[index + 1];
        ReceivedBytes[index] = ReceivedBytes[index + 1];
        OverflowAlert[index] = OverflowAlert[index + 1];
        for (unsigned int byte = 0; byte < WSA_MAXIMUM_WAIT_EVENTS + 1; byte++) {
            BufferRecieved[index][byte] = BufferRecieved[index+1][byte];
        }
    }
    ClientIndex--;
}
/// <summary>
/// FD_READ event response.
/// Read the incoming data and put a zero at the end
/// activates or deactivates the OverflowAlert if the incoming data is bigger or smaller than buffer
/// </summary>
/// <param name="SocketArrayIndex">Index of the receiving socket in the internal SocketArray[]</param>
protected:void FD_READ_response(int SocketArrayIndex) {
    ReceivedBytes[SocketArrayIndex]=recv(SocketArray[SocketArrayIndex], BufferRecieved[SocketArrayIndex], DATA_BUFSIZE, 0);
    if (ReceivedBytes[SocketArrayIndex] >= DATA_BUFSIZE)
    {
        OverflowAlert[SocketArrayIndex] = TRUE;
    }
    else {
        OverflowAlert[SocketArrayIndex] = FALSE;
    }
    //El tamaño del buffer es DATA_BUFSIZE+1 para poder colocar un cero al final
    BufferRecieved[SocketArrayIndex][ReceivedBytes[SocketArrayIndex]] = 0;
}
/// <summary>
/// Called when buffer space becomes available. Not implemented. Does nothing.
/// </summary>
/// <param name="SocketArrayIndex">ClientSocket Index in SocketArray[]</param>
protected:void FD_WRITE_response(int SocketArrayIndex) {
   /* 
   https://docs.microsoft.com/es-es/windows/win32/api/winsock2/nf-winsock2-wsaeventselect?redirectedfrom=MSDN
   
   The FD_WRITE network event is handled slightly differently.
   An FD_WRITE network event is recorded when a socket is first 
   connected with a call to the connect, ConnectEx, WSAConnect, 
   WSAConnectByList, or WSAConnectByName function or when a socket 
   is accepted with accept, AcceptEx, or WSAAccept function and then 
   after a send fails with WSAEWOULDBLOCK and buffer space becomes available.
   Therefore, an application can assume that sends are possible starting 
   from the first FD_WRITE network event settingand lasting until a send 
   returns WSAEWOULDBLOCK.After such a failure the application will find 
   out that sends are again possible when an FD_WRITE network event is 
   recordedand the associated event object is set.
   */
}

/// <summary>
/// Send the content of "char" buffer to a client
/// </summary>
/// <param name="socketIndex">Index of the receiving socket in the internal SocketArray[]</param>
/// <param name="text">text to send</param>
/// <param name="textLen">size of text to send</param>
/// <returns>TRUE if success.FALSE if fails. If socketIndex is out of bounds of "socketIndex"
/// returns fails but lastWSAError is not updated
/// In case of OTHER error, lastWSAError stores the value returned by WSAGetLastError().</returns>
public:BOOL SendText(unsigned int socketIndex, char* text, int textLen) {
    lastWSAError = 0;
    int bytesSend = 0;
    if ((socketIndex>0)&&(socketIndex<ClientIndex)) {
        bytesSend = send(SocketArray[socketIndex], text, textLen, 0);
        if (bytesSend == SOCKET_ERROR) {
            lastWSAError = WSAGetLastError();
            XTrace(L"Error al enviar texto. Codigo: %u = %s", lastWSAError, WindowsErrorToString(lastWSAError));
            return FALSE;
        }
        return TRUE;
    }
    else
    {
        XTrace(L"WSA_non_blocking::SendText() error: socketIndex out of bounds\n");
        return FALSE;
    }
}
/// <summary>
/// My personal version of inet_ntop()
/// </summary>
/// <param name="address">IP structure to retrieve a string</param>
/// <param name="IPString">buffer where the ip will be stored</param>
/// <param name="PortString">buffer where the port will be stored</param>
/// <returns>Puntero a IPString</returns>
public: static wchar_t* socketadress_to_string(sockaddr* address, wchar_t* IPString, wchar_t* PortString) {
    int source = 0, dest = 0;
    wchar_t byte_string[4] = { 0 };
    wchar_t word_string[6] = { 0 };
    for (int nByte = 2; nByte < 6; nByte++) {
        _itow_s(address->sa_data[nByte], byte_string, 10);
        source = 0;
        while (byte_string[source] != 0)
        {
            IPString[dest] = byte_string[source]; source++; dest++;
        }
        if (nByte < 5)IPString[dest++] = L'.';
        else IPString[dest] = 0;
    }

    WORD hByte = (unsigned char)(address->sa_data[0]);
    WORD lByte = (unsigned char)(address->sa_data[1]);
    WORD port = (hByte * 256) +lByte;
    _itow_s(port, word_string, 10);
    source = 0;
    while (word_string[source] != 0) {
        PortString[source] = word_string[source];
        source++;
    }
    PortString[source] = 0;
    return IPString;
}
/// <summary>
/// Translate Windows Error code to Human code
/// </summary>
/// <param name="ErrorCode">Windows Error code</param>
/// <returns>pointer to this->lpBuffer</returns>
public: wchar_t* WindowsErrorToString(int ErrorCode)
{

    if (FormatMessageW( FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        ErrorCode,
        0,
        lpBuffer,
        ErrorBufferLen,
        NULL) == 0)
    {
        XTrace(L"Error with FormatMessage\n");
    }
    return lpBuffer;
}


};