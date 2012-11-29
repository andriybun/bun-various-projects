#pragma once

#include <iostream>
#include <winsock2.h>

#include "executionResult.h"

// Compile using ws2_32.lib

#define MAX_STRLEN		256

class sockClient
{
private:
	SOCKET serverSocket;
	
	char hostName[MAX_STRLEN];
public:
	sockClient(int port, const char * ipAddress, executionResultEnum &status);
	~sockClient(void);
};
