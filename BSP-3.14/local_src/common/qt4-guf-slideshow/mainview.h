#ifndef MAINVIEW_H
#define MAINVIEW_H

#include "ui_mainview.h"
#include "qslideshow.h"

#include <QTimer>
#include <QDialog>
 
class MainView : public QDialog, public Ui::MainView {

	Q_OBJECT

	public:
		MainView(QWidget *parent=0);
		~MainView();

		QSlideShow *slideshow;
		QTimer *timer;
};

#endif // MAINVIEW_H