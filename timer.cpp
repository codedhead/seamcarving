#include "timer.h"

#include <stdio.h>
#include <windows.h>

static LARGE_INTEGER freq,pc1={0},pc2={0};

void tic()
{
	static int _dummy_freq=QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&pc1);
}
void toc()
{
	QueryPerformanceCounter(&pc2);
	double timecnt=((double)(pc2.QuadPart-pc1.QuadPart))/((double)freq.QuadPart);
	printf("Time elapsed %f ms\n",1000.*timecnt);
}

ScopeTic::ScopeTic(const char* t)
{
	static int _dummy_freq=QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&this->pc1);

	if(t) strcpy(text,t);
	else text[0]=0;
}
ScopeTic::~ScopeTic()
{
	QueryPerformanceCounter(&this->pc2);
	double timecnt=((double)(this->pc2.QuadPart-this->pc1.QuadPart))/((double)freq.QuadPart);
	if(text[0])
		printf("[%s] Time elapsed %f ms\n",text,1000.*timecnt);
	else
		printf("Time elapsed %f ms\n",1000.*timecnt);
}