#pragma once

#include <iostream>
#include <winsock2.h>

#include "executionResult.h"

// Compile using ws2_32.lib

#define MAX_CLIENTS		2
#define MAX_STRLEN		256

class sockServer
{
private:
	int listenPort;
	SOCKET listenSocket;
	SOCKET * clientSocket;

	char hostName[MAX_STRLEN];
	
	void AddressToString(struct sockaddr addr, char *result);
public:
	sockServer(int port, executionResultEnum &status);
	~sockServer(void);
	executionResultEnum listenForConnections(int maxClients);
};
