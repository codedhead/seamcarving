#include "seamcarvewnd.h"
#include "timer.h"
#include "seamcarver.h"



#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/gpu/gpu.hpp>

#include <QImage>
#include <QFileDialog>
#include <QResizeEvent>
#include <QActionGroup>



SeamCarveWnd::SeamCarveWnd(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags),edit_mode(EDIT_NONE),w_delta(0),h_delta(0),loading_file(false),
	which_carver(CARVER_GRADIENT),actgrp_energy(this),actgrp_matrix(this)
{
	ui.setupUi(this);

	seam_carvers[CARVER_GRADIENT]=new SeamCarver;
	seam_carvers[CARVER_SALIENCY]=new SaliencySCarver;
	seam_carvers[CARVER_GRAD_SALIENCY]=new GradSaliencySCarver;

	actgrp_energy.setExclusive(true);
	actgrp_matrix.setExclusive(true);

	actgrp_energy.addAction(ui.actionEnergyGradient);
	actgrp_energy.addAction(ui.actionEnergySaliency);
	actgrp_energy.addAction(ui.actionEnergyGradSaliency);

	actgrp_matrix.addAction(ui.actionMatrixDP);
	actgrp_matrix.addAction(ui.actionMatrixParallel);

// 	connect(&resize_timer, SIGNAL(timeout()), this, SLOT(resize_timecheck()));
// 	resize_timer.setSingleShot(true);
	is_resizing=false;
}

SeamCarveWnd::~SeamCarveWnd()
{

}

void SeamCarveWnd::reset()
{
	//Ioriginal=Mat();

	ui.glviewer->set_image(0,0,0);
// 	this->setMaximumWidth(w+w_delta);
// 	this->setMaximumHeight(h+h_delta);
// 	this->resize(w+w_delta,h+h_delta);

	bool dp_or_greedy=ui.actionMatrixDP->isChecked();

	for(int i=0;i<CARVER_COUNT;++i)
	{
		seam_carvers[i]->matrix_method(dp_or_greedy);
		seam_carvers[i]->reset();
	}
}

void SeamCarveWnd::setCarver(int which)
{
	if(which_carver==which) return;
	if(which!=-1) which_carver=which;
	if(!Ioriginal.data)
		return;

	ui.actionDrawProtect->blockSignals(true);
	ui.actionDrawRemove->blockSignals(true);

	ui.actionDrawProtect->setChecked(false);
	ui.actionDrawRemove->setChecked(false);

	ui.actionDrawProtect->blockSignals(false);
	ui.actionDrawRemove->blockSignals(false);


	int h=Ioriginal.rows,w=Ioriginal.cols;
	loading_file=true;

	this->setMaximumWidth(w+w_delta);
	this->setMaximumHeight(h+h_delta);
	this->resize(w+w_delta,h+h_delta);

	seam_carvers[which_carver]->load(Ioriginal);
	ui.glviewer->set_image(seam_carvers[which_carver]->image_ptr(),w,h);

	loading_file=false;
}

void SeamCarveWnd::on_actionEnergyGradient_toggled(bool checked)
{
	if(checked)
	{
		setCarver(CARVER_GRADIENT);
	}
}
void SeamCarveWnd::on_actionEnergySaliency_toggled(bool checked)
{
	if(checked)
	{
		setCarver(CARVER_SALIENCY);
	}
}

void SeamCarveWnd::on_actionEnergyGradSaliency_toggled(bool checked)
{
	if(checked)
	{
		setCarver(CARVER_GRAD_SALIENCY);
	}
}

void SeamCarveWnd::on_actionMatrixDP_toggled(bool checked)
{
	if(checked)
	{
		reset();
		setCarver(-1);
	}
}
void SeamCarveWnd::on_actionMatrixParallel_toggled(bool checked)
{
	if(checked)
	{
		reset();
		setCarver(-1);
	}
}

void SeamCarveWnd::on_actionResetSize_triggered(bool checked)
{
	setCarver(-1);
}

void SeamCarveWnd::on_actionOpen_triggered(bool checked)
{
	static bool calc_wh_delta=true;
	if(calc_wh_delta)
	{
		calc_wh_delta=false;
		w_delta=this->width()-ui.glviewer->width();
		h_delta=this->height()-ui.glviewer->height();
		//printf("%d %d\n",w_delta,h_delta);
	}

	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Open File"), "", tr("Images (*.png *.jpg *.bmp)"));
	if(!fileName.isEmpty())
	{
		Mat I=imread(fileName.toStdString());
		if(!I.data)
		{
			return;
		}
		if(I.rows<MIN_WIDTH||I.cols<MIN_HEIGHT)
		{
			printf("Image size should be larger than %d*%d\n",MIN_WIDTH,MIN_HEIGHT);
			return;
		}

		Ioriginal=I;

		reset();

		setCarver(-1);
	}
}

void SeamCarveWnd::on_actionSave_triggered(bool checked)
{
	QString filename=QFileDialog::getSaveFileName(this,
		tr("Save File"),"",tr("Images (*.png)"));
	if(filename.isEmpty()||!seam_carvers[which_carver]->image_ptr()) return;

	imwrite(filename.toStdString(),seam_carvers[which_carver]->shrinked_image());
}

void SeamCarveWnd::on_actionDrawProtect_toggled(bool checked)
{
	if(checked)
	{
		ui.actionDrawRemove->setChecked(false);
		edit_mode=EDIT_PROTECT;

		seam_carvers[which_carver]->test_show_seam(true);

	}
	else if(!ui.actionDrawRemove->isChecked())
	{
		edit_mode=EDIT_NONE;

		seam_carvers[which_carver]->test_remove_seam(true);
		//seam_carver.test_show_seam(false);
	}

	ui.glviewer->update_image();
}
void SeamCarveWnd::on_actionDrawRemove_toggled(bool checked)
{
// 	if(checked)
// 	{
// 		ui.actionDrawProtect->setChecked(false);
// 		edit_mode=EDIT_REMOVE;
// 	}
// 	else if(!ui.actionDrawProtect->isChecked())
// 	{
// 		edit_mode=EDIT_NONE;
// 	}
	seam_carvers[which_carver]->debug_show_seam(true,10);
	ui.glviewer->update_image();
}

// void SeamCarveWnd::resize_timecheck()
// {
// 	if(is_resizing)
// 	{
// 		is_resizing=false;
// 
// 		if(!loading_file&&Ioriginal.data/*&&event->oldSize().width()>event->size().width()*/)
// 		{
// 			printf("resize begin...\n");
// 			tic();		
// 			seam_carvers[which_carver]->shrink(ui.glviewer->width(),ui.glviewer->height());
// 			ui.glviewer->update_image();
// 			toc();
// 			
// 		}
// 	}
// }


void SeamCarveWnd::resizeEvent ( QResizeEvent * event )
{
	if(!loading_file&&Ioriginal.data)
	{
		is_resizing=true;
		//if(!resize_timer.isActive())
		//	resize_timer.start(500);
		QString sztext;
		sztext.sprintf("(%d, %d)",ui.glviewer->width(),ui.glviewer->height());
		if(ui.glviewer->width()!=Ioriginal.cols||ui.glviewer->height()!=Ioriginal.rows)
			sztext.append(" Press Enter to start resizing.");
		this->statusBar()->showMessage(sztext);
	}

	
	
	
// 	if(!loading_file&&Ioriginal.data/*&&event->oldSize().width()>event->size().width()*/)
// 	{
//  		tic();		
// 		seam_carvers[which_carver]->shrink(ui.glviewer->width(),ui.glviewer->height());
// 		ui.glviewer->update_image();
//  		toc();
// 	}

	return QMainWindow::resizeEvent(event);
}

void SeamCarveWnd::keyPressEvent ( QKeyEvent * event )
{
	if(is_resizing&&(event->key()==Qt::Key_Enter||event->key()==Qt::Key_Return))
	{

		is_resizing=false;

		if(!loading_file&&Ioriginal.data)
		{
			printf("resize begin...\n");
			tic();		
			seam_carvers[which_carver]->shrink(ui.glviewer->width(),ui.glviewer->height());
			ui.glviewer->update_image();
			toc();

		}
		return event->accept();
	}

	return QMainWindow::keyPressEvent(event);
}