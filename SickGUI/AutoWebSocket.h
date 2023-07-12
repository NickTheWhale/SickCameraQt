#pragma once
#include <qobject.h>
#include <qwebsocket.h>
#include <qtimer.h>

class AutoWebSocket : public QObject
{
	Q_OBJECT

public:
	explicit AutoWebSocket(const QUrl& url, const QWebSocketProtocol::Version& version, QObject* parent = nullptr);
	~AutoWebSocket();

	void start();
	void stop();
	qint64 sendBinaryMessage(const QByteArray& data);
	qint64 sendTextMessage(const QString& message);

signals:
	void connectSocket();
	void disconnectSocket();

private slots:
	void doConnectSocket();
	void doDisconnectSocket();
	void onSocketConnected();
	void onSocketDisconnected();
	void onTimer();

private:
	QWebSocket* socket = nullptr;
	QTimer* reconnectTimer = nullptr;

	const QUrl url;
	const int reconnectTimerInterval = 5000; /* ms */
};

