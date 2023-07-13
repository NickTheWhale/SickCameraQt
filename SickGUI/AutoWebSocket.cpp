#include "AutoWebSocket.h"

AutoWebSocket::AutoWebSocket(const QUrl& url, const QWebSocketProtocol::Version& version, QObject* parent)
	: url(url), QObject(parent)
{
	reconnectTimer = new QTimer(this);
	reconnectTimer->setInterval(reconnectTimerInterval);

	_socket = new QWebSocket("", version, parent);

	connect(this, &AutoWebSocket::connectSocket, this, &AutoWebSocket::doConnectSocket);
	connect(this, &AutoWebSocket::disconnectSocket, this, &AutoWebSocket::doDisconnectSocket);
	connect(reconnectTimer, &QTimer::timeout, this, &AutoWebSocket::onTimer);

	connect(_socket, &QWebSocket::connected, this, &AutoWebSocket::onSocketConnected);
	connect(_socket, &QWebSocket::disconnected, this, &AutoWebSocket::onSocketDisconnected);
}

AutoWebSocket::~AutoWebSocket()
{
	this->stop();
	_socket->close();
}

void AutoWebSocket::start()
{
	if (!reconnectTimer->isActive())
		reconnectTimer->start();
}

void AutoWebSocket::stop()
{
	if (reconnectTimer->isActive())
		reconnectTimer->stop();
}

QWebSocket* AutoWebSocket::socket()
{
	return _socket;
}

void AutoWebSocket::doConnectSocket()
{
	qDebug() << "doConnectSocket";
	if (_socket->state() == QAbstractSocket::UnconnectedState)
	{
		qDebug() << "connecting to websocket";
		_socket->open(url);
	}
}

void AutoWebSocket::doDisconnectSocket()
{
	qDebug() << "doDisconnectSocket";
	if (_socket->state() == QAbstractSocket::ConnectedState)
	{
		qDebug() << "disconnecting from websocket";
		_socket->close();
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
