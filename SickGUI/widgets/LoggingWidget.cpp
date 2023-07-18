#include "LoggingWidget.h"
#include <qdatetime.h>
#include <qtextdocument.h>
#include <qscrollbar.h>
#include <qpushbutton.h>
#include <qevent.h>

LoggingWidget::LoggingWidget(QWidget* parent) : QWidget(parent)
{
	grid = new QGridLayout(this);

	textEdit = new LogTextEdit(this);

	grid->addWidget(textEdit);
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
	textEdit->showMessage(level, message);
}
