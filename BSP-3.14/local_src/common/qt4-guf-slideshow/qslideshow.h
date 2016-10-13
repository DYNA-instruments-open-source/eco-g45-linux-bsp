#ifndef QSLIDESHOW_H
#define QSLIDESHOW_H

#include <QWidget>
#include <QPixmap>

class QSlideShow : public QWidget {
    
	Q_OBJECT
	
	public:
		explicit QSlideShow(QWidget *parent = 0);
		void setModeRandom(bool m);
		
		int nHeight;
		int nWidth;
	   
	protected:
		void paintEvent(QPaintEvent *event);
		void mousePressEvent(QMouseEvent *event);
	
	signals:
		void activePress();
	
	public slots:
		void nextPixmap();
		void startSlide();

	private:
		void searchFollowingImage();
		
		bool modeRandom;
		int curSlide;
		int maxSlide;
};

#endif // QSLIDESHOW_H