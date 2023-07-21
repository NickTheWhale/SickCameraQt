#include "LoggingWidget.h"
#include <qtextdocument.h>
#include <qtimer.h>

LoggingWidget::LoggingWidget(QWidget* parent) : buffer(bufferSize), QWidget(parent)
{
	grid = new QGridLayout(this);
	textEdit = new LogTextEdit(this);
	grid->addWidget(textEdit);

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
	buffer.push_back(std::make_pair(level, message));
}

void LoggingWidget::processMessageBuffer()
{
	while (!buffer.empty())
	{
		const auto& logEntry = buffer.front();
		textEdit->showMessage(logEntry.first, logEntry.second);
		buffer.pop_front();
	}
}
