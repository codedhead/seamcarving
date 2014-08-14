#include "imageviewer.h"

GLImageViewer::GLImageViewer(QWidget * parent)
:QGLWidget(parent),image(0),image_width(0),image_height(0)
{

}

GLuint tex_quad;

void GLImageViewer::initializeGL ()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	


	glGenTextures(1,&tex_quad);
	glBindTexture(GL_TEXTURE_2D,tex_quad);

	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
}
void GLImageViewer::paintGL ()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	glBegin(GL_POLYGON);
	glTexCoord2f(0.f,0.f);
	glVertex2i(0,0);

	glTexCoord2f(0.f,1.f);
	glVertex2i(0,image_height);

	glTexCoord2f(1.f,1.f);
	glVertex2i(image_width,image_height);

	glTexCoord2f(1.f,0.f);
	glVertex2i(image_width,0);
	glEnd();


	glFlush();
}

void GLImageViewer::resizeGL ( int w, int h )
{
	glViewport(0,0, w, h);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0,w,h,0);
	glMatrixMode(GL_MODELVIEW);
}

void GLImageViewer::set_image(uchar* img,int w, int h)
{
	image=img;
	image_width=w; image_height=h;
	
	if(img)
	{
		glEnable(GL_TEXTURE_2D);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,
			w,h,0,GL_BGR_EXT,GL_UNSIGNED_BYTE,img);
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
	}
	updateGL();
	
}

void GLImageViewer::update_image()
{
	if(image)
	{
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,
			image_width,image_height,0,GL_BGR_EXT,GL_UNSIGNED_BYTE,image);
		updateGL();
	}
}
