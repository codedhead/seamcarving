#ifndef _TIMER_H_
#define _TIMER_H_

#include <windows.h>

void tic();
void toc();

class ScopeTic
{
public:
	ScopeTic(const char* t=0);
	~ScopeTic();

	LARGE_INTEGER pc1,pc2;
	char text[128];
};

#endif