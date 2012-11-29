#include <iostream>
#include "sockClient.h"

#define PORT		5555

using namespace std;

int main(int argc, char * argv[])
{
	executionResultEnum status;
	sockClient client(PORT, "127.0.0.1", status);
	system("pause");


}