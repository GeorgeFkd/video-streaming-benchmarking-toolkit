

#include <QTimer>
#include <QtWidgets>
#include <QApplication>
#include "PreciseTimer.h"




int main(int argc,char **argv) {
	QApplication app(argc,argv);
	PreciseTimer timer;
	timer.show();
	
	return app.exec();
}
