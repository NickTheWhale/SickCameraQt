#include "QAutoWebSocket.h"

QAutoWebSocket::QAutoWebSocket(const QUrl& url, const QString& rubberBandOrigin, const QWebSocketProtocol::Version& version, QObject* parent)
	: url(url), reconnectTimer(QTimer()), QWebSocket(rubberBandOrigin, version, parent)
{
	reconnectTimer.setInterval(reconnectTimerInterval);
	makeConnections();
}

QAutoWebSocket::QAutoWebSocket(const QUrl& url, const QWebSocketProtocol::Version& version, QObject* parent)
	: url(url), reconnectTimer(QTimer()), QWebSocket(QString(), version, parent)
{
	reconnectTimer.setInterval(reconnectTimerInterval);
	makeConnections();
}

QAutoWebSocket::QAutoWebSocket(const QUrl& url, QObject* parent)
	: url(url), reconnectTimer(QTimer()), QWebSocket(QString(), QWebSocketProtocol::VersionLatest, parent)
{
	reconnectTimer.setInterval(reconnectTimerInterval);
	makeConnections();
}

QAutoWebSocket::~QAutoWebSocket()
{
	stop();
	close();
}

void QAutoWebSocket::start()
{
	//qDebug() << "websocket: start()";
	if (!reconnectTimer.isActive())
		reconnectTimer.start();
}

void QAutoWebSocket::stop()
{
	//qDebug() << "websocket: stop()";
	reconnectTimer.stop();
}

void QAutoWebSocket::doConnectSocket()
{
	//qDebug() << "websocket: doConnectSocket()";
	if (state() == QAbstractSocket::UnconnectedState)
	{
		open(url);
	}
}

void QAutoWebSocket::doDisconnectSocket()
{
	//qDebug() << "websocket: doDisconnectSocket()";
	if (state() == QAbstractSocket::ConnectedState)
	{
		close();
	}
}

void QAutoWebSocket::onSocketConnected()
{
	qInfo() << "websocket: onSocketConnected";
}

void QAutoWebSocket::onSocketDisconnected()
{
	//qDebug() << "websocket: onSocketDisconnected";
}

void QAutoWebSocket::onConnectTimer()
{
	//qDebug() << "websocket: onConnectTimer()";
	emit connectSocket();
}

void QAutoWebSocket::makeConnections()
{
	connect(this, &QAutoWebSocket::connectSocket, this, &QAutoWebSocket::doConnectSocket);
	connect(this, &QAutoWebSocket::disconnectSocket, this, &QAutoWebSocket::doDisconnectSocket);
	connect(&reconnectTimer, &QTimer::timeout, this, &QAutoWebSocket::onConnectTimer);

	connect(this, &QWebSocket::connected, this, &QAutoWebSocket::onSocketConnected);
	connect(this, &QWebSocket::disconnected, this, &QAutoWebSocket::onSocketDisconnected);
}
