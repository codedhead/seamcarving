#ifndef _SEAMCARVER_H_
#define _SEAMCARVER_H_

#include <opencv2/core/core.hpp>
using namespace cv;

#include "bitset.h"

#define MIN_WIDTH 20
#define MIN_HEIGHT 20
#define ENERGY_INFINITY (1000000)

#define MATRIX_DP (true)
#define MATRIX_GREEDY (false)

#define SEAMC_TEST

class SeamCarver
{
public:
	SeamCarver();
	virtual ~SeamCarver();
	bool load(const Mat& I);

	
	virtual void reset();
	virtual void preprocess();
	virtual void compute_energy(const Mat& I,bool is_vertical,Mat& Ienergy);
	virtual void shrink(int w,int h);



	void matrix_method(bool dpg){dp_or_greedy=dpg;}

	Mat shrinked_image(){return Iworking(Rect(0,0,cur_width,cur_height));}
	inline uchar* image_ptr(){return Iworking.data;}
	// original size
	inline int image_width(){return Ioriginal.cols;}
	inline int image_height(){return Ioriginal.rows;}


	// input I is already the ROI region
	int find_seam(const Mat& I,bool is_vertical,int* seamindex);
	void show_seam(Mat& I,bool is_vertical,int* seamindex);

	void debug_show_seam(bool is_vertical,int cnt);
	
	virtual void remove_seam(Mat& I,bool is_vertical,int* seamindex);

	

#ifdef SEAMC_TEST
	void test_show_seam(bool is_vertical);
	void test_remove_seam(bool is_vertical);
	int test_width,test_height;
	int* test_seam_index;
#endif

	bool dp_or_greedy;

protected:
	Mat Ioriginal,Iworking,IenergyMask;
	int cur_width,cur_height;
	
	bitset transport_map; // 1 go up, 0 go left
};

class SaliencySCarver: public SeamCarver
{
public:
	SaliencySCarver();
	~SaliencySCarver();

	virtual void shrink(int w,int h);
	virtual void reset();
	virtual void preprocess();
	virtual void compute_energy(const Mat& I,bool is_vertical,Mat& Ienergy);
	virtual void remove_seam(Mat& I,bool is_vertical,int* seamindex);


protected:
	//bool do_foward;
	Mat Isaliency,Ienergy_working;
};

class GradSaliencySCarver: public SaliencySCarver
{
public:
	virtual void compute_energy(const Mat& I,bool is_vertical,Mat& Ienergy);
};

#endif