#include "mainview.h"
#include "qslideshow.h"
#include "stdio.h"

#define TIMER			false
#define TIMER_INTERVAL  2000

MainView::MainView(QWidget *parent) : QDialog(parent) {

	setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setupUi(this);
	
	slideshow = new QSlideShow(this);		
    timer = new QTimer(this);
	
	slideshow->setGeometry(QRect(0, 0, 800, 480));
	slideshow->setModeRandom(false);
	slideshow->show();
	slideshow->startSlide();
	
	if (TIMER) {
		
		timer->setInterval(TIMER_INTERVAL);
		timer->setSingleShot(false);
		timer->start();
		
		QObject::connect(timer, SIGNAL(timeout()), slideshow, SLOT(nextPixmap()));
	}
	else {
		
		QObject::connect(slideshow, SIGNAL(activePress()), slideshow, SLOT(nextPixmap()));	
	}
}

MainView::~MainView() {

}