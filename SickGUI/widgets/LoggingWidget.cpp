#include "LoggingWidget.h"
#include <qdatetime.h>

LoggingWidget::LoggingWidget(QWidget* parent) : QWidget(parent)
{
	grid = new QGridLayout(this);

	textEdit = new QTextEdit(this);
	textEdit->setReadOnly(true);
	textEdit->setLineWrapMode(QTextEdit::NoWrap);
	textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	grid->addWidget(textEdit);
}

LoggingWidget::~LoggingWidget()
{
}

void LoggingWidget::logInfo(const QString& message)
{
	log(Level::Info, message);
}

void LoggingWidget::logWarning(const QString& message)
{
	log(Level::Warning, message);
}

void LoggingWidget::logError(const QString& message)
{
	log(Level::Error, message);
}

void LoggingWidget::setMaxLineCount(unsigned int maxLineCount)
{
	this->maxLineCount = maxLineCount;
}

void LoggingWidget::log(Level level, const QString& message)
{
	QDateTime time = QDateTime::currentDateTime();
	QString preamble = "[" + time.toString() + "] ";
	
	switch (level)
	{
	case LoggingWidget::Info:
		preamble += "[Info   ]";
		break;
	case LoggingWidget::Warning:
		preamble += "[Warning]";
		break;
	case LoggingWidget::Error:
		preamble += "[Error  ]";
		break;
	default:
		break;
	}

	textEdit->append(preamble + " " + message);
	textEdit->ensureCursorVisible();


	if (textEdit->document()->lineCount() > maxLineCount)
	{
		QTextCursor cursor = textEdit->textCursor();
		cursor.movePosition(QTextCursor::Start);
		cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, textEdit->document()->lineCount() - maxLineCount);
		cursor.removeSelectedText();
		textEdit->setTextCursor(cursor);
	}
}
