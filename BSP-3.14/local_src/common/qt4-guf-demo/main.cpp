#include <QApplication>
#include <qwindowsystem_qws.h>
#include "mainview.h"


int main(int argc, char * argv[]) {
	QApplication app(argc, argv);
	app.setOverrideCursor(Qt::BlankCursor);
	QWSServer::setCursorVisible(false);
	MainView *mainView = new MainView();
	mainView->show();
	
	return app.exec();
}
