/*****************************************************************//**
 * @file   CustomMessageHandler.h
 * @brief  Used as a bridge to redirect qDebug(), qWarning(), etc macro outputs.
 * 
 * @author Nicholas Loehrke
 * @date   August 2023
 *********************************************************************/

#pragma once

#include <qobject.h>
#include <qdebug>

class CustomMessageHandler : public QObject
{
	Q_OBJECT

public:
	explicit CustomMessageHandler(QObject* parent = nullptr);
	void handle(QtMsgType type, const QMessageLogContext& context, const QString& message);

signals:
	void newMessage(const QtMsgType level, const QString& message);
};
