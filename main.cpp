#include "seamcarvewnd.h"
#include <QtGui/QApplication>

#include <windows.h>

// class ResizeStopFilter : public QObject
// {
// public:
// 	ResizeStopFilter(QObject* parent=0):QObject(parent){}
// 
// protected:
// 	bool eventFilter(QObject *obj, QEvent *event)
// 	{
// 		if (event->type() == QEvent::Resize::MouseButtonRelease)
// 		{
// 			//QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
// 			//qDebug("Ate key press %d", keyEvent->key());
// 			printf("release\n");
// 			
// 		}
// 		return false;
// 	}
// };


int main(int argc, char *argv[])
{
	AllocConsole();
	
	//freopen("test1.txt","w",stdout);
	freopen("CONOUT$","w",stdout);

	QApplication a(argc, argv);
	//QCoreApplication::instance()->installEventFilter(&rzstopfilter);
	//a.installEventFilter(&rzstopfilter);
	SeamCarveWnd w;
	w.setWindowFlags(w.windowFlags()&(~Qt::WindowMaximizeButtonHint));
	w.show();
	return a.exec();
}
