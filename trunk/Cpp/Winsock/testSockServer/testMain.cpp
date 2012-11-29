#include <iostream>
#include "sockServer.h"

#define PORT		5555

using namespace std;

int main(int argc, char * argv[])
{
	executionResultEnum status;
	sockServer server(PORT, status);
	server.listenForConnections(2);
	system("pause");
}