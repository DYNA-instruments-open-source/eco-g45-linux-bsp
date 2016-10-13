#include <QtGui>
#include <QRect>
#include <QStringList>

#include "qgufslider.h"

#define M_KORR		0
#define MAX_TEMP        60

QGufSlider :: QGufSlider (QWidget *parent, unsigned int steps,
							QPixmap empty)
: QWidget(parent)
{
	sliderSteps = steps;
	oldPos = 270;
	pos = 0;
    ten       = 0;
    one       = 0;
    fractional   = 0;
	emptySlider = empty;
}

void QGufSlider::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit isActive();
        setBalkPos(event->x());
    }
}

void QGufSlider::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        emit isActive();
        setBalkPos(event->x());
    }
}

void QGufSlider::setBalkPos(int xPosGiven)
{
	if(xPosGiven >= 0 && xPosGiven < (int)width()) {
		if(width() != 0) {
			int xPos = xPosGiven + (width()/sliderSteps)/2;

			ten = (unsigned int)((double)xPosGiven/(double)width() * 10.0f);
			one = (unsigned int)((double)xPosGiven/(double)width() * 100.0f) - ten * 10;
			fractional = (unsigned int)((double)xPosGiven/(double)width() * 1000.0f) - ten * 100 - one * 10 ;
			if(ten > 9) {
				emit pos1(9);
				emit pos2(9);
				emit pos3(9);
			} else {
			
				emit pos1(ten);
				emit pos2(one);
				emit pos3(fractional);
			}
			
			if(xPos > 0) {
				pos = xPos - (xPos % (width()/sliderSteps));
			} else {
				pos = 0;
			}
			if(pos != oldPos) {
				update();
				oldPos = pos;
			}
		}
	}
}

void QGufSlider::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
	/* Full slider is already drawn on the background. Only
	   the needed part of the empty slider needs to be drawn. */
	painter.drawPixmap(pos, 0, emptySlider, pos, 0, width() - pos, height());
}
