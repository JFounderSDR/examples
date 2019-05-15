#include "../include/common_utils.h"

unsigned int scaDebugLevel = 0;

void 
setCompDebugLevel(
	unsigned int level)
{
	scaDebugLevel = level;
}

double
setPrecision(
	int n)
{
    double sum = 1.0;
    for(int i = 1;i <= n;++i)
        sum = sum *10;
    return sum;
}

float
processDouble(
	float t,
	int n)
{
    int value = (int)(t * setPrecision(n));
    int temp = (int)(t * setPrecision(n+1))%10;
    if(temp >= 5)
    	value = value + 1;
    float dval = value/setPrecision(n);
    return dval;
}
