#ifndef _BITSET_H_
#define _BITSET_H_

typedef unsigned char uchar;

class bitset
{
public:
	bitset();
	bitset(int sz);
	~bitset();
	bool operator[](int pos);
	bitset& operator=(const bitset& b);
	void resize(int sz);
	void reset();

	void set(int pos);
	void clear(int pos);

	void print(int nline=-1);

	int bitcnt_; // how many bits
private:
	int unitcnt_; // how many uchars
	uchar* bits_;
};
#endif