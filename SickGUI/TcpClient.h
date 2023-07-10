#pragma once
#include <qobject.h>
#include <qtcpsocket.h>
#include <qtimer.h>

class TcpClient : public QObject
{
	Q_OBJECT

public:
	explicit TcpClient(const QString &hostAddress, quint16 hostPort, QObject* parent = nullptr);
	~TcpClient();

	void start();
	void stop();
	qint64 write(const char* data, qint64 maxSize);
	qint64 write(const char* data);
	qint64 write(const QByteArray& data);

signals:
	void connectServer();
	void disconnectServer();

private slots:
	void doConnectServer();
	void doDisconnectServer();
	void onSocketConnected();
	void onSocketDisconnected();
	void onTimer();

private:
	QTcpSocket* socket = nullptr;
	QTimer* reconnectTimer = nullptr;

	const QString hostAddress;
	const quint16 hostPort;
	const int reconnectTimerInterval = 5000; /* ms */
};

