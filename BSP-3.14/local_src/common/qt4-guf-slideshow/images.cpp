#include <QString>
#include <QDir>

#include "images.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"

Images::Images(QWidget *parent) {
	path = ".";
}

void Images::setPath(const QString& s) {
	path = s;
}

QString Images::getPath() {
	return path;
}

int Images::count() {

	QDir imageDir(path);
	
	QStringList nameFilter;
	nameFilter.append("*.jpg");
    imageDir.setNameFilters(nameFilter);
	QStringList imageList = imageDir.entryList();		
	
	return imageList.length();
}

QString Images::getRandomImage() {

	QDir imageDir(path);
	
	QStringList nameFilter;
	nameFilter.append("*.jpg");
    imageDir.setNameFilters(nameFilter);
	QStringList imageList = imageDir.entryList();	
	
	calcRandomNumber(0, imageList.length() - 1);
	
	return imageList.at(curRandomNumber);
}

void Images::calcRandomNumber(int min, int max) {
	
	int newRandomNumber;
	
	srand(time(NULL));	
	newRandomNumber = rand() % max + min;
	
	while (newRandomNumber == curRandomNumber) {
		
		newRandomNumber = rand() % max + min;
	}
	
	curRandomNumber = newRandomNumber;
}

Images::~Images() {

}