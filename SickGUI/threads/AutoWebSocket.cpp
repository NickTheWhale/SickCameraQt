#include "AutoWebSocket.h"

AutoWebSocket::AutoWebSocket(const QUrl& url, const QWebSocketProtocol::Version& version, QObject* parent)
	: url(url), _socket(QWebSocket("", version)), reconnectTimer(new QTimer(parent)), QObject(parent)
{
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
	qDebug() << "start";
	if (!reconnectTimer->isActive())
		reconnectTimer->start();
}

void AutoWebSocket::stop()
{
	qDebug() << "stop";
	if (reconnectTimer->isActive())
		reconnectTimer->stop();
}

QWebSocket* AutoWebSocket::socket()
{
	return &_socket;
}

QTimer* AutoWebSocket::timer()
{
	return reconnectTimer;
}


qint64 AutoWebSocket::remainingTime()
{
	return reconnectTimer->remainingTime();
}

void AutoWebSocket::doConnectSocket()
{
	qDebug() << "doConnect";
	if (_socket.state() == QAbstractSocket::UnconnectedState)
	{
		_socket.open(url);
	}
}

void AutoWebSocket::doDisconnectSocket()
{
	qDebug() << "doDisconnect";
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
	qDebug() << "onTimer";
	emit connectSocket();
}
