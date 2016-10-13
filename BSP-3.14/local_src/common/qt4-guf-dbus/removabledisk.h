#ifndef REMOVALBLEDISK_H
#define REMOVALBLEDISK_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>

class RemovableDisk: public QObject
{
    Q_OBJECT
public:
	RemovableDisk();
	~RemovableDisk();
public slots:
    void deviceAdded(QDBusObjectPath path);
    void deviceRemoved(QDBusObjectPath path);
private:
	QDBusInterface *disksIface;
	QMap<QString, QString> mounts;
};

#endif