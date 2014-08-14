#include "seamcarver.h"
#include "utils.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <QProgressDialog>

#include <iostream>
using namespace std;
using namespace cv;

SaliencySCarver::SaliencySCarver()
{

}
SaliencySCarver::~SaliencySCarver()
{

}

void SaliencySCarver::reset()
{
	SeamCarver::reset();

	Isaliency=Mat();
	//Ilab=Mat();
	Ienergy_working=Mat();
}

void SaliencySCarver::preprocess()
{
	SeamCarver::preprocess();

	saliency_map(Ioriginal,Isaliency);
	Isaliency.copyTo(Ienergy_working);

	//Ienergy_forward;
}

void SaliencySCarver::compute_energy(const Mat& I,bool is_vertical,Mat& Ienergy)
{
	Ienergy=is_vertical?Ienergy_working(Rect(0,0,I.cols,I.rows))
		:Ienergy_working(Rect(0,0,I.cols,I.rows)).t();

	namedWindow( "energy", CV_WINDOW_AUTOSIZE );
	imshow("energy",is_vertical?Ienergy:Ienergy.t());
}
void GradSaliencySCarver::compute_energy(const Mat& I,bool is_vertical,Mat& Ienergy)
{
	Mat Ig,Is;
	

	gradient_energe(is_vertical?I:I.t(),Ig);
	Is=is_vertical?Ienergy_working(Rect(0,0,I.cols,I.rows))
		:Ienergy_working(Rect(0,0,I.cols,I.rows)).t();


//#define ENERGY_MULTI

#ifdef ENERGY_MULTI

	Mat Ig_float,Is_float,Ienergy_float;
	Is.convertTo(Is_float,CV_32FC1);
	Ig.convertTo(Ig_float,CV_32FC1);

		
	multiply(Ig_float,Is_float,Ienergy_float);
	//convertScaleAbs(Ienergy_float,Ienergy);
	double min_val,max_val;
	minMaxLoc(Ienergy_float,&min_val,&max_val);
	Ienergy_float*=(255.f/max_val);


	Ienergy_float.convertTo(Ienergy,CV_8UC1);
#else

	addWeighted( Is, 0.4, Ig, 0.6, 0, Ienergy );
#endif

	

	

	namedWindow( "energy", CV_WINDOW_AUTOSIZE );
	imshow("energy",is_vertical?Ienergy:Ienergy.t());

}

void SaliencySCarver::remove_seam(Mat& I,bool is_vertical,int* seamindex)
{
	uchar* pI,*pEnergy;
	int bplI=I.step.buf[0],bpeI=I.step.buf[1],
		bplE=Ienergy_working.step.buf[0],bpeE=Ienergy_working.step.buf[1];
	int h=I.rows,w=I.cols;

	//ScopeTic scopetic("remove_seam");

//#define MEM_POS(y,x) ((bytes_per_line*(y)+(x)*bytes_per_ele))

	if(is_vertical)
	{
		for(int y=0,x=0;y<h;++y)
		{
			x=seamindex[y];
			assert(x<w);

			pI=I.data+ bplI*y+bpeI*x;
			pEnergy=Ienergy_working.data+ bplE*y+bpeE*x;
			memmove(pI,pI+bpeI,bpeI*(w-x-1));
			memmove(pEnergy,pEnergy+bpeE,bpeE*(w-x-1));
		}
	}
	else
	{
		// brute force approach
		for(int x=0,y=0;x<w;++x)
		{
			y=seamindex[x];
			for(int j=y;j<h-1;++j)
			{
				I.at<Vec3b>(j,x)=I.at<Vec3b>(j+1,x);
				Ienergy_working.at<uchar>(j,x)=Ienergy_working.at<uchar>(j+1,x);
			}
		}
		// todo: always maintain a transpose version of I?
	}
}

void SaliencySCarver::shrink(int w,int h)
{
	Isaliency.copyTo(Ienergy_working);
	SeamCarver::shrink(w,h);
}
