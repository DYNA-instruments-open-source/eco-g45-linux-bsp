#include <QTimer>
#include <QtGui>
#include <QStringList>

#include "qslideshow.h"

QSlideShow::QSlideShow(QWidget *parent) :
    QWidget(parent)
{
    currentSlide   = 0;
}

void QSlideShow::addImage(QPixmap image) {
	images.append(image);
}

void QSlideShow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
	if(currentSlide < images.size()){
		painter.drawPixmap(0, 0, images[currentSlide]);
	}
}

void QSlideShow::nextPixmap()
{
    if (currentSlide == images.size()) {
        currentSlide = 0;
    } else{
		currentSlide++;
	}
    update();
}

void QSlideShow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton) {
        emit activePress();
    }
}

void QSlideShow::startSlide()
{
    currentSlide = 0;
}
