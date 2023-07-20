#pragma once
#include <qobject.h>
#include <qwebsocket.h>
#include <qtimer.h>

class AutoWebSocket : public QObject
{
	Q_OBJECT

public:
	explicit AutoWebSocket(const QUrl& url, const QWebSocketProtocol::Version& version = QWebSocketProtocol::VersionLatest, QObject* parent = nullptr);
	explicit AutoWebSocket(const QUrl& url, QObject* parent = nullptr);
	~AutoWebSocket();

	void start();
	void stop();
	QWebSocket* socket();

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
	QWebSocket _socket;
	QTimer reconnectTimer;

	const QUrl url;
	const int reconnectTimerInterval = 5000; /* ms */
};

