#ifndef _COMMONUTILS_H
#define _COMMONUTILS_H

#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

extern unsigned int scaDebugLevel;
extern void setCompDebugLevel(unsigned int level);

#define COMPDEBUG(level, title, debuginfo) \
	if(level <= scaDebugLevel)\
		std::cout << "PID:" << getpid() << "  " << #title << ":" << debuginfo << std::endl;
		
double
setPrecision(
	int n);

float
processDouble(
	float t,
	int n);
		
#endif	//__COMMONUTILS_H