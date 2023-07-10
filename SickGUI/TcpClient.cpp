#include "TcpClient.h"

TcpClient::TcpClient(const QString& hostAddress, quint16 hostPort, QObject* parent)
	: hostAddress(hostAddress), hostPort(hostPort), QObject(parent)
{
	reconnectTimer = new QTimer(this);
	reconnectTimer->setInterval(reconnectTimerInterval);

	socket = new QTcpSocket(this);

	connect(this, &TcpClient::connectServer, this, &TcpClient::doConnectServer);
	connect(this, &TcpClient::disconnectServer, this, &TcpClient::doDisconnectServer);
	connect(reconnectTimer, &QTimer::timeout, this, &TcpClient::onTimer);

	connect(socket, &QTcpSocket::connected, this, &TcpClient::onSocketConnected);
	connect(socket, &QTcpSocket::disconnected, this, &TcpClient::onSocketDisconnected);
}

TcpClient::~TcpClient()
{
}

void TcpClient::start()
{
	if (!reconnectTimer->isActive())
		reconnectTimer->start();
}

void TcpClient::stop()
{
	if (reconnectTimer->isActive())
		reconnectTimer->stop();
}

qint64 TcpClient::write(const char* data, qint64 maxSize)
{
	if (socket && socket->state() == QAbstractSocket::ConnectedState)
		return socket->write(data, maxSize);
	return -1;
}

qint64 TcpClient::write(const char* data)
{
	if (socket && socket->state() == QAbstractSocket::ConnectedState)
		return socket->write(data);
	return -1;
}

qint64 TcpClient::write(const QByteArray& data)
{
	if (socket && socket->state() == QAbstractSocket::ConnectedState)
		return socket->write(data);
	return -1;
}

void TcpClient::doConnectServer()
{
	qDebug() << "doConnectServer";
	if (socket->state() == QAbstractSocket::UnconnectedState)
	{
		qDebug() << "connecting to server";
		socket->connectToHost(hostAddress, hostPort);
	}
}

void TcpClient::doDisconnectServer()
{
	qDebug() << "doDisconnectServer";
	if (socket->state() == QAbstractSocket::ConnectedState)
	{
		qDebug() << "disconnecting to server";
		socket->close();
	}
}

void TcpClient::onSocketConnected()
{
	qDebug() << "Socket connected";
}

void TcpClient::onSocketDisconnected()
{
	qDebug() << "Socket disconnected";
}

void TcpClient::onTimer()
{
	emit connectServer();
}
