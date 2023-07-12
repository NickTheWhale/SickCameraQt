#include "AutoWebSocket.h"

AutoWebSocket::AutoWebSocket(const QUrl& url, const QWebSocketProtocol::Version& version, QObject* parent)
	: url(url), QObject(parent)
{
	reconnectTimer = new QTimer(this);
	reconnectTimer->setInterval(reconnectTimerInterval);

	socket = new QWebSocket("", version, parent);

	connect(this, &AutoWebSocket::connectSocket, this, &AutoWebSocket::doConnectSocket);
	connect(this, &AutoWebSocket::disconnectSocket, this, &AutoWebSocket::doDisconnectSocket);
	connect(reconnectTimer, &QTimer::timeout, this, &AutoWebSocket::onTimer);

	connect(socket, &QWebSocket::connected, this, &AutoWebSocket::onSocketConnected);
	connect(socket, &QWebSocket::disconnected, this, &AutoWebSocket::onSocketDisconnected);
}

AutoWebSocket::~AutoWebSocket()
{
}

void AutoWebSocket::start()
{
	if (!reconnectTimer->isActive())
		reconnectTimer->start();
}

void AutoWebSocket::stop()
{
	if (reconnectTimer->isActive())
		reconnectTimer->start();
}

qint64 AutoWebSocket::sendBinaryMessage(const QByteArray& data)
{
	if (socket && socket->state() == QAbstractSocket::ConnectedState)
	{
		return socket->sendBinaryMessage(data);
	}
	return -1;
}

qint64 AutoWebSocket::sendTextMessage(const QString& message)
{
	if (socket && socket->state() == QAbstractSocket::ConnectedState)
	{
		return socket->sendTextMessage(message);
	}
	return -1;
}

void AutoWebSocket::doConnectSocket()
{
	qDebug() << "doConnectSocket";
	if (socket->state() == QAbstractSocket::UnconnectedState)
	{
		qDebug() << "connecting to websocket";
		socket->open(url);
	}
}

void AutoWebSocket::doDisconnectSocket()
{
	qDebug() << "doDisconnectSocket";
	if (socket->state() == QAbstractSocket::ConnectedState)
	{
		qDebug() << "disconnecting from websocket";
		socket->close();
	}
}

void AutoWebSocket::onSocketConnected()
{
	qDebug() << "websocket connected";
}

void AutoWebSocket::onSocketDisconnected()
{
	qDebug() << "websocket disconnected";
}

void AutoWebSocket::onTimer()
{
	emit connectSocket();
}
