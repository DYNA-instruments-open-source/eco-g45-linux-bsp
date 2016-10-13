#include <stdio.h>
#include <stdlib.h>

#include <QtCore/QDebug>

#include "removabledisk.h"

void RemovableDisk::deviceAdded(QDBusObjectPath path)
{
	qDebug() << "Device added: " << path.path();

	QDBusInterface *propIface = new QDBusInterface("org.freedesktop.DeviceKit.Disks", 
		path.path(), 
		"org.freedesktop.DBus.Properties", 
		QDBusConnection::systemBus());

	QDBusInterface *deviceIface = new QDBusInterface("org.freedesktop.DeviceKit.Disks", 
		path.path(), 
		"org.freedesktop.DeviceKit.Disks.Device", 
		QDBusConnection::systemBus());

	QDBusReply<QVariant> reply = propIface->call("Get", "org.freedesktop.DeviceKit.Disks.Device", "device-file");
	if (!reply.isValid()) {
			qDebug() << "Call failed: " << reply.error().message();
    } else {
		qDebug() << "Device fle: " << reply.value().toString();
	}

	/* Check if we are a partition */
	reply = propIface->call("Get", "org.freedesktop.DeviceKit.Disks.Device", "device-is-partition");
	if (!reply.isValid()) {
			qDebug() << "Call failed: " << reply.error().message();
    } else {
		if(reply.value().toBool()) {
			qDebug() << "Mounting..."; 
			QDBusReply<QString> reply = deviceIface->call("FilesystemMount", "auto", (QStringList)"rw");	
			if (!reply.isValid()) {
				qDebug() << "Call failed: " << reply.error().message();
			} else {
				qDebug() << "Mounted to: " << reply.value(); 
				mounts[path.path()] = reply.value();
			}
		}
	}
	delete propIface;
	delete deviceIface;
}

void RemovableDisk::deviceRemoved(const QDBusObjectPath path)
{
	/* NOTE: We cannot query the properties of the Object here, because the object is removed
             and does not exist anymore. A call "Get" or "GetAll" on "org.freedesktop.DBus.Properties" 
			 would fail on this object. */
	qDebug() << "Device removed: " << path.path();

	QDBusInterface *deviceIface = new QDBusInterface("org.freedesktop.DeviceKit.Disks", 
		path.path(), 
		"org.freedesktop.DeviceKit.Disks.Device", 
		QDBusConnection::systemBus());

		QMapIterator<QString, QString> it(mounts);
		while(it.hasNext()) {
			it.next();
			if(it.key() == path.path()) {
				qDebug() << "Unmounting...";
				QDBusReply<QString> reply = deviceIface->call("FilesystemUnmount", (QStringList)"force");
				qDebug() << "Unmounted: " << it.value(); 
				mounts.remove(path.path());
			}
		}
}

RemovableDisk::RemovableDisk()
{
	if (!QDBusConnection::systemBus().isConnected()) {
		qDebug() << "Cannot connect to the D-Bus system bus.";
    }
	
	qDebug() << "Connected to the D-Bus system bus successful.";

	/* It seems that the service "org.freesesktop.DeviceKitDisks" is not registered unitl device enumeration.
	   Sequence is taken from "clementine-0.7.1", "src/devices/devicekitlistener", "DeviceKitLister::Init()". */
	disksIface = new QDBusInterface("org.freedesktop.DeviceKit.Disks", 
		"/org/freedesktop/DeviceKit/Disks", 
		"org.freedesktop.DeviceKit.Disks", 
		QDBusConnection::systemBus());
		
    QDBusPendingReply<QList<QDBusObjectPath> > reply = disksIface->call("EnumerateDevices");
	reply.waitForFinished();
    if (!reply.isValid()) {
		qDebug() << "Call failed: " << reply.error().message();
	}
 
	QDBusConnection::systemBus().connect(
		"org.freedesktop.DeviceKit.Disks",
		"/org/freedesktop/DeviceKit/Disks",
		"org.freedesktop.DeviceKit.Disks",
		"DeviceAdded",
		this, SLOT(deviceAdded(QDBusObjectPath)));

	QDBusConnection::systemBus().connect(
		"org.freedesktop.DeviceKit.Disks",
		"/org/freedesktop/DeviceKit/Disks",
		"org.freedesktop.DeviceKit.Disks",
		"DeviceRemoved",
		this, SLOT(deviceRemoved(QDBusObjectPath)));
}

RemovableDisk::~RemovableDisk()
{
	delete disksIface;
}
