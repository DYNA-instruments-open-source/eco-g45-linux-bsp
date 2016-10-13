#ifndef QGUFSLIDER_H
#define QGUFSLIDER_H

#include <QWidget>
#include <QLabel>
#include <QPixmap>

class QGufSlider : public QWidget{

    Q_OBJECT
	
    public:
        QGufSlider(QWidget *parent = 0, unsigned int steps = 0,
			QPixmap empty = 0);
        void paintEvent(QPaintEvent *event);

        int ten;
        int one;
        int fractional ;
        unsigned int sliderSteps;


    protected:
        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void setBalkPos(int xPos);


    private:
        unsigned int sliderStepWidth;
		QPixmap emptySlider;
        unsigned int pos; /* mousposition in slidebalk */
        unsigned int oldPos;

    signals:
		/* signals sending the digit values */
        void pos1(unsigned int ten);
        void pos2(unsigned int one);
        void pos3(unsigned int fractional);
        void isActive();
};

#endif

