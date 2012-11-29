#include <iostream>
#include <string>
#include <tchar.h>
#include <windows.h>
#include <winsock.h>

#define DEFAULT_BUFLEN 12

char *sendbuf = "Hello World!\0";
char recvbuf[DEFAULT_BUFLEN];
int recvbuflen = DEFAULT_BUFLEN;

SOCKET serverSocket; //Socket handle

//CONNECTTOHOST: Connects to a remote host
bool ConnectToHost(int PortNo, const char * IPAddress)
{
    //Start up Winsock
    WSADATA wsadata;
    int error = WSAStartup(0x0202, &wsadata);
    //Did something happen?
    if (error)
        return false;
    //Did we get the right Winsock version?
    if (wsadata.wVersion != 0x0202)
    {
        WSACleanup(); //Clean up Winsock
        return false;
    }

    //Fill out the information needed to initialize a socket
    SOCKADDR_IN target; //Socket address information

    target.sin_family = AF_INET; // address family Internet
    target.sin_port = htons(PortNo); //Port to connect on
    target.sin_addr.s_addr = inet_addr(IPAddress); //Target IP

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //Create socket
    if (serverSocket == INVALID_SOCKET)
    {
        return false; //Couldn't create the socket
    }  
    //Try connecting...
    if (connect(serverSocket, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR)
    {
		std::cout << "Couldn't connect!" << std::endl;
        return false; //Couldn't connect
    }
    else
	{
		std::cout << "Connected successfully!" << std::endl;
        return true; //Success
	}
}

int SendMsg()
{
	int iResult;
	// Send an initial buffer
	iResult = send(serverSocket, sendbuf, (int) strlen(sendbuf), 0);
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed: %d\n", WSAGetLastError());
		return 1;
	}
	printf("send successful!\n");

	// Receive data until the server closes the connection
	//do
	//{
		iResult = recv(serverSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
		{
			printf("Bytes received: %d\n", iResult);
			printf("message: %s\n", recvbuf);
		}
		else if (iResult == 0)
		{
			printf("Connection closed\n");
		}
		else
		{
			printf("recv failed: %d\n", WSAGetLastError());
		}
		//iResult = send(serverSocket, sendbuf, (int) strlen(sendbuf), 0);
	//}
	//while (iResult > 0);

	return 0;
}

//CLOSECONNECTION: shuts down the socket and closes any connection on it
void CloseConnection ()
{
    //Close the socket if it exists
    if (serverSocket)
        closesocket(serverSocket);
    WSACleanup(); //Clean up Winsock
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::string ipAddr = "127.0.0.1";
	//std::cin >> ipAddr;
	ConnectToHost(5555, ipAddr.c_str());
	SendMsg();
	CloseConnection();
	system("pause");
	return 0;
}

