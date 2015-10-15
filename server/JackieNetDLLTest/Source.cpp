#include "BasicTypes.h"
#include "GlobalFunctions.h"
int main(void)
{
	char ip[65];
	DomainNameToIP("ZMD-SERVER", ip);
	printf("(%s)", ip);
	return 0;
}