#ifndef QSLIDESHOW_H
#define QSLIDESHOW_H

#include <QWidget>
#include <QPixmap>

class QSlideShow : public QWidget
{
    Q_OBJECT
public:
    explicit QSlideShow(QWidget *parent = 0);	
	void addImage(QPixmap image);
   
protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);

signals:
    void activePress();

public slots:
    void nextPixmap();
    void startSlide();


private:
    int currentSlide;
	QVector<QPixmap> images;
};

#endif // QSLIDESHOW_H
