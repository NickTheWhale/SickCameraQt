/*****************************************************************//**
 * @file   CustomMessageHandler.cpp
 * @brief  Used as a bridge to redirect qDebug(), qWarning(), etc macro outputs.
 * 
 * @author Nicholas Loehrke
 * @date   August 2023
 *********************************************************************/

#include "CustomMessageHandler.h"

CustomMessageHandler::CustomMessageHandler(QObject* parent) : QObject(parent)
{
}

void CustomMessageHandler::handle(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
	switch (type)
	{
	case QtDebugMsg:
		emit newMessage(QtDebugMsg, message);
		break;
	case QtInfoMsg:
		emit newMessage(QtInfoMsg, message);
		break;
	case QtWarningMsg:
		emit newMessage(QtWarningMsg, message);
		break;
	case QtCriticalMsg:
		emit newMessage(QtCriticalMsg, message);
		break;
	case QtFatalMsg:
		emit newMessage(QtFatalMsg, message);
		break;
	}
}