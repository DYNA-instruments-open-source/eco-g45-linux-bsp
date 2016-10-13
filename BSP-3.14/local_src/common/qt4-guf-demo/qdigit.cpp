#include <QtGui>
#include <QRect>
#include "qdigit.h"

QDigit::QDigit(QWidget *parent, unsigned int num, QPixmap digits) :
    QWidget(parent)
{
    currentNum = num;
	digitImage = digits;
}

void QDigit::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
	painter.drawPixmap(0, 0, digitImage, currentNum * digitImage.width() / 10,
						0, digitImage.width() / 10, digitImage.height());
}

void QDigit::setNum(unsigned int num)
{
	if(num <= 9) {
		currentNum = num;
		update();
	}
}
