#include "AutoWebSocket.h"

AutoWebSocket::AutoWebSocket(const QUrl& url, const QWebSocketProtocol::Version& version, QObject* parent)
	: url(url), _socket("", version), QObject(parent)
{
	reconnectTimer = new QTimer(this);
	reconnectTimer->setInterval(reconnectTimerInterval);

	connect(this, &AutoWebSocket::connectSocket, this, &AutoWebSocket::doConnectSocket);
	connect(this, &AutoWebSocket::disconnectSocket, this, &AutoWebSocket::doDisconnectSocket);
	connect(reconnectTimer, &QTimer::timeout, this, &AutoWebSocket::onTimer);

	connect(&_socket, &QWebSocket::connected, this, &AutoWebSocket::onSocketConnected);
	connect(&_socket, &QWebSocket::disconnected, this, &AutoWebSocket::onSocketDisconnected);
}

AutoWebSocket::~AutoWebSocket()
{
	this->stop();
	_socket.close();
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
	return &_socket;
}

void AutoWebSocket::doConnectSocket()
{
	if (_socket.state() == QAbstractSocket::UnconnectedState)
	{
		_socket.open(url);
	}
}

void AutoWebSocket::doDisconnectSocket()
{
	if (_socket.state() == QAbstractSocket::ConnectedState)
	{
		_socket.close();
	}
}

void AutoWebSocket::onSocketConnected()
{
	qInfo() << "websocket connected";
}

void AutoWebSocket::onSocketDisconnected()
{
	qDebug() << "websocket disconnected";
}

void AutoWebSocket::onTimer()
{
	emit connectSocket();
}
