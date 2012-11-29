#include <iostream>
#include <tchar.h>
#include <windows.h>
#include <winsock.h>

#define DEFAULT_BUFLEN 12
#define MAX_CLIENTS 2
#define PORT 5555

char recvbuf[DEFAULT_BUFLEN];
int recvbuflen = DEFAULT_BUFLEN;

SOCKET listenSocket;
SOCKET clientSocket[MAX_CLIENTS];
bool clientSocketProcessed[MAX_CLIENTS];

bool UnprocessedSocketsLeft()
{
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (!clientSocketProcessed[i]) return true;
	}
	return false;
}

void AddressToString(struct sockaddr addr, char *result)
{
	unsigned char d1 = (unsigned char)addr.sa_data[2];
	unsigned char d2 = (unsigned char)addr.sa_data[3];
	unsigned char d3 = (unsigned char)addr.sa_data[4];
	unsigned char d4 = (unsigned char)addr.sa_data[5];
	sprintf(result, "%d.%d.%d.%d", d1, d2, d3, d4);
}

//in_addr tmp;
//get_in_addr(&clientAddr, tmp);
//void get_in_addr(struct sockaddr *sa, struct in_addr &result)
//{
//	if (sa->sa_family == AF_INET)
//	{
//		result = (((struct sockaddr_in*)sa)->sin_addr);
//	}
//	//return &(((struct sockaddr_in6*)sa)->sin6_addr);
//}

//LISTENONPORT: Listens on a specified port for incoming connections or data
int ListenOnPort(int portno)
{
	WSADATA w;
    int error = WSAStartup (0x0202, &w);   // Fill in WSA info

    if (error)
    {
        return false; //For some reason we couldn't start Winsock
    }

    if (w.wVersion != 0x0202) //Wrong Winsock version?
    {
        WSACleanup ();
        return false;
    }

    SOCKADDR_IN addr; // The address structure for a TCP socket

    addr.sin_family = AF_INET;      // Address family
    addr.sin_port = htons(portno);   // Assign port to this socket

    //Accept a connection from any IP using INADDR_ANY
    //You could pass inet_addr("0.0.0.0") instead to accomplish the 
    //same thing. If you want only to watch for a connection from a 
    //specific IP, specify that //instead.

    addr.sin_addr.s_addr = htonl (INADDR_ANY);  
    listenSocket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP); // Create socket

    if (listenSocket == INVALID_SOCKET)
    {
        return false; //Don't continue if we couldn't create a //socket!!
    }

    if (bind(listenSocket, (LPSOCKADDR)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
       //We couldn't bind (this will happen if you try to bind to the same  
       //socket more than once)
        return false;
    }

	size_t hostNameSize = 32;
	char hostName[32];
	gethostname(hostName, hostNameSize);
	printf("Host: %s\n", hostName);

    //Now we can start listening (allowing as many connections as possible to  
    //be made at the same time using SOMAXCONN). You could specify any 
    //integer value equal to or lesser than SOMAXCONN instead for custom 
    //purposes). The function will not //return until a connection request is 
    //made
	int listenResult = listen(listenSocket, SOMAXCONN);

	//Don't forget to clean up with CloseConnection()!

	std::cout << "Listening to port #" << portno << std::endl;

	for (int clientIdx = 0; clientIdx < MAX_CLIENTS; clientIdx++)
	{
		printf("Waiting for client #%d to connect...\n", clientIdx+1);

		// Accept the connection.
		struct sockaddr clientAddr;
		int clientAddrLen;
		//clientSocket[clientIdx] = accept(listenSocket, &clientAddr, &clientAddrLen);
		clientSocket[clientIdx] = accept(listenSocket, &clientAddr, NULL);
		clientSocketProcessed[clientIdx] = false;
		if (clientSocket[clientIdx] == INVALID_SOCKET)
		{
			printf("accept failed with error: %ld\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}
		else
		{
			char ipAddrStr[16];
			AddressToString(clientAddr, ipAddrStr);
			//getpeername(clientSocket[clientIdx], &clientAddr, &clientAddrLen);
			printf("%s - ", ipAddrStr);
			printf("Client connected.\n");
		}
	}
	return true;
}

int ReceiveMsg()
{
	int iResult, iSendResult;
	int clientIdx = 0;
	do
	{
		clientIdx = (clientIdx == 0) ? 1 : 0;
		if (clientSocketProcessed[clientIdx]) continue;
		iResult = recv(clientSocket[clientIdx], recvbuf, recvbuflen, 0);
		if (iResult > 0)
		{
			printf("Bytes received: %d\n", iResult);
			printf("Received message: \"%s\"\n", recvbuf);
			sprintf(recvbuf, "to client #%d", clientIdx);
			// Echo the buffer back to the sender

			iSendResult = send(clientSocket[clientIdx], recvbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR)
			{
				printf("send failed: %d\n", WSAGetLastError());
				return 1;
			}
			printf("Bytes sent: %d\n", iSendResult);
			clientSocketProcessed[clientIdx] = true;
			if (!UnprocessedSocketsLeft()) return 0;
		}
		else if (iResult == 0)
		{
			printf("Connection closing...\n");
		}
		else
		{
			printf("recv failed: %d\n", WSAGetLastError());
			return 1;
		}
		//return 0;
	}
	while (iResult > 0);
}

int ReceiveMsgAlt()
{
	bool activeConnectionExists = false;

	struct timeval tv;
	fd_set readfds;

	tv.tv_sec = 5;
	tv.tv_usec = 500000;

	FD_ZERO(&readfds);
	for (int clientIdx = 0; clientIdx < MAX_CLIENTS; clientIdx++)
	{
		FD_SET(clientSocket[clientIdx], &readfds);
	}

	system("pause");
	select(MAX_CLIENTS, &readfds, NULL, NULL, &tv);

	printf("> readfds count = %d\n", readfds.fd_count);

	for (int clientIdx = 0; clientIdx < MAX_CLIENTS; clientIdx++)
	{
		if (FD_ISSET(clientSocket[clientIdx], &readfds))
		{
			FD_CLR(clientSocket[clientIdx], &readfds);
			activeConnectionExists = true;
			int iResult = recv(clientSocket[clientIdx], recvbuf, recvbuflen, 0);
			if (iResult > 0)
			{
				printf("Bytes received: %d\n", iResult);
				printf("Received message: \"%s\"\n", recvbuf);
				sprintf(recvbuf, "to client #%d", clientIdx);
				// Echo the buffer back to the sender

				int iSendResult = send(clientSocket[clientIdx], recvbuf, iResult, 0);
				if (iSendResult == SOCKET_ERROR)
				{
					printf("send failed: %d\n", WSAGetLastError());
					return 1;
				}
				printf("Bytes sent: %d\n", iSendResult);
			}
			else if (iResult == 0)
			{
				printf("Connection closing...\n");
			}
			else
			{
				printf("Socket #%d - recv failed: %d\n", clientSocket[clientIdx], WSAGetLastError());
				return 1;
			}
		}
	}
	
	if (activeConnectionExists) return 0;
	return 1;
}


//CLOSECONNECTION: shuts down the socket and closes any connection on it
void CloseConnection ()
{
    //Close the socket if it exists
    if (listenSocket)
        closesocket(listenSocket);
    WSACleanup(); //Clean up Winsock
}

int _tmain(int argc, _TCHAR* argv[])
{
	ListenOnPort(PORT);
	ReceiveMsgAlt();
	CloseConnection();
	system("pause");
	return 0;
}

