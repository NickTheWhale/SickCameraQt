/*****************************************************************//**
 * @file   LoggingWidget.cpp
 * @brief  Displays log messages using a LogTextEdit. Output's are buffered for performance.
 * 
 * @author Nicholas Loehrke
 * @date   August 2023
 *********************************************************************/

#include "LoggingWidget.h"
#include <qtextdocument.h>
#include <qtimer.h>

LoggingWidget::LoggingWidget(QWidget* parent) : buffer(bufferSize), QWidget(parent)
{
	grid = new QGridLayout(this);
	textEdit = new LogTextEdit(this);
	grid->addWidget(textEdit);

	// cyclic timer used to trigger processing message buffer
	QTimer* timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &LoggingWidget::processMessageBuffer);
	timer->start(processTimeout);
}

LoggingWidget::~LoggingWidget()
{
}

void LoggingWidget::setMaxLineCount(unsigned int maxLineCount)
{
	textEdit->setMaxLineCount(maxLineCount);
}

void LoggingWidget::showMessage(const QtMsgType level, const QString& message)
{
	// add the message to the buffer to process later
	buffer.push_back(std::make_pair(level, message));
}

void LoggingWidget::processMessageBuffer()
{
	// display all the messages in the buffer
	while (!buffer.empty())
	{
		const auto& logEntry = buffer.front();
		textEdit->showMessage(logEntry.first, logEntry.second);
		buffer.pop_front();
	}
}
