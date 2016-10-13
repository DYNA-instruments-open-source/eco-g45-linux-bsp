#include <QTimer>
#include <QtGui>
#include <QStringList>
#include <QPainter>
#include <QFile>

#include "qslideshow.h"
#include "images.h"
#include "stdio.h"

#define IMAGE_PATH   "/usr/share/qt4-guf-slideshow/images/"

QSlideShow::QSlideShow(QWidget *parent) : QWidget(parent) {

	QApplication::setOverrideCursor(Qt::BlankCursor);
	
    /* default settings for CUPID and NESO */
    nHeight = 480;
    nWidth  = 800;
	
	modeRandom = true;
}

void QSlideShow::setModeRandom(bool m) {
	modeRandom = m;
}

void QSlideShow::paintEvent(QPaintEvent *event) {

    QPainter painter(this);
    QStringList stringList;
	
	Images *images = new Images();
	images->setPath(IMAGE_PATH);
	
	maxSlide = images->count();
	
	if (images->count() > 0) {	
		
		if (modeRandom) {
			
			QPixmap pixmap(IMAGE_PATH + images->getRandomImage());
			painter.drawPixmap(0, 0, pixmap.scaled(nWidth, nHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));	/* scale image */
		}
		else {
			
			QFile file(IMAGE_PATH + QString::number(curSlide) + ".jpg");
			
			if (file.exists() == false) {
				searchFollowingImage();
			}
			
			QPixmap pixmap(IMAGE_PATH + QString::number(curSlide) + ".jpg");
			painter.drawPixmap(0, 0, pixmap.scaled(nWidth, nHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));	/* scale image */
		}		
	}
	else {
		painter.drawText(50, 50, "No images found in " + (QString)IMAGE_PATH);
	}
}

void QSlideShow::mousePressEvent(QMouseEvent *event) {
    
	if(event->button() == Qt::LeftButton) {
        emit activePress();
    }
}

void QSlideShow::nextPixmap() {

    curSlide++;	
	
	if ((curSlide - 1) == maxSlide) {
		curSlide = 1;
	}
	
    update();	/* calls paintEvent() */	
}

void QSlideShow::startSlide() {

    Images *images = new Images();
	images->setPath(IMAGE_PATH);
	
	curSlide = 1;
	maxSlide = images->count();
}

void QSlideShow::searchFollowingImage() {

    QFile file(IMAGE_PATH + QString::number(curSlide) + ".jpg");
	
	while (file.exists() == false) {
		
		file.setFileName(IMAGE_PATH + QString::number(curSlide) + ".jpg");
		curSlide++;
		
		if ((curSlide - 1) == maxSlide) {
			curSlide = 1;
		}
	}
}