#include <QtCore/QCoreApplication>
#include <QNetworkProxy>
#include <QSslSocket>
#include <QDebug>
#include <QFile>
#include <QSslCertificate>

int main(int argc, char * argv[]){
 QCoreApplication app(argc, argv);
 QSslSocket sslSocket;

  sslSocket.connectToHost("signin.ebay.com", 443);
  if (!sslSocket.waitForConnected()) {
	qDebug() << sslSocket.errorString();
	return false;
  }
  qDebug() << "Connected!";

  sslSocket.startClientEncryption();
  if (!sslSocket.waitForEncrypted(2000)) {
	qDebug() << sslSocket.errorString();
	return false;
  }

  sslSocket.write("GET / HTTP/1.0\r\n\r\n");
  while (sslSocket.waitForReadyRead())
	qDebug() << sslSocket.readAll().data();

}