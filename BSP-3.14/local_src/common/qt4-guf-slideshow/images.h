#ifndef IMAGES_H
#define IMAGES_H

#include <QString>

class Images {

	public:
		Images(QWidget *parent = 0);
		~Images();

		void setPath(const QString& s);
		QString getPath();		
		int count();
		QString getRandomImage();
	
	private:
		void calcRandomNumber(int min, int max);		
		
		int curRandomNumber;		
		QString path;
};

#endif // IMAGES_H