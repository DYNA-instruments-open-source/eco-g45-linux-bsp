#include "mainview.h"
#include "qgufslider.h"
#include "qdigit.h"
#include "qslideshow.h"
#include <stdio.h>
#include <stdlib.h>
#include <QDesktopWidget>

#define PIC_DELAY   4000    // in ms -> 1000ms = 1s
#define T_BEGINN    10000   // in ms

MainView::MainView(QWidget *parent) : QDialog(parent)
{
	setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

	xResolution =  (unsigned int)QApplication::desktop()->screenGeometry().width();
	yResolution =  (unsigned int)QApplication::desktop()->screenGeometry().height();

	setupUi(this, xResolution, yResolution);
}

MainView::~MainView()
{
}

void MainView::restartTL()
{
    QTimer::singleShot(1, timeline, SLOT(stop()));
    QTimer::singleShot(1, timeline, SLOT(start()));
}

void MainView::rebootSystem()
{
    system("reboot");  
}

void MainView::setupUi(QDialog *MainView, unsigned int xResolution, 
	unsigned int yResolution)
{
    QIcon icon_00;
    QIcon icon_01;
    QIcon icon_02;
    QIcon icon_03;
    QIcon icon_10;
    QIcon icon_11;
    QIcon icon_12;
    QIcon icon_13;
    QIcon icon_off;
	QIcon icon_slideshow;
    QIcon icon_video;
    QIcon icon_menu;
    QIcon icon_slider;
    QIcon icon_help;
	QString path;
	
	unsigned int header_height = 0;
	unsigned int menu_button_width = 0;
	unsigned int menu_button_height = 0;
	unsigned int num_menu_buttons = 0;
	unsigned int off_button_width = 0;
	unsigned int off_button_height = 0;
	unsigned int right_button_width = 0;
	unsigned int right_button_height = 0;
	unsigned int slideshow_button_width = 0;
	unsigned int slideshow_button_height = 0;
	unsigned int slider_x_offset = 0;
	unsigned int slider_y_offset = 0;
	unsigned int slider_width = 0;
	unsigned int slider_height = 0;
	unsigned int digits_x_offset = 0;
	unsigned int digits_y_offset = 0;
	unsigned int digits_width = 0;
	unsigned int digits_height = 0;
	unsigned int background_label_width = 0;
	unsigned int background_label_height = 0;
	
	/* The images for the platform (CUPID, ...) are automatically copied
	   to the image folder during the build. */
	/* TODO: 800 x 600 needed for VINCELL */
	if(xResolution == 800 && yResolution == 480) {
		path = QString::fromUtf8("/usr/share/qt4-guf-demo/images/gui70/");
		header_height = 105;
		menu_button_width = 160;
		menu_button_height = 134;
		num_menu_buttons = 8;
		off_button_width = 33;
		off_button_height = 106;
		slideshow_button_width = 127;
		slideshow_button_height = 106;
		right_button_width = 160;
		right_button_height = 67;
		slider_x_offset = 144;
		slider_y_offset = 271;
		slider_width = 352;
		slider_height = 90;
		background_label_width = 640;
		background_label_height = 480;
		digits_x_offset = 173;
		digits_y_offset = 131;
		digits_width = 64;
		digits_height = 97;
	} else if(xResolution == 640 && yResolution == 480) {
		path = QString::fromUtf8("/usr/share/qt4-guf-demo/images/gui57/");
		header_height = 105;
		menu_button_width = 160;
		menu_button_height = 134;
		num_menu_buttons = 6;
		off_button_width = 33;
		off_button_height = 106;
		slideshow_button_width = 127;
		slideshow_button_height = 106;
		right_button_width = 160;
		right_button_height = 67;
		slider_x_offset = 65;
		slider_y_offset = 264;
		slider_width = 352;
		slider_height = 90;
		background_label_width = 480;
		background_label_height = 374;
		digits_x_offset = 90;
		digits_y_offset = 123;
		digits_width = 64;
		digits_height = 96;
	} else if(xResolution == 480 && yResolution == 272) {
		path = QString::fromUtf8("/usr/share/qt4-guf-demo/images/gui43/");
		header_height = 36;
		menu_button_width = 120;
		menu_button_height = 100;
		num_menu_buttons = 6;
		off_button_width = 30;
		off_button_height = 36;
		slideshow_button_width = 90;
		slideshow_button_height = 36;
		right_button_width = 120;
		right_button_height = 50;
		slider_x_offset = 36;
		slider_y_offset = 153;
		slider_width = 288;
		slider_height = 65;
		background_label_width = 360;
		background_label_height = 236;
		digits_x_offset = 124;
		digits_y_offset = 62;
		digits_width = 32;
		digits_height = 48;
	} else { /* default the samallest one */
		path = QString::fromUtf8("/usr/share/qt4-guf-demo/images/gui43/");
		header_height = 36;
		menu_button_width = 120;
		menu_button_height = 100;
		off_button_width = 30;
		off_button_height = 36;
		slideshow_button_width = 90;
		slideshow_button_height = 36;
		right_button_width = 120;
		right_button_height = 50;
		slider_x_offset = 36;
		slider_y_offset = 153;
		slider_width = 288;
		slider_height = 65;
		background_label_width = 360;
		background_label_height = 236;
		digits_x_offset = 124;
		digits_y_offset = 62;
		digits_width = 32;
		digits_height = 48;
	}
	
    if (MainView->objectName().isEmpty()) {
        MainView->setObjectName(QString::fromUtf8("MainView"));
	}
	
    MainView->resize(xResolution, yResolution);
    MainView->setAutoFillBackground(false);
    label_background = new QLabel(MainView);
    label_background->setObjectName(QString::fromUtf8("label_background"));
    label_background->setGeometry(QRect(0, 0, xResolution, yResolution));
    label_background->setPixmap(QPixmap(path + QString::fromUtf8("background.jpg")));
    label_background->setScaledContents(true);
    label_background->setAlignment(Qt::AlignBottom|Qt::AlignLeading|Qt::AlignLeft);
    button_00 = new QPushButton(MainView);
    menuButtonGroup = new QButtonGroup(MainView);
    menuButtonGroup->setObjectName(QString::fromUtf8("menuButtonGroup"));
    menuButtonGroup->addButton(button_00);
    button_00->setObjectName(QString::fromUtf8("button_00"));
    button_00->setGeometry(QRect(0, header_height, menu_button_width, menu_button_height));
    icon_00.addFile(path + QString::fromUtf8("menu_00_0.jpg"), QSize(), QIcon::Normal, QIcon::Off);
    icon_00.addFile(path + QString::fromUtf8("menu_00_1.jpg"), QSize(), QIcon::Normal, QIcon::On);
    button_00->setIcon(icon_00);
    button_00->setIconSize(QSize(menu_button_width, menu_button_height));
    button_00->setCheckable(true);
    button_00->setAutoExclusive(true);
    button_01 = new QPushButton(MainView);
    menuButtonGroup->addButton(button_01);
    button_01->setObjectName(QString::fromUtf8("button_01"));
    button_01->setGeometry(QRect(menu_button_width, header_height, menu_button_width, menu_button_height));
    icon_01.addFile(path + QString::fromUtf8("menu_01_0.jpg"), QSize(), QIcon::Normal, QIcon::Off);
    icon_01.addFile(path + QString::fromUtf8("menu_01_1.jpg"), QSize(), QIcon::Normal, QIcon::On);
    button_01->setIcon(icon_01);
    button_01->setIconSize(QSize(menu_button_width, menu_button_height));
    button_01->setCheckable(true);
    button_01->setAutoExclusive(true);
    button_02 = new QPushButton(MainView);
    menuButtonGroup->addButton(button_02);
    button_02->setObjectName(QString::fromUtf8("button_02"));
    button_02->setGeometry(QRect(2 * menu_button_width, header_height, menu_button_width, menu_button_height));
    icon_02.addFile(path + QString::fromUtf8("menu_02_0.jpg"), QSize(), QIcon::Normal, QIcon::Off);
    icon_02.addFile(path + QString::fromUtf8("menu_02_1.jpg"), QSize(), QIcon::Normal, QIcon::On);
    button_02->setIcon(icon_02);
    button_02->setIconSize(QSize(menu_button_width, menu_button_height));
    button_02->setCheckable(true);	
    button_10 = new QPushButton(MainView);
    menuButtonGroup->addButton(button_10);
    button_10->setObjectName(QString::fromUtf8("button_10"));
    button_10->setGeometry(QRect(0, header_height + menu_button_height, menu_button_width, menu_button_height));
    icon_10.addFile(path + QString::fromUtf8("menu_10_0.jpg"), QSize(), QIcon::Normal, QIcon::Off);
    icon_10.addFile(path + QString::fromUtf8("menu_10_1.jpg"), QSize(), QIcon::Normal, QIcon::On);
    button_10->setIcon(icon_10);
    button_10->setIconSize(QSize(menu_button_width, menu_button_height));
    button_10->setCheckable(true);
    button_10->setAutoExclusive(true);
    button_11 = new QPushButton(MainView);
    menuButtonGroup->addButton(button_11);
    button_11->setObjectName(QString::fromUtf8("button_11"));
    button_11->setGeometry(QRect(menu_button_width, header_height + menu_button_height, menu_button_width, menu_button_height));
    icon_11.addFile(path + QString::fromUtf8("menu_11_0.jpg"), QSize(), QIcon::Normal, QIcon::Off);
    icon_11.addFile(path + QString::fromUtf8("menu_11_1.jpg"), QSize(), QIcon::Normal, QIcon::On);
    button_11->setIcon(icon_11);
    button_11->setIconSize(QSize(menu_button_width, menu_button_height));
    button_11->setCheckable(true);
    button_11->setAutoExclusive(true);
    button_12 = new QPushButton(MainView);
    menuButtonGroup->addButton(button_12);
    button_12->setObjectName(QString::fromUtf8("button_12"));
    button_12->setGeometry(QRect(2 * menu_button_width, header_height + menu_button_height, menu_button_width, menu_button_height));
    icon_12.addFile(path + QString::fromUtf8("menu_12_0.jpg"), QSize(), QIcon::Normal, QIcon::Off);
    icon_12.addFile(path + QString::fromUtf8("menu_12_1.jpg"), QSize(), QIcon::Normal, QIcon::On);
    button_12->setIcon(icon_12);
    button_12->setIconSize(QSize(menu_button_width, menu_button_height));
    button_12->setCheckable(true);
    button_12->setAutoExclusive(true);
	if(xResolution == 800 && yResolution == 480) {
		button_03 = new QPushButton(MainView);
		menuButtonGroup->addButton(button_03);
		button_03->setObjectName(QString::fromUtf8("button_03"));
		button_03->setGeometry(QRect(3 * menu_button_width, header_height, menu_button_width,
			menu_button_height));
		icon_03.addFile(path + QString::fromUtf8("menu_03_0.jpg"), QSize(), QIcon::Normal, QIcon::Off);
		icon_03.addFile(path + QString::fromUtf8("menu_03_1.jpg"), QSize(), QIcon::Normal, QIcon::On);
		button_03->setIcon(icon_03);
		button_03->setIconSize(QSize(menu_button_width, menu_button_height));
		button_03->setCheckable(true);
		button_03->setAutoExclusive(true);
		button_13 = new QPushButton(MainView);
		menuButtonGroup->addButton(button_13);
		button_13->setObjectName(QString::fromUtf8("button_13"));
		button_13->setGeometry(QRect(3 * menu_button_width, header_height + menu_button_height,
								menu_button_width, menu_button_height));
		icon_13.addFile(path + QString::fromUtf8("menu_13_0.jpg"), QSize(), QIcon::Normal, QIcon::Off);
		icon_13.addFile(path + QString::fromUtf8("menu_13_1.jpg"), QSize(), QIcon::Normal, QIcon::On);
		button_13->setIcon(icon_13);
		button_13->setIconSize(QSize(menu_button_width, menu_button_height));
		button_13->setCheckable(true);
		button_13->setAutoExclusive(true);
	}
    rightButtonGroup = new QButtonGroup(MainView);
    rightButtonGroup->setObjectName(QString::fromUtf8("rightButtonGroup"));
    button_off = new QPushButton(MainView);
    rightButtonGroup->addButton(button_off);
    button_off->setObjectName(QString::fromUtf8("button_off"));
    button_off->setGeometry(QRect(num_menu_buttons / 2 * menu_button_width + slideshow_button_width, header_height + 
								4 * right_button_height, off_button_width, off_button_height));
    icon_off.addFile(path + QString::fromUtf8("button_11_0.jpg"), QSize(), QIcon::Normal, QIcon::Off);
    icon_off.addFile(path + QString::fromUtf8("button_11_1.jpg"), QSize(), QIcon::Normal, QIcon::On);
    button_off->setIcon(icon_off);
    button_off->setIconSize(QSize(off_button_width, off_button_height));
    button_off->setCheckable(false);
    button_off->setAutoExclusive(false);
	button_slideshow = new QPushButton(MainView);
	rightButtonGroup->addButton(button_slideshow);
	button_slideshow->setObjectName(QString::fromUtf8("button_slideshow"));
	button_slideshow->setGeometry(QRect(num_menu_buttons / 2 * menu_button_width, header_height + 4 * right_button_height,
									slideshow_button_width, slideshow_button_height));
	icon_slideshow.addFile(path + QString::fromUtf8("button_5_0.jpg"), QSize(), QIcon::Normal, QIcon::Off);
	icon_slideshow.addFile(path + QString::fromUtf8("button_5_1.jpg"), QSize(), QIcon::Normal, QIcon::On);
	button_slideshow->setIcon(icon_slideshow);
	button_slideshow->setIconSize(QSize(slideshow_button_width, slideshow_button_height));
	button_slideshow->setCheckable(false);
	button_slideshow->setAutoExclusive(false);
    button_video = new QPushButton(MainView);
    rightButtonGroup->addButton(button_video);
    button_video->setObjectName(QString::fromUtf8("button_video"));
    button_video->setGeometry(QRect(num_menu_buttons / 2 * menu_button_width, header_height,
								right_button_width, right_button_height));
    icon_video.addFile(path + QString::fromUtf8("button_1_0.jpg"), QSize(), QIcon::Normal, QIcon::Off);
    icon_video.addFile(path + QString::fromUtf8("button_1_1.jpg"), QSize(), QIcon::Normal, QIcon::On);
    button_video->setIcon(icon_video);
    button_video->setIconSize(QSize(right_button_width, right_button_height));
    button_video->setCheckable(true);
    button_video->setAutoExclusive(true);
    button_menu = new QPushButton(MainView);
    rightButtonGroup->addButton(button_menu);
    button_menu->setObjectName(QString::fromUtf8("button_menu"));
    button_menu->setGeometry(QRect(num_menu_buttons / 2 * menu_button_width, header_height + right_button_height,
								right_button_width, right_button_height));
    icon_menu.addFile(path + QString::fromUtf8("button_2_0.jpg"), QSize(), QIcon::Normal, QIcon::Off);
    icon_menu.addFile(path + QString::fromUtf8("button_2_1.jpg"), QSize(), QIcon::Normal, QIcon::On);
    button_menu->setIcon(icon_menu);
    button_menu->setIconSize(QSize(right_button_width, right_button_height));
    button_menu->setCheckable(true);
    button_menu->setAutoExclusive(true);
    button_slider = new QPushButton(MainView);
    rightButtonGroup->addButton(button_slider);
    button_slider->setObjectName(QString::fromUtf8("button_slider"));
    button_slider->setGeometry(QRect(num_menu_buttons / 2 * menu_button_width, header_height + 2 * right_button_height,
								right_button_width, right_button_height));
    icon_slider.addFile(path + QString::fromUtf8("button_3_0.jpg"), QSize(), QIcon::Normal, QIcon::Off);
    icon_slider.addFile(path + QString::fromUtf8("button_3_1.jpg"), QSize(), QIcon::Normal, QIcon::On);
    button_slider->setIcon(icon_slider);
    button_slider->setIconSize(QSize(right_button_width, right_button_height));
    button_slider->setCheckable(true);
    button_slider->setAutoExclusive(true);
    button_help = new QPushButton(MainView);
    rightButtonGroup->addButton(button_help);
    button_help->setObjectName(QString::fromUtf8("button_help"));
    button_help->setGeometry(QRect(num_menu_buttons / 2 * menu_button_width, header_height + 3 * right_button_height,
								right_button_width, right_button_height));
    icon_help.addFile(path + QString::fromUtf8("button_4_0.jpg"), QSize(), QIcon::Normal, QIcon::Off);
    icon_help.addFile(path + QString::fromUtf8("button_4_1.jpg"), QSize(), QIcon::Normal, QIcon::On);
    button_help->setIcon(icon_help);
    button_help->setIconSize(QSize(right_button_width, right_button_height));
    button_help->setCheckable(true);
    button_help->setAutoExclusive(true);
    label_slider = new QLabel(MainView);
    label_slider->setObjectName(QString::fromUtf8("label_slider"));
    label_slider->setGeometry(QRect(0, 0, background_label_width, background_label_height));
    label_slider->setPixmap(QPixmap(path + QString::fromUtf8("slider_back.jpg")));
    label_slider->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
	label_slider->hide();
    label_help = new QLabel(MainView);
	label_help->hide();
    label_help->setObjectName(QString::fromUtf8("label_help"));
    label_help->setGeometry(QRect(0, 0, background_label_width, background_label_height));
    label_help->setPixmap(QPixmap(path + QString::fromUtf8("help_back.jpg")));
    label_help->setScaledContents(true);
    label_help->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
	qgufslider = new QGufSlider(this, 16,
					QPixmap(path + QString::fromUtf8("bar_0.jpg")));
    qgufslider->hide();
    qgufslider->setGeometry(QRect(slider_x_offset, slider_y_offset, slider_width, slider_height));
	qdigit1 = new QDigit(this, 0, QPixmap(path + QString::fromUtf8("digits_2.jpg")));
    qdigit2 = new QDigit(this, 0, QPixmap(path + QString::fromUtf8("digits_1.jpg")));
    qdigit3 = new QDigit(this, 0, QPixmap(path + QString::fromUtf8("digits_0.jpg")));
    qdigit1->hide();
    qdigit2->hide();
    qdigit3->hide();
	qdigit1->setGeometry(QRect(digits_x_offset, digits_y_offset, digits_width, digits_height));
    qdigit2->setGeometry(QRect(digits_x_offset + digits_width + 5, digits_y_offset, digits_width, digits_height));
    qdigit3->setGeometry(QRect(digits_x_offset + 2 * digits_width + 5 + 22, digits_y_offset, digits_width, digits_height));	
	qslideshow = new QSlideShow(this);
    qslideshow->hide();
	qslideshow->setGeometry(QRect( 0, 0, xResolution, yResolution));
    qslideshow->addImage(QPixmap(path + QString::fromUtf8("slide_0.jpg")));
    qslideshow->addImage(QPixmap(path + QString::fromUtf8("slide_1.jpg")));
    qslideshow->addImage(QPixmap(path + QString::fromUtf8("slide_2.jpg")));
    qslideshow->addImage(QPixmap(path + QString::fromUtf8("slide_3.jpg")));
    qslideshow->addImage(QPixmap(path + QString::fromUtf8("slide_4.jpg")));
    qslideshow->addImage(QPixmap(path + QString::fromUtf8("slide_5.jpg")));
    QMetaObject::connectSlotsByName(MainView);
	timer = new QTimer(this);
    timeline = new QTimeLine(T_BEGINN,this);
    timer->setInterval(PIC_DELAY);
    timeline->start();
	
    // ****** CONNECTIONS FOR DIGIT & SLIDER ******
    QObject::connect(button_video, SIGNAL(clicked()), label_slider, SLOT(hide()));
    QObject::connect(button_video, SIGNAL(clicked()), qgufslider,SLOT(hide()));
    QObject::connect(button_video, SIGNAL(clicked()), label_help,SLOT(hide()));
    QObject::connect(button_video, SIGNAL(clicked()), qdigit1,SLOT(hide()));
    QObject::connect(button_video, SIGNAL(clicked()), qdigit2,SLOT(hide()));
    QObject::connect(button_video, SIGNAL(clicked()), qdigit3,SLOT(hide()));
    QObject::connect(button_video, SIGNAL(clicked()), button_00,SLOT(hide()));
    QObject::connect(button_video, SIGNAL(clicked()), button_01,SLOT(hide()));
    QObject::connect(button_video, SIGNAL(clicked()), button_02,SLOT(hide()));
    QObject::connect(button_video, SIGNAL(clicked()), button_10,SLOT(hide()));
    QObject::connect(button_video, SIGNAL(clicked()), button_11,SLOT(hide()));
    QObject::connect(button_video, SIGNAL(clicked()), button_12,SLOT(hide()));
    QObject::connect(button_slider, SIGNAL(clicked()), label_slider, SLOT(show()));
    QObject::connect(button_slider, SIGNAL(clicked()), qgufslider,SLOT(show()));
    QObject::connect(button_slider, SIGNAL(clicked()), qdigit1,SLOT(show()));
    QObject::connect(button_slider, SIGNAL(clicked()), qdigit2,SLOT(show()));
    QObject::connect(button_slider, SIGNAL(clicked()), qdigit3,SLOT(show()));
    QObject::connect(button_slider, SIGNAL(clicked()), button_00,SLOT(hide()));
    QObject::connect(button_slider, SIGNAL(clicked()), button_01,SLOT(hide()));
    QObject::connect(button_slider, SIGNAL(clicked()), button_02,SLOT(hide()));
    QObject::connect(button_slider, SIGNAL(clicked()), button_10,SLOT(hide()));
    QObject::connect(button_slider, SIGNAL(clicked()), button_11,SLOT(hide()));
    QObject::connect(button_slider, SIGNAL(clicked()), button_12,SLOT(hide()));
    QObject::connect(button_menu, SIGNAL(clicked()), qgufslider,SLOT(hide()));
    QObject::connect(button_menu, SIGNAL(clicked()), qdigit1,SLOT(hide()));
    QObject::connect(button_menu, SIGNAL(clicked()), qdigit2,SLOT(hide()));
    QObject::connect(button_menu, SIGNAL(clicked()), qdigit3,SLOT(hide()));
    QObject::connect(button_menu, SIGNAL(clicked()), label_slider, SLOT(hide()));
    QObject::connect(button_menu, SIGNAL(clicked()), button_00,SLOT(show()));
    QObject::connect(button_menu, SIGNAL(clicked()), button_01,SLOT(show()));
    QObject::connect(button_menu, SIGNAL(clicked()), button_02,SLOT(show()));
    QObject::connect(button_menu, SIGNAL(clicked()), button_10,SLOT(show()));
    QObject::connect(button_menu, SIGNAL(clicked()), button_11,SLOT(show()));
    QObject::connect(button_menu, SIGNAL(clicked()), button_12,SLOT(show()));
    QObject::connect(button_help, SIGNAL(clicked()), qgufslider,SLOT(hide()));
    QObject::connect(button_help, SIGNAL(clicked()), qdigit1,SLOT(hide()));
    QObject::connect(button_help, SIGNAL(clicked()), qdigit2,SLOT(hide()));
    QObject::connect(button_help, SIGNAL(clicked()), qdigit3,SLOT(hide()));
    QObject::connect(button_help, SIGNAL(clicked()), label_slider, SLOT(hide()));
    QObject::connect(button_help, SIGNAL(clicked()), button_00,SLOT(hide()));
    QObject::connect(button_help, SIGNAL(clicked()), button_01,SLOT(hide()));
    QObject::connect(button_help, SIGNAL(clicked()), button_02,SLOT(hide()));
    QObject::connect(button_help, SIGNAL(clicked()), button_10,SLOT(hide()));
    QObject::connect(button_help, SIGNAL(clicked()), button_11,SLOT(hide()));
    QObject::connect(button_help, SIGNAL(clicked()), button_12,SLOT(hide()));

    QObject::connect(qgufslider, SIGNAL(pos1(unsigned int)), qdigit1, SLOT(setNum(unsigned int)));
    QObject::connect(qgufslider, SIGNAL(pos2(unsigned int)), qdigit2, SLOT(setNum(unsigned int)));
    QObject::connect(qgufslider, SIGNAL(pos3(unsigned int)), qdigit3, SLOT(setNum(unsigned int)));
	
    // ****** CONNECTIONS FOR TIMER CONTROL ******
    QObject::connect(timeline, SIGNAL(finished()), timer,SLOT(start()));
    QObject::connect(timeline, SIGNAL(finished()), qslideshow,SLOT(show()));
    QObject::connect(timeline, SIGNAL(finished()), qslideshow,SLOT(startSlide()));
    QObject::connect(timer, SIGNAL(timeout()), qslideshow,SLOT(nextPixmap()));
    QObject::connect(timeline,SIGNAL(finished()),timeline,SLOT(stop()));
    QObject::connect(timeline,SIGNAL(finished()),qslideshow,SLOT(raise()));

    QObject::connect(qslideshow,SIGNAL(activePress()),qslideshow,SLOT(hide()));
    QObject::connect(qslideshow,SIGNAL(activePress()),timer,SLOT(stop()));
    QObject::connect(qslideshow,SIGNAL(activePress()),timeline,SLOT(start()));

    QObject::connect(button_slideshow, SIGNAL(clicked()), timer,SLOT(start()));

    // ****** SLIDER ACTIVITY *******
    QObject::connect(qgufslider,SIGNAL(isActive()), this,SLOT(restartTL()));

    // ****** BUTTONGROUP 2 ******
    QObject::connect(button_00,SIGNAL(pressed()), this,SLOT(restartTL()));
    QObject::connect(button_01,SIGNAL(pressed()), this,SLOT(restartTL()));
    QObject::connect(button_02,SIGNAL(pressed()), this,SLOT(restartTL()));
    QObject::connect(button_10,SIGNAL(pressed()), this,SLOT(restartTL()));
    QObject::connect(button_11,SIGNAL(pressed()), this,SLOT(restartTL()));
    QObject::connect(button_12,SIGNAL(pressed()), this,SLOT(restartTL()));

	if(xResolution == 800 && yResolution == 480) {
		QObject::connect(button_03,SIGNAL(pressed()), this,SLOT(restartTL()));
		QObject::connect(button_13,SIGNAL(pressed()), this,SLOT(restartTL()));
		QObject::connect(button_video, SIGNAL(clicked()), button_03,SLOT(hide()));
		QObject::connect(button_video, SIGNAL(clicked()), button_13,SLOT(hide()));
		QObject::connect(button_menu, SIGNAL(clicked()), button_03,SLOT(show()));
		QObject::connect(button_menu, SIGNAL(clicked()), button_13,SLOT(show()));
		QObject::connect(button_slider, SIGNAL(clicked()), button_03,SLOT(hide()));
		QObject::connect(button_slider, SIGNAL(clicked()), button_13,SLOT(hide()));
		QObject::connect(button_help, SIGNAL(clicked()), button_03,SLOT(hide()));
		QObject::connect(button_help, SIGNAL(clicked()), button_13,SLOT(hide()));
	}
	
    // ****** BUTTONGROUP 1 ******
    QObject::connect(button_video,SIGNAL(pressed()), this,SLOT(restartTL()));
    QObject::connect(button_menu,SIGNAL(pressed()), this,SLOT(restartTL()));
    QObject::connect(button_slider,SIGNAL(pressed()), this,SLOT(restartTL()));
    QObject::connect(button_help,SIGNAL(pressed()), this,SLOT(restartTL()));

    // ***** HELP(INFO) BUTTON ****
    QObject::connect(button_help,SIGNAL(clicked()), label_help,SLOT(show()));
    QObject::connect(button_slider,SIGNAL(clicked()), label_help,SLOT(hide()));
    QObject::connect(button_menu,SIGNAL(clicked()), label_help,SLOT(hide()));
    QObject::connect(button_video,SIGNAL(clicked()), label_help,SLOT(hide()));

    // ***** QUIT BUTTON*****
    QObject::connect(button_off,SIGNAL(clicked()), this,SLOT(rebootSystem()));
    QObject::connect(button_slideshow, SIGNAL(clicked()), qslideshow,SLOT(show()));
    QObject::connect(button_slideshow, SIGNAL(clicked()), qslideshow,SLOT(startSlide()));
    QObject::connect(button_slideshow, SIGNAL(clicked()), timeline,SLOT(stop()));
    QObject::connect(button_slideshow, SIGNAL(clicked()), qslideshow,SLOT(raise()));
}
   
