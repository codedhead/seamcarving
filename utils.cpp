#include "utils.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;


void gradient_energe(const Mat& I,Mat& Ienergy)
{
	Mat Igray;
	cvtColor(I,Igray,CV_RGB2GRAY);

	/// Generate grad_x and grad_y
	Mat grad_x, grad_y;
	Mat abs_grad_x, abs_grad_y;

	int ddepth = CV_16S;

	//Scharr( src_gray, grad_x, ddepth, 1, 0, scale, delta, BORDER_DEFAULT );
	Sobel( Igray, grad_x, ddepth, 1, 0 );
	convertScaleAbs( grad_x, abs_grad_x ); // CV_8U
	//Scharr( src_gray, grad_y, ddepth, 0, 1, scale, delta, BORDER_DEFAULT );
	Sobel( Igray, grad_y, ddepth, 0, 1 );
	convertScaleAbs( grad_y, abs_grad_y ); // CV_8U

	addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, Ienergy );
}
/*
void sobel_energe_gpu(const Mat& I_cpu,Mat& Ienergy_cpu,bool dotranspose)
{
	gpu::GpuMat I,Igray,Ienergy;
	if(dotranspose)
		I.upload(I_cpu.t());
	else
		I.upload(I_cpu);

	gpu::cvtColor(I,Igray,CV_RGB2GRAY);

	/// Generate grad_x and grad_y
	gpu::GpuMat grad_x, grad_y,abs_grad_x, abs_grad_y;

	int ddepth = CV_16S;

	//Scharr( src_gray, grad_x, ddepth, 1, 0, scale, delta, BORDER_DEFAULT );
	gpu::Sobel( Igray, grad_x, ddepth, 1, 0 );
	//gpu::convertScaleAbs( grad_x, abs_grad_x ); // CV_8U
	//Scharr( src_gray, grad_y, ddepth, 0, 1, scale, delta, BORDER_DEFAULT );
	gpu::Sobel( Igray, grad_y, ddepth, 0, 1 );
	//gpu::convertScaleAbs( grad_y, abs_grad_y ); // CV_8U

	//gpu::addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, Ienergy );
	gpu::addWeighted( grad_x, 0.5, grad_y, 0.5, 0, Ienergy );

	Ienergy.download(Ienergy_cpu);

	//namedWindow( "energymap", CV_WINDOW_AUTOSIZE );// Create a window for display.
	//imshow( "energymap", Ienergy );                   // Show our image inside it.
}*/

void bgr2lab(const Mat& I,Mat& chnl_l,Mat& chnl_a,Mat& chnl_b)
{
	int w=I.cols,h=I.rows;
	chnl_l.create(h,w,CV_64FC1);
	chnl_a.create(h,w,CV_64FC1);
	chnl_b.create(h,w,CV_64FC1);

	for(int i=0;i<h;++i)
	{
		for(int j=0;j<w;++j)
		{
			Vec3b sBGR=I.at<Vec3b>(i,j);
			//------------------------
			// sRGB to XYZ conversion
			// (D65 illuminant assumption)
			//------------------------
			double R = sBGR[2]/255.0;
			double G = sBGR[1]/255.0;
			double B = sBGR[0]/255.0;

			double r, g, b;

			if(R <= 0.04045)	r = R/12.92;
			else				r = pow((R+0.055)/1.055,2.4);
			if(G <= 0.04045)	g = G/12.92;
			else				g = pow((G+0.055)/1.055,2.4);
			if(B <= 0.04045)	b = B/12.92;
			else				b = pow((B+0.055)/1.055,2.4);

			double X = r*0.4124564 + g*0.3575761 + b*0.1804375;
			double Y = r*0.2126729 + g*0.7151522 + b*0.0721750;
			double Z = r*0.0193339 + g*0.1191920 + b*0.9503041;
			//------------------------
			// XYZ to LAB conversion
			//------------------------
			double epsilon = 0.008856;	//actual CIE standard
			double kappa   = 903.3;		//actual CIE standard

			double Xr = 0.950456;	//reference white
			double Yr = 1.0;		//reference white
			double Zr = 1.088754;	//reference white

			double xr = X/Xr;
			double yr = Y/Yr;
			double zr = Z/Zr;

			double fx, fy, fz;
			if(xr > epsilon)	fx = pow(xr, 1.0/3.0);
			else				fx = (kappa*xr + 16.0)/116.0;
			if(yr > epsilon)	fy = pow(yr, 1.0/3.0);
			else				fy = (kappa*yr + 16.0)/116.0;
			if(zr > epsilon)	fz = pow(zr, 1.0/3.0);
			else				fz = (kappa*zr + 16.0)/116.0;

			chnl_l.at<double>(i,j)=116.0*fy-16.0;
			chnl_a.at<double>(i,j)=500.0*(fx-fy);
			chnl_b.at<double>(i,j)=200.0*(fy-fz);
		}
	}
}

void bgr2lab(const Mat& I,Mat& lab)
{
	int w=I.cols,h=I.rows;
	lab.create(h,w,CV_64FC3);

	for(int i=0;i<h;++i)
	{
		for(int j=0;j<w;++j)
		{
			Vec3b sRGB=I.at<Vec3b>(i,j);
			//------------------------
			// sRGB to XYZ conversion
			// (D65 illuminant assumption)
			//------------------------
			double R = sRGB[2]/255.0;
			double G = sRGB[1]/255.0;
			double B = sRGB[0]/255.0;

			double r, g, b;

			if(R <= 0.04045)	r = R/12.92;
			else				r = pow((R+0.055)/1.055,2.4);
			if(G <= 0.04045)	g = G/12.92;
			else				g = pow((G+0.055)/1.055,2.4);
			if(B <= 0.04045)	b = B/12.92;
			else				b = pow((B+0.055)/1.055,2.4);

			double X = r*0.4124564 + g*0.3575761 + b*0.1804375;
			double Y = r*0.2126729 + g*0.7151522 + b*0.0721750;
			double Z = r*0.0193339 + g*0.1191920 + b*0.9503041;
			//------------------------
			// XYZ to LAB conversion
			//------------------------
			double epsilon = 0.008856;	//actual CIE standard
			double kappa   = 903.3;		//actual CIE standard

			double Xr = 0.950456;	//reference white
			double Yr = 1.0;		//reference white
			double Zr = 1.088754;	//reference white

			double xr = X/Xr;
			double yr = Y/Yr;
			double zr = Z/Zr;

			double fx, fy, fz;
			if(xr > epsilon)	fx = pow(xr, 1.0/3.0);
			else				fx = (kappa*xr + 16.0)/116.0;
			if(yr > epsilon)	fy = pow(yr, 1.0/3.0);
			else				fy = (kappa*yr + 16.0)/116.0;
			if(zr > epsilon)	fz = pow(zr, 1.0/3.0);
			else				fz = (kappa*zr + 16.0)/116.0;

			lab.at<Vec3d>(i,j)=Vec3d(116.0*fy-16.0,500.0*(fx-fy),200.0*(fy-fz));
			//lab.at<Vec3d>(i,j)=Vec3d(B,G,R);
		}
	}
}

/*
void sassliency_map(const Mat& I,Mat& Is,bool dotranspose)
{
	Mat Ilab,Ig;
	if(dotranspose)
		GaussianBlur(I.t(),Ig,Size(3,3),0);
	else
		GaussianBlur(I,Ig,Size(3,3),0);

	bgr2lab(Ig,Ilab);

	Scalar labmean=mean(Ilab);

	Mat chnls[3];
	split(Ilab,chnls);

	chnls[0]-=labmean[0];
	chnls[1]-=labmean[1];
	chnls[2]-=labmean[2];

	Mat Is_double=(chnls[0].mul(chnls[0])+chnls[1].mul(chnls[1])+chnls[2].mul(chnls[2]));

	double min_val,max_val;
	minMaxLoc(Is_double,&min_val,&max_val);
	Is_double=Is_double.mul(255.f/max_val);

	Is_double.convertTo(Is,CV_8UC1);


	namedWindow( "saliency", CV_WINDOW_AUTOSIZE );// Create a window for display.
	imshow( "saliency", Is );                   // Show our image inside it.
	// 
	// 	namedWindow( "saliency", CV_WINDOW_AUTOSIZE );// Create a window for display.
	// 	imshow( "saliency", Is );                   // Show our image inside it.
}*/
void saliency_map(const Mat& I,/*Mat& Ilab,*/Mat& Is)
{
	Mat Ig,Ilab;
	GaussianBlur(I,Ig,Size(3,3),0);

	cvtColor(Ig,Ilab,CV_BGR2Lab);
	Scalar labmean=mean(Ilab);

	Mat Ilab_float;
	Ilab.convertTo(Ilab_float,CV_32FC3);
	Mat chnls[3];
	split(Ilab_float,chnls);

	chnls[0]-=labmean[0];
	chnls[1]-=labmean[1];
	chnls[2]-=labmean[2];

	Mat Is_double=(chnls[0].mul(chnls[0])+chnls[1].mul(chnls[1])+chnls[2].mul(chnls[2]));
	double min_val,max_val;
	minMaxLoc(Is_double,&min_val,&max_val);
	Is_double=Is_double.mul(255.f/max_val);
	Is_double.convertTo(Is,CV_8UC1);
}