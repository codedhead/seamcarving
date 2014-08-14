#ifndef _GLIMAGEVIEWER_H_
#define _GLIMAGEVIEWER_H_

#include <QGLWidget>

class GLImageViewer:public QGLWidget
{
public:
	GLImageViewer(QWidget * parent = 0);
	void	initializeGL ();
	void	paintGL ();
 	void	resizeGL ( int w, int h );

	void set_image(uchar* img,/*uchar* msk,*/int w, int h);
	void update_image();

// protected:
// 	virtual void	mouseMoveEvent ( QMouseEvent * event );
// 	virtual void	mousePressEvent ( QMouseEvent * event );
// 	virtual void	mouseReleaseEvent ( QMouseEvent * event );

private:
	int image_width, image_height;
	uchar* image,*emask;
};

#endif