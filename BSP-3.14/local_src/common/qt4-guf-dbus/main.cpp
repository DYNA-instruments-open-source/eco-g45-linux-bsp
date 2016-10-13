#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtDBus/QtDBus>

#include "removabledisk.h"

int main(int argc, char * argv[]) {
	QCoreApplication app(argc, argv);
	RemovableDisk removableDisk;
	app.exec();	
    return 0;
}
