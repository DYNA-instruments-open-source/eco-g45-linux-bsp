#ifndef MAINVIEW_H
#define MAINVIEW_H

#include "qgufslider.h"
#include "qdigit.h"
#include "qslideshow.h"

#include <QTimer>
#include <QTimeLine>
#include <QDialog>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>

class MainView : public QDialog
{
  Q_OBJECT
public:
   QLabel *label_background;
    QPushButton *button_00;
    QPushButton *button_01;
    QPushButton *button_02;
    QPushButton *button_03;
    QPushButton *button_10;
    QPushButton *button_11;
    QPushButton *button_12;
    QPushButton *button_13;
    QPushButton *button_off;
    QPushButton *button_slideshow;
    QPushButton *button_video;
    QPushButton *button_menu;
    QPushButton *button_slider;
    QPushButton *button_help;
    QLabel *label_slider;
    QLabel *label_help;
    QButtonGroup *menuButtonGroup;
    QButtonGroup *rightButtonGroup;
	unsigned int xResolution;
	unsigned int yResolution;

public:
  MainView(QWidget *parent=0);
  ~MainView();
  void setupUi(QDialog *MainView, unsigned int xResolution, 
				unsigned int yResolution);
  
  QGufSlider *qgufslider;
  QDigit *qdigit1;
  QDigit *qdigit2;
  QDigit *qdigit3;

  QSlideShow *qslideshow;
  QTimer *timer;
  QTimeLine *timeline;

public slots:
  void restartTL();
  void rebootSystem(); 
};
#endif // MAINVIEW_H
