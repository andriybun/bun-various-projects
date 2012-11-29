#include "sockClient.h"

sockClient::sockClient(int port, const char * ipAddress, executionResultEnum &status)
{
	WSADATA wsaInfo;
	int error = WSAStartup(0x0202, &wsaInfo);

	if (error)
	{
		status = EXECUTION_RESULT_FAILED_TO_START_WINSOCK;
	}

	if (wsaInfo.wVersion != 0x0202)
	{
		WSACleanup();
		status = EXECUTION_RESULT_WRONG_WINSOCK_VERSION;
	}

	SOCKADDR_IN target;
	target.sin_family = AF_INET;
	target.sin_port = htons(port);
	target.sin_addr.s_addr = inet_addr(ipAddress);

	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET)
	{
		status = EXECUTION_RESULT_INVALID_SOCKET;
	}  

	gethostname(hostName, MAX_STRLEN);

	if (connect(serverSocket, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR)
	{
		status = EXECUTION_RESULT_CONNECTION_TO_SERVER_FAILED;
	}
	else
	{
		printf("Client %s:\tConnected successfully!\n", hostName);
		status = EXECUTION_RESULT_OK;
	}
}

sockClient::~sockClient(void)
{
	if (serverSocket)
	{
		closesocket(serverSocket);
	}
	WSACleanup();

}
