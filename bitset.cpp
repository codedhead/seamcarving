#include "bitset.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#define BITSET_UNIT_SIZE (sizeof(uchar)<<3)
bitset::bitset():bits_(0),bitcnt_(0),unitcnt_(0)
{
}
bitset::bitset(int sz):bits_(0),bitcnt_(0),unitcnt_(0)
{
	resize(sz);
}
bitset::~bitset()
{
	if(bits_) free(bits_);
}
bitset& bitset::operator=(const bitset& b)
{
	bitcnt_=b.bitcnt_;unitcnt_=b.unitcnt_;
	if(bits_) free(bits_);

	bits_=(uchar*)malloc(sizeof(uchar)*unitcnt_);
	memcpy(bits_,b.bits_,sizeof(uchar)*unitcnt_);
	return *this;
}
bool bitset::operator[](int pos)
{
	if(pos<0||pos>=bitcnt_) return true;

	int base=pos/BITSET_UNIT_SIZE;
	int rem=pos%BITSET_UNIT_SIZE;
	uchar bit=bits_[base];
	return bit&(1<<rem);
}
void bitset::reset()
{
	if(unitcnt_&&bits_)
	{
		memset(bits_,0,sizeof(uchar)*unitcnt_);
	}
}
void bitset::resize(int sz)
{
	bitcnt_=sz;
	if(bits_) free(bits_);bits_=0;

	unitcnt_=(bitcnt_+BITSET_UNIT_SIZE-1)/BITSET_UNIT_SIZE;
	if(unitcnt_)
	{
		bits_=(uchar*)malloc(sizeof(uchar)*unitcnt_);
		reset();
	}
}

void bitset::set(int pos)
{
	if(pos<0||pos>=bitcnt_) return;

	int base=pos/BITSET_UNIT_SIZE;
	int rem=pos%BITSET_UNIT_SIZE;
	bits_[base]|=(1<<rem);
}
void bitset::clear(int pos)
{
	if(pos<0||pos>=bitcnt_) return;

	int base=pos/BITSET_UNIT_SIZE;
	int rem=pos%BITSET_UNIT_SIZE;
	bits_[base]&=(uchar)~((uchar)(1<<rem));
}

void bitset::print(int nline)
{
	if(nline==-1)
		nline=bitcnt_;

	printf("\n");
	for(int i=0,bpos=0;i<unitcnt_;++i)
	{
		for(int j=0;j<8;++j,++bpos)
		{
			if(bpos&&bpos%nline==0) printf("\n");
			printf("%d",(bits_[i]&(1<<j))?1:0);
		}
	}
	printf("\n\n");
}