#include "BasicTypes.h"
#include "GlobalFunctions.h"
#include "NetTypes.h"
#include "JackieINetSocket.h"
int main(void)
{
	char ip[65];
	DomainNameToIP("ZMD-SERVER", ip);
	printf("(%s)", ip);
	return 0;
}