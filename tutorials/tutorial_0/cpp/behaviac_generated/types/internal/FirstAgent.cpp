#include "FirstAgent.h"

FirstAgent::FirstAgent()
{
	p1 = false;
	p2 = 0;
	p3 = 0;
}

FirstAgent::~FirstAgent()
{
}

bool FirstAgent::m2()
{
	printf("m2\n");

	return false;
}

void FirstAgent::SayHello()
{
	printf("\nHello Behaviac!\n\n");
}
