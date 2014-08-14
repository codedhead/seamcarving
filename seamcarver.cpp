
//#include "carve_gpu.h"
#include "seamcarver.h"
#include "timer.h"
#include "utils.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>


#include <QProgressDialog>

#include <omp.h>

#include <iostream>
using namespace std;
using namespace cv;



#define USE_OMP
//#define USE_BSGP

SeamCarver::SeamCarver():dp_or_greedy(MATRIX_DP)
{
#ifdef SEAMC_TEST
	test_seam_index=0;
	test_height=test_width=0;
#endif
}
SeamCarver::~SeamCarver()
{
#ifdef SEAMC_TEST
	if(test_seam_index) delete[] test_seam_index;
#endif
}

void SeamCarver::preprocess()
{	
	/*int tots=100;
	QProgressDialog progress("Saving images...", "Abort", 0, tots, NULL);
	progress.setWindowModality(Qt::WindowModal);
	for(int i=0;i<tots;++i)
	{
		progress.setValue(i);

	}
	progress.setValue(tots);*/


	// starting from the original image
	int orig_h=Ioriginal.rows,orig_w=Ioriginal.cols;

#define transport_map_(y,x) (transport_map[(x)+(y)*(orig_w)])
#define transport_map_set0(y,x) transport_map.clear((x)+(y)*(orig_w))
#define transport_map_set1(y,x) transport_map.set((x)+(y)*(orig_w))

	transport_map.resize(orig_h*orig_w+1);
	// compute T(0,?), only remove vertical seams
	for(int c=1;c<=orig_w-MIN_WIDTH;++c) // ver seams count
	{
		//transport_map_set0(0,c); // go left
	}
	// compute T(?:0), only remove horizontal seams
	for(int r=1;r<=orig_h-MIN_HEIGHT;++r) // hor seams count
	{
		transport_map_set1(r,0); // go up
	}

	// alternate order
	for(int r=1;r<=orig_h-MIN_HEIGHT;++r) // remove r hor seams
	{
		for(int c=1;c<=orig_w-MIN_WIDTH;++c) // remove c ver seams
		{
			if((r+c)%2)
				transport_map_set1(r,c);
		}
	}
	//transport_map.print(orig_w);
	return;

/*

	//------- transport map
	int* Tmap=new int[(orig_h-MIN_HEIGHT)*(orig_w-MIN_WIDTH)];
#define Tmap_(y,x) (Tmap[(x)+(y)*(orig_w+1)])

	int *vseams=new int[orig_w*orig_h],
		*hseams=new int[orig_h*orig_w],
		*temp_seam=new int[max(orig_h,orig_w)];

#define vseams_(i) (vseams+(i)*orig_h)
#define hseams_(i) (hseams+(i)*orig_w)

	int *last_vseam,*last_hseam;

	Mat Itemp,Iroi;

	Ioriginal.copyTo(Itemp);
	Iroi=Itemp;
	// compute T(0,?), only remove vertical seams
	for(int c=1;c<=orig_w-MIN_WIDTH;++c) // ver seams count
	{
		transport_map_set0(0,c); // go left

		Tmap_(0,c)=Tmap_(0,c-1)+find_seam(Iroi,true,vseams_(c));
		remove_seam(Iroi,true,vseams_(c));

		// next roi
		Iroi=Itemp(Rect(0,0,orig_w-c,orig_h));
	}

	Ioriginal.copyTo(Itemp);
	Iroi=Itemp;
	// compute T(?:0), only remove horizontal seams
	for(int r=1;r<=orig_h-MIN_HEIGHT;++r) // hor seams count
	{
		transport_map_set1(r,0); // go up

		Tmap_(r,0)=Tmap_(r-1,0)+find_seam(Iroi,false,hseams_(r));
		remove_seam(Iroi,false,hseams_(r));

		// next roi
		Iroi=Itemp(Rect(0,0,orig_w,orig_h-r));
	}

	float Eleft,Eabove;
	for(int r=1;r<=orig_h-MIN_HEIGHT;++r) // remove r hor seams
	{
		for(int c=1;c<=orig_w-MIN_WIDTH;++c) // remove c ver seams
		{
			Eleft=Tmap_(r,c-1)+vseams_(c-1);
			Eabove=Tmap_(r-1,c)+Tmap_hseam(r-1,c);

			if(Eleft<Eabove)
			{
				transport_map;
			}
			else
			{
				transport_map;
			}

		}
	}


	delete [] Tmap;
	delete [] hseams;
	delete [] vseams;
	delete [] temp_seam;
#undef Tmap_*/
}

bool SeamCarver::load(const Mat& Iorg)
{
	bool loaded=false;

	// already loaded
	if(Ioriginal.data)
	{
		loaded=true;
	}

	Ioriginal=Iorg; // shallow copy
	Ioriginal.copyTo(Iworking);
	//IenergyMask.setTo(0);
	
#ifdef SEAMC_TEST
	test_height=Ioriginal.rows;
	test_width=Ioriginal.cols;
	if(!loaded)
		test_seam_index=new int[max(test_height,test_width)];
#endif

	cur_height=Ioriginal.rows;
	cur_width=Ioriginal.cols;

	if(!loaded)
	{
		preprocess();
	}
	
	return true;
}
void SeamCarver::reset()
{
// 	if(Ioriginal.data)
// 	{
// 		Ioriginal.copyTo(Iworking);
// 		test_height=Ioriginal.rows;
// 		test_width=Ioriginal.cols;
// 	}
	Ioriginal=Mat();
	Iworking=Mat();
	IenergyMask=Mat();
	transport_map.reset();
	
#ifdef SEAMC_TEST
	test_height=test_width=0;
	if(test_seam_index){delete[] test_seam_index;test_seam_index=0;}
#endif
}

inline int min3i(int a,int b,int c,int* mini)
{
// 	return a<b?
// 		(a<c?*minval=a,0:*minval=c,2):
// 		(b<c?*minval=b,1:*minval=c,2);
	if(a<b)
	{
		if(a<c)
		{
			*mini=0;
			return a;
		}
		else
		{
			*mini=2;
			return c;
		}
	}
	else
	{
		if(b<c)
		{
			*mini=1;
			return b;
		}
		else
		{
			*mini=2;
			return c;
		}
	}
}



// x is offset by 1 (left border)
#define cost_matrix_(y,x) (cost_matrix[(x+1)+(y)*(w+2)])
#define indices_(y,x) (indices[(x)+(y)*w])
#define sources_(y,x) (sources[(x)+(y)*w])

void print_cost_matrix(int* cost_matrix,int w,int h)
{
	//return;
	for(int y=0;y<h;++y)
	{
		printf("%d:  ",y);
		for(int x=0;x<w;++x)
		{
			printf("%4d",cost_matrix_(y,x));
		}
		printf("\n");
	}
	printf("\n");
}
void print_indices(int* indices,int w,int h)
{
	//return;
	for(int y=0;y<h;++y)
	{
		printf("%d:  ",y);
		for(int x=0;x<w;++x)
		{
			printf("%4d",indices_(y,x));
		}
		printf("\n");
	}
	printf("\n");
}

int compute_cost_matrix_DP(int* cost_matrix,int* indices,int w,int h,const Mat& Ienergy)
{
	//ScopeTic scopetic_dp("find_seam: dp");
	
	// fill the first row
	for(int i=0;i<w;++i)
		cost_matrix_(0,i)=Ienergy.at<uchar>(0,i);

#ifdef USE_OMP

	for(int y=1;y<h;++y) // start from second row
	{
		#pragma omp parallel for
		for(int x=0;x<w;++x)
		{
			int minE,mini;

			minE=min3i(
				cost_matrix_(y-1,x-1),
				cost_matrix_(y-1,x),
				cost_matrix_(y-1,x+1),
				&mini);

			indices_(y,x)=x+mini-1;
			cost_matrix_(y,x)=Ienergy.at<uchar>(y,x)+minE;
		}
	}
#else
	for(int y=1;y<h;++y) // start from second row
	{
		int minE,mini;
		for(int x=0;x<w;++x)
		{
			minE=min3i(
				cost_matrix_(y-1,x-1),
				cost_matrix_(y-1,x),
				cost_matrix_(y-1,x+1),
				&mini);

			indices_(y,x)=x+mini-1;
			cost_matrix_(y,x)=Ienergy.at<uchar>(y,x)+minE;
		}
	}

#endif

	//------- the endpoint of minimum seam
	int mini=0;
	int minE=cost_matrix_(h-1,0); // first col in last row
	for(int x=1;x<w;++x)
	{
		if(cost_matrix_(h-1,x)<minE)
		{
			minE=cost_matrix_(h-1,x);
			mini=x;
		}
	}

//	printf("DP minE: %d\n",minE);
// 	printf("DP matrix:\n");
// 	print_cost_matrix(cost_matrix,w,h);

	return mini;
}

int compute_cost_matrix_greedy(int* cost_matrix,int* indices,int w,int h,const Mat& Ienergy)
{
	//ScopeTic scopetic_dp("find_seam: greedy");
	int minE,mini;


/*
#ifdef USE_BSGP
	for(int y=0;y<h;++y)
	{
		for(int x=0;x<w;++x)
		{
			cost_matrix_(y,x)=Ienergy.at<uchar>(y,x);
		}
	}

	{
		ScopeTic scopetic_dp("find_seam: bsgp");
		compute_cost_matrix_greedy_bsgp(cost_matrix,indices,w,h);
	}
#else*/
	int * sources=new int[w*h];
	memset(indices,-1,sizeof(int)*w*h);

#ifdef USE_OMP
	
	#pragma omp parallel for
	for(int px=0;px<h*w;++px)
	{
		int x=px%w,y=px/w;
		cost_matrix_(y,x)=Ienergy.at<uchar>(y,x);
		sources_(y,x)=x; // itself
	}

	for(int part_len=2;part_len/2<h;(part_len<<=1))
	{
		int parts_num=(h+part_len-1)/part_len; // divide into

		#pragma omp parallel for
		for(int px=0;px<parts_num*w;++px)
		{
			int minE,mini;
			int srcx;
			int part=px/w;
			int x=px%w;
			int second_y=((part+1)*part_len-1);
			int first_y=(second_y-part_len/2);

			if(second_y>=h) second_y=h-1;

			if(second_y>first_y)
			{
				
				srcx=sources_(second_y,x);
				minE=min3i(
					cost_matrix_(first_y,srcx-1),
					cost_matrix_(first_y,srcx),
					cost_matrix_(first_y,srcx+1),
					&mini);

				cost_matrix_(second_y,x)+=minE;

				indices_(first_y+1,srcx)=srcx+mini-1; // link it
				sources_(second_y,x)=sources_(first_y,srcx+mini-1); // update source
			}
		}
	}

#else

	for(int y=0;y<h;++y)
	{
		for(int x=0;x<w;++x)
		{
			cost_matrix_(y,x)=Ienergy.at<uchar>(y,x);
			sources_(y,x)=x; // itself
		}
	}

	int srcx;
	int first_y,second_y;
	for(int part_len=2;part_len/2<h;(part_len<<=1))
	{
		int parts_num=(h+part_len-1)/part_len; // divide into

		for(int part=0;part<parts_num;++part)
		{
			second_y=((part+1)*part_len-1);
			first_y=(second_y-part_len/2);
			if(second_y>=h) second_y=h-1;

			if(second_y>first_y)
			{
				for(int x=0;x<w;++x)
				{
					srcx=sources_(second_y,x);
					minE=min3i(
						cost_matrix_(first_y,srcx-1),
						cost_matrix_(first_y,srcx),
						cost_matrix_(first_y,srcx+1),
						&mini);

					cost_matrix_(second_y,x)+=minE;
					
					indices_(first_y+1,srcx)=srcx+mini-1; // link it
					sources_(second_y,x)=sources_(first_y,srcx+mini-1); // update source
				}
			}
		}
	}


#endif

	delete[] sources;

	//------- the endpoint of minimum seam
	mini=-1;
	minE=ENERGY_INFINITY;
	//minE=cost_matrix_(h-1,0); // first col in last row
	for(int x=0;x<w;++x)
	{
		if(/*sources_(h-1,x)!=-1&&*/cost_matrix_(h-1,x)<minE)
		{
			minE=cost_matrix_(h-1,x);
			mini=x;
		}
	}

//	printf("greedy minE: %d\n",minE);
// 	printf("greedy matrix:\n");
// 	print_cost_matrix(cost_matrix,w,h);
// 	printf("greedy indices:\n");
// 	print_indices(indices,w,h);
// 	printf("greedy sources:\n");
// 	print_indices(sources,w,h);

	

	return mini;
}

inline bool is_powerof2(int x)
{
	return x&&((x&(x-1))==0);
}

/*int pdp(int* cost_matrix,int* indices,int* sources,int w,int h,bool is_down)
{
	int mini,minE;
	
	if(h==2) // trivial case
	{
		if(is_down)
		{
			for(int x=0;x<w;++x)
			{
				cost_matrix_(1,x)+=min3i(
					cost_matrix_(0,x-1),
					cost_matrix_(0,x),
					cost_matrix_(0,x+1),
					&mini);

				sources_(1,x)=indices_(1,x)=x+mini-1; // backtrack
			}
		}
		else
		{
			for(int x=0;x<w;++x)
			{
				cost_matrix_(0,x)+=min3i(
					cost_matrix_(1,x-1),
					cost_matrix_(1,x),
					cost_matrix_(1,x+1),
					&mini);

				sources_(0,x)=indices_(0,x)=x+mini-1;
			}
		}
	}
	else
	{
		assert(h>2&&is_powerof2(h));

		pdp(cost_matrix,indices,sources,w,h/2,true);
		pdp(&cost_matrix_(h/2,-1),&indices_(h/2,0),&sources_(h/2,0),w,h/2,false);
		
		int dstx,dsty,
			y_samedir,y_revdir,
			y_rev_low,y_rev_high;

		if(is_down)
		{
			dsty=h-1;
			y_samedir=h/2-1;
			y_revdir=h/2;
			
			y_rev_low=h/2;
			y_rev_high=h;
		}
		else
		{
			dsty=0;
			y_samedir=h/2;
			y_revdir=h/2-1;

			y_rev_low=0;
			y_rev_high=h/2;
		}

		
		// fill big number
		for(int x=0;x<w;++x) // (dsty,x)
		{
			cost_matrix_(dsty,x)=ENERGY_INFINITY;
			sources_(dsty,x)=-1;
		}

		// get min energy
		for(int x=0;x<w;++x) // (y_revdir,x)
		{
			dstx=sources_(y_revdir,x);
			if(dstx==-1) continue;

			minE=min3i(
				cost_matrix_(y_samedir,x-1),
				cost_matrix_(y_samedir,x),
				cost_matrix_(y_samedir,x+1),
				&mini);

			// invalid
			if(sources_(y_samedir,x+mini-1)==-1)
				continue;

			minE+=cost_matrix_(y_revdir,x);
			if(minE<cost_matrix_(dsty,dstx))
			{
				cost_matrix_(dsty,dstx)=minE;
				// some fake information
				sources_(dsty,dstx)=x+mini-1; // same_dir
				indices_(dsty,dstx)=x; // rev_dir
			}
		}

		if(is_down)
		{

			for(int x=0;x<w;++x) // (dsty,x)
			{
				if(sources_(dsty,x)!=-1)
				{
					int save_prev=sources_(dsty,x);
					int head=indices_(dsty,x),next,prev=save_prev;

					// reverse indices of [h/2,h)
					for(int y=h/2;y<h;++y)
					{
						next=indices_(y,head);

						indices_(y,head)=prev;
						prev=head;
						head=next;
					}

					// correct the source
					sources_(dsty,x)=sources_(y_samedir,save_prev);
				}
			}

		}
		else
		{

			for(int x=0;x<w;++x) // (dsty,x)
			{
				if(sources_(dsty,x)!=-1)
				{
					int save_prev=sources_(dsty,x);
					int head=indices_(dsty,x),next,prev=save_prev;

					// reverse indices of [h/2-1,0]
					for(int y=h/2-1;y>=0;--y)
					{
						next=indices_(y,head);

						indices_(y,head)=prev;
						prev=head;
						head=next;
					}

					// correct the source
					sources_(dsty,x)=sources_(y_samedir,save_prev);
				}
			}

		}
	}

	return mini;
}

int compute_cost_matrix_parallel(int* cost_matrix,int* indices,int w,int h,const Mat& Ienergy)
{
	ScopeTic scopetic_dp("find_seam: parallel");
	int minE,mini;

	assert(is_powerof2(w)&&is_powerof2(h));

	memset(indices,-1,sizeof(int)*w*h);

	for(int y=0;y<h;++y)
	{
		for(int x=0;x<w;++x)
		{
			cost_matrix_(y,x)=Ienergy.at<uchar>(y,x);
		}
	}

	printf("energy matrix:\n");
	print_cost_matrix(cost_matrix,w,h);

	int * sources=new int[w*h];

	pdp(cost_matrix,indices,sources,w,h,true);

	
	
	//------- the endpoint of minimum seam
	mini=-1;
	minE=ENERGY_INFINITY;//cost_matrix_(h-1,0); // first col in last row
	for(int x=0;x<w;++x)
	{
		if(sources_(h-1,x)!=-1&&cost_matrix_(h-1,x)<minE)
		{
			minE=cost_matrix_(h-1,x);
			mini=x;
		}
	}

	printf("parallel minE: %d\n",minE);
	printf("parallel matrix:\n");
	print_cost_matrix(cost_matrix,w,h);

	delete[] sources;

	return mini;
}
*/

void SeamCarver::compute_energy(const Mat& I,bool is_vertical,Mat& Ienergy)
{
	gradient_energe(is_vertical?I:I.t(),Ienergy);

	namedWindow( "energy", CV_WINDOW_AUTOSIZE );
	imshow("energy",is_vertical?Ienergy:Ienergy.t());
}
int SeamCarver::find_seam(const Mat& I,bool is_vertical,int* seamindex)
{
	//ScopeTic scopetic("find_seam");
	
	//------- compute energy (gradients)
	Mat Ienergy; // U8!!!
	{
		//ScopeTic scopetic_energy("find_seam: energy");
		// setting the roi, if horizontal, do a transpose, so it can be handled as vertical case
		compute_energy(I,is_vertical,Ienergy);
	}
	
	int h=Ienergy.rows,
		w=Ienergy.cols;

	//------- MCE matrix
	// pack the matrix with borders, this would simplify our code
	int* cost_matrix=new int[h*(w+2)];
	int* indices=new int[h*w];

	// init matrix
	// guard with the left right borders
	for(int i=0;i<h;++i)
	{
		cost_matrix_(i,-1)=cost_matrix_(i,w)=ENERGY_INFINITY;
	}

	int sx=(dp_or_greedy==MATRIX_DP)?
		compute_cost_matrix_DP(cost_matrix,indices,w,h,Ienergy)
		:compute_cost_matrix_greedy(cost_matrix,indices,w,h,Ienergy);

// 	for(int y=h-1;y>=0;--y)
// 	{
// 		seamindex[y]=0;
// 	}

	assert(sx!=-1);
	//------- backtrack
	int idx=0;
	for(int y=h-1;y>=1;--y)
	{
		seamindex[y]=sx;

		idx=indices_(y,sx);
		/*assert(idx>=0&&idx<=2);*/

		//sx+=idx-1; // 0 for x-1, 1 for x, 2 for x+1
		sx=idx;
	}
	seamindex[0]=sx;

	delete [] cost_matrix;
	delete [] indices;

	return 0/*minE*/;
// #undef cost_matrix_
// #undef indices_
}

void SeamCarver::debug_show_seam(bool is_vertical,int cnt)
{
	Mat I;
	Ioriginal.copyTo(I);
	Ioriginal.copyTo(Iworking);

	int w=Ioriginal.cols,h=Ioriginal.rows;
	int *theseam=new int[max(w,h)];

	if(is_vertical)
	{
		for(int i=0;i<cnt;++i)
		{
			find_seam(I(Rect(0,0,w-i,h)),true,theseam);
			remove_seam(I,true,theseam);

			show_seam(Iworking,true,theseam);
		}
	}
	else
	{
		for(int i=0;i<cnt;++i)
		{
			find_seam(I(Rect(0,0,w,h-i)),false,theseam);
			remove_seam(I,false,theseam);

			show_seam(Iworking,false,theseam);
		}
	}
	
	delete[] theseam;
}

void SeamCarver::remove_seam(Mat& I,bool is_vertical,int* seamindex)
{
	uchar* p;
	int bytes_per_line=I.step.buf[0],bytes_per_ele=I.step.buf[1];
	int h=I.rows,w=I.cols;

	//ScopeTic scopetic("remove_seam");

#define MEM_POS(y,x) ((bytes_per_line*(y)+(x)*bytes_per_ele))

	if(is_vertical)
	{
		for(int y=0,x=0;y<h;++y)
		{
			x=seamindex[y];
			assert(x<w);

			p=I.data+MEM_POS(y,x);
			memmove(p,p+bytes_per_ele,bytes_per_ele*(w-x-1));
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
			}
		}
		// todo: always maintain a transpose version of I?
	}
#undef MEM_POS
}

void SeamCarver::show_seam(Mat& I,bool is_vertical,int* seamindex)
{
	Vec3b seam_color(0,0,255);
	if(is_vertical)
	{
		for(int y=0,h=I.rows;y<h;++y)
		{
			I.at<Vec3b>(y,seamindex[y])=seam_color;
		}
	}
	else
	{
		for(int x=0,w=I.cols;x<w;++x)
		{
			I.at<Vec3b>(seamindex[x],x)=seam_color;
		}
	}
}
/*
template <class EleType>
void remove_seam(Mat& I,bool is_vertical,int* seamindex)
{
	uchar* p;
	int bytes_per_line=I.step.buf[0],bytes_per_ele=I.step.buf[1];
	int h=I.rows,w=I.cols;
	
	ScopeTic scopetic("remove_seam");

#define MEM_POS(y,x) ((bytes_per_line*(y)+(x)*bytes_per_ele))

	if(is_vertical)
	{
		for(int y=0,x=0;y<h;++y)
		{
			x=seamindex[y];
			assert(x<w);

			p=I.data+MEM_POS(y,x);
			memmove(p,p+bytes_per_ele,bytes_per_ele*(w-x-1));
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
				I.at<EleType>(j,x)=I.at<EleType>(j+1,x);
			}
		}
		// todo: always maintain a transpose version of I?
	}
}
*/
void SeamCarver::shrink(int dstw,int dsth)
{
	int orig_h=Ioriginal.rows,orig_w=Ioriginal.cols;
	if(dstw>orig_w||dsth>orig_h||dstw<MIN_WIDTH||dsth<MIN_HEIGHT) return;
	int r=orig_h-dsth,c=orig_w-dstw;
	if(r==0&&c==0) return;

	// reset Iworking
	Ioriginal.copyTo(Iworking);
	
	int path_len=r+c;
	int* path=new int[path_len+4];

	int k=path_len-1; // store in previous step
	while(k>=0) 
	{
		if(transport_map_(r,c)) // go up
		{
			--r;
			path[k--]=1;
		}
		else
		{
			--c;
			path[k--]=0;
		}
	}
	
	int* vseam=new int[orig_h],
		*hseam=new int[orig_w];

	

	int roi_w=orig_w,roi_h=orig_h;
	Mat Iroi;
	for(int i=0;i<path_len;++i)
	{
		Iroi=Iworking(Rect(0,0,roi_w,roi_h));
		if(path[i]==0) // go left right
		{
			find_seam(Iroi,true,vseam);
			remove_seam(Iroi,true,vseam);

			--roi_w;
		}
		else // go up down
		{
			find_seam(Iroi,false,hseam);
			remove_seam(Iroi,false,hseam);

			--roi_h;
		}

		printf("*");
	}
	printf("\n");
	delete[] vseam;
	delete[] hseam;

	cur_width=dstw;
	cur_height=dsth;
}

#ifdef SEAMC_TEST
void SeamCarver::test_show_seam(bool is_vertical)
{
	if(!Iworking.data) return;

	Mat Iroi=Iworking(Rect(0,0,test_width,test_height));

	find_seam(Iroi,is_vertical,test_seam_index);
	show_seam(Iroi,is_vertical,test_seam_index);
}

void SeamCarver::test_remove_seam(bool is_vertical)
{
	if(!Iworking.data) return;

	Mat Iroi=Iworking(Rect(0,0,test_width,test_height));

	// assume test_seam_index already computed in show_seam
	if(is_vertical)
	{
		remove_seam(Iroi,is_vertical,test_seam_index);
		--test_width;
	}
	else
	{
		remove_seam(Iroi,is_vertical,test_seam_index);
		--test_height;
	}
}
#endif