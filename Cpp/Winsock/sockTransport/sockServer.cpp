#include "sockServer.h"

sockServer::sockServer(int port, executionResultEnum &status)
{
	WSADATA wsaInfo;
	int error = WSAStartup(0x0202, &wsaInfo);

	if (error)
	{
		status = EXECUTION_RESULT_FAILED_TO_START_WINSOCK;
	}

	if (wsaInfo.wVersion != 0x0202)
	{
		WSACleanup ();
		status = EXECUTION_RESULT_WRONG_WINSOCK_VERSION;
	}

	SOCKADDR_IN addr;

	addr.sin_family = AF_INET;
	listenPort = port;
	addr.sin_port = htons(listenPort);

	//Accept a connection from any IP using INADDR_ANY. You could pass inet_addr("0.0.0.0") instead to accomplish the 
	//same thing. If you want only to watch for a connection from a specific IP, specify that //instead.
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		//socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (listenSocket == INVALID_SOCKET)
	{
		status = EXECUTION_RESULT_INVALID_SOCKET;
	}

	if (bind(listenSocket, (LPSOCKADDR)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		//We couldn't bind (this will happen if you try to bind to the same socket more than once)
		status = EXECUTION_RESULT_BINDING_ERROR;
	}

	gethostname(hostName, MAX_STRLEN);
	printf("Host: %s\n", hostName);

	status = EXECUTION_RESULT_OK;
}

sockServer::~sockServer(void)
{
	if (listenSocket)
	{
		closesocket(listenSocket);
		// TODO: do I need to close connection sockets?
	}
	WSACleanup();
}

executionResultEnum sockServer::listenForConnections(int maxClients)
{
	//Now we can start listening (allowing as many connections as possible to  
	//be made at the same time using SOMAXCONN). You could specify any 
	//integer value equal to or lesser than SOMAXCONN instead for custom 
	//purposes). The function will not //return until a connection request is 
	//made
	int listenResult = listen(listenSocket, SOMAXCONN);

	clientSocket = new SOCKET[MAX_CLIENTS];

	printf("%s:\tListening to port #%d\n", hostName, listenPort);

	for (int clientIdx = 0; clientIdx < MAX_CLIENTS; clientIdx++)
	{
		printf("%s:\tWaiting for client #%d to connect...\n", hostName, clientIdx+1);

		// Accept the connection.
		struct sockaddr clientAddr;
		int clientAddrLen;
		clientSocket[clientIdx] = WSAAccept(listenSocket, &clientAddr, &clientAddrLen, NULL, NULL);
		//clientSocket[clientIdx] = accept(listenSocket, &clientAddr, NULL);

		if (clientSocket[clientIdx] == INVALID_SOCKET)
		{
			printf("%s:\taccept failed with error: %ld\n", hostName, WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return EXECUTION_RESULT_FAILED_TO_ACCEPT_CONNECTION;
		}
		else
		{
			char ipAddrStr[16];
			AddressToString(clientAddr, ipAddrStr);
			//getpeername(clientSocket[clientIdx], &clientAddr, &clientAddrLen);
			printf("%s - connected\n", ipAddrStr);
		}
	}

	delete [] clientSocket;

	return EXECUTION_RESULT_OK;
}

void sockServer::AddressToString(struct sockaddr addr, char *result)
{
	unsigned char d1 = (unsigned char)addr.sa_data[2];
	unsigned char d2 = (unsigned char)addr.sa_data[3];
	unsigned char d3 = (unsigned char)addr.sa_data[4];
	unsigned char d4 = (unsigned char)addr.sa_data[5];
	sprintf(result, "%d.%d.%d.%d", d1, d2, d3, d4);
}
