#ifndef SEAMCARVEWND_H
#define SEAMCARVEWND_H

#include <QtGui/QMainWindow>
#include <QTimer>
#include "ui_seamcarvewnd.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace cv;


class SeamCarver;


enum SCEditMode
{
	EDIT_NONE,
	EDIT_PROTECT,
	EDIT_REMOVE,
};

enum SCCarver
{
	CARVER_GRADIENT,
	CARVER_SALIENCY,
	CARVER_GRAD_SALIENCY,

	CARVER_COUNT,
};

class SeamCarveWnd : public QMainWindow
{
	Q_OBJECT

public:
	SeamCarveWnd(QWidget *parent = 0, Qt::WFlags flags = 0);
	~SeamCarveWnd();

	void reset();
	void setCarver(int which);

public slots:
	void on_actionOpen_triggered(bool checked);
	void on_actionSave_triggered(bool checked);
	void on_actionResetSize_triggered(bool checked);
	void on_actionDrawProtect_toggled(bool checked);
	void on_actionDrawRemove_toggled(bool checked);

	void on_actionEnergyGradient_toggled(bool checked);
	void on_actionEnergySaliency_toggled(bool checked);
	void on_actionEnergyGradSaliency_toggled(bool checked);

	void on_actionMatrixDP_toggled(bool checked);
	void on_actionMatrixParallel_toggled(bool checked);

	//void resize_timecheck();

protected:
	void resizeEvent ( QResizeEvent * event );
	void keyPressEvent ( QKeyEvent * event );
	//void mouseReleaseEvent ( QMouseEvent * e );
	//void mouseMoveEvent ( QMouseEvent * event );

private:
	Ui::SeamCarveWndClass ui;
	int edit_mode;

	QActionGroup actgrp_energy,actgrp_matrix;

	int w_delta,h_delta;

	SeamCarver* seam_carvers[CARVER_COUNT];
	int which_carver;
	Mat Ioriginal;

	bool loading_file;

	QTimer resize_timer;
	bool is_resizing;
};

#endif // SEAMCARVEWND_H
