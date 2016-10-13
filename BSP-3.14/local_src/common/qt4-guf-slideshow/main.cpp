#include <QApplication>
#include <qwindowsystem_qws.h>
#include "mainview.h"
#include "stdio.h"

int main(int argc, char * argv[]) {
	
	//printf ("start qt4-guf-slideshow\n");
	QApplication app(argc, argv);
	app.setOverrideCursor(Qt::BlankCursor);
	QWSServer::setCursorVisible(false);
	MainView *mainView = new MainView();
	mainView->show();
	return app.exec();
}
