/*****************************************************************//**
 * @file   LogTextEdit.cpp
 * @brief  Scrolling text edit with pause-on-scroll and maximum line count.
 * 
 * @author Nicholas Loehrke
 * @date   August 2023
 *********************************************************************/

#include "LogTextEdit.h"
#include <qevent.h>
#include <qscrollbar.h>
#include <qdatetime.h>
#include <qmenu.h>

LogTextEdit::LogTextEdit(QWidget* parent) : QTextEdit(parent)
{
	// we don't want the user to be able to edit the logs
	setReadOnly(true);
	setLineWrapMode(QTextEdit::LineWrapMode::WidgetWidth);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

void LogTextEdit::setMaxLineCount(unsigned int maxLineCount)
{
	this->maxLineCount = maxLineCount;
}

void LogTextEdit::contextMenuEvent(QContextMenuEvent* event)
{
	QMenu menu(this);
	QAction* clearAction = menu.addAction("Clear");
	QAction* selectedAction = menu.exec(event->globalPos());
	if (selectedAction == clearAction)
	{
		this->clear();
	}
}

void LogTextEdit::wheelEvent(QWheelEvent* event)
{
	QTextEdit::wheelEvent(event);

	const int rawDelta = event->angleDelta().y();
	// get scroll wheel amount
	const int delta = event->inverted() ? -rawDelta : rawDelta;

	// if the user scrolled up, pause
	if (delta > 0)
	{
		paused = true;
	}
	// if the user scrolled down and the scrollbar is at the bottom, unpause
	else
	{
		const int value = verticalScrollBar()->value();
		const int max = verticalScrollBar()->maximum();
		if (value >= max)
		{
			paused = false;
		}
	}
}

const QColor LogTextEdit::levelToColor(const QtMsgType level) const
{
	// converts a log level to a color
	QColor color;
	switch (level)
	{
	case QtDebugMsg:
		color = { 134, 202, 60 };
		break;
	case QtInfoMsg:
		color = { 0, 176, 240 };
		break;
	case QtWarningMsg:
		color = { 255, 192, 0 };
		break;
	case QtCriticalMsg:
		color = { 217, 104, 13 };
		break;
	case QtFatalMsg:
		color = { 255, 0, 0 };
		break;
	default:
		color = { 0, 0, 0 };
		break;
	}
	return color;
}

const QString LogTextEdit::levelToString(const QtMsgType level) const
{
	// converts a log level to a string representation
	QString levelStr;
	switch (level)
	{
	case QtDebugMsg:
		levelStr = "DEBUG";
		break;
	case QtInfoMsg:
		levelStr = "INFO";
		break;
	case QtWarningMsg:
		levelStr = "WARNING";
		break;
	case QtCriticalMsg:
		levelStr = "CRITICAL";
		break;
	case QtFatalMsg:
		levelStr = "FATAL";
		break;
	default:
		levelStr = "UNKNOWN";
		break;
	}
	return levelStr;
}

void LogTextEdit::clearExcessBlocks()
{
	// if the output is full, clear the oldest logs
	if (full())
	{
		QTextCursor cursor(document());
		cursor.movePosition(QTextCursor::Start);
		cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, document()->blockCount() - maxLineCount);
		cursor.removeSelectedText();
	}
}

const bool LogTextEdit::full() const
{
	// true if log count is greater than the max line count
	return document()->blockCount() > maxLineCount;
}

void LogTextEdit::writeMessage(const QtMsgType level, const QString& message)
{
	const QColor levelColor = levelToColor(level).darker(130); // 30% darker looks better
	const QString levelStr = levelToString(level);
	moveCursor(QTextCursor::End);

	// write time (black)
	setTextColor(Qt::black);
	insertPlainText(QString("[%1] [").arg(QDateTime::currentDateTime().toString()));

	// write level (color)
	setTextColor(levelColor);
	insertPlainText(QString("%1").arg(levelStr));

	// write message (black)
	setTextColor(Qt::black);
	insertPlainText("] " + message + "\n");
}

void LogTextEdit::clear()
{
	QTextEdit::clear();
	paused = false;
}

void LogTextEdit::showMessage(const QtMsgType level, const QString& message)
{
	const int prevScrollPosition = verticalScrollBar()->value();

	if (paused && full())				//  Paused  |  Full  |  Action
	{									// ----------------------------
		               					//   True   |  True  |  do nothing
		return;							//   True   |  False |  add text, keep scroll position
	}									//   False  |  True  |  add text, remove excess blocks
										//   False  |  False |  add text
	else if (paused && !full())
	{
		const int prevScrollPosition = verticalScrollBar()->value();
		writeMessage(level, message);
		verticalScrollBar()->setValue(prevScrollPosition);
	}

	else if (!paused && full())
	{
		clearExcessBlocks();
		writeMessage(level, message);
		ensureCursorVisible();
	}

	else
	{
		writeMessage(level, message);
	}
}
