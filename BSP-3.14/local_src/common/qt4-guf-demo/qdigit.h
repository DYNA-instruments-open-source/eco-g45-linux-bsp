#ifndef QDIGIT_H
#define QDIGIT_H

#include <QWidget>
#include <QLabel>
#include <QPixmap>

class QDigit : public QWidget
{

    Q_OBJECT

	public:
        QDigit(QWidget *parent = 0, unsigned int num = 0, QPixmap digits = 0);		
   
    protected:
		void paintEvent(QPaintEvent *event);
    
    public slots:
        void setNum(unsigned int num);

    private:
        int currentNum;
		QPixmap digitImage;
};

#endif // QDIGIT_H
