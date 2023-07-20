#pragma once
#include <qwebsocket.h>
#include <qtimer.h>

class QAutoWebSocket : public QWebSocket
{
	Q_OBJECT

signals:
	void connectSocket();
	void disconnectSocket();

public:
	explicit QAutoWebSocket(const QUrl& url, const QString& origin, const QWebSocketProtocol::Version& version = QWebSocketProtocol::VersionLatest, QObject* parent = nullptr);
	explicit QAutoWebSocket(const QUrl& url, const QWebSocketProtocol::Version& version = QWebSocketProtocol::VersionLatest, QObject* parent = nullptr);
	explicit QAutoWebSocket(const QUrl& url, QObject* parent = nullptr);
	~QAutoWebSocket();

	void start();
	void stop();

private slots:
	void doConnectSocket();
	void doDisconnectSocket();
	void onSocketConnected();
	void onSocketDisconnected();
	void onConnectTimer();

private:
	const QUrl url;
	QTimer reconnectTimer;
	const int reconnectTimerInterval = 500; /* ms */

	void makeConnections();
};

