#include "SickViewer.h"
#include <qwebengineview.h>
#include <qwebsocket.h>

SickViewer::SickViewer(QWidget *parent) : QMainWindow(parent)
{
    ui.setupUi(this);

    QWebEngineView* webView = new QWebEngineView(this);
    setCentralWidget(webView);

    QWebSocket* client = new QWebSocket("", QWebSocketProtocol::VersionLatest, this);

    QObject::connect(client, &QWebSocket::connected, []()
        {
            qDebug() << "Connected to WebSocket server";
        });

    QObject::connect(client, &QWebSocket::disconnected, []()
        {
            qDebug() << "Disconnected from WebSocket server";
        });

    QObject::connect(client, &QWebSocket::textMessageReceived, this, [webView]()
        {
            if (webView)
            {
                QString message = "message";
                webView->page()->runJavaScript("document.body.innerHTML += '<p>" + message + "</p>';");
            }
        });
}

SickViewer::~SickViewer()
{}
