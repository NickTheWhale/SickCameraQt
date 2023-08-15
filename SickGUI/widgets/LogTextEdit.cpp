#include "LogTextEdit.h"
#include <qevent.h>
#include <qscrollbar.h>
#include <qdatetime.h>
#include <qmenu.h>

LogTextEdit::LogTextEdit(QWidget* parent) : QTextEdit(parent)
{
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

	int rawDelta = event->angleDelta().y();
	int delta = event->inverted() ? -rawDelta : rawDelta;


	if (delta > 0)
	{
		paused = true;
	}
	else
	{
		int value = verticalScrollBar()->value();
		int max = verticalScrollBar()->maximum();
		if (value >= max)
		{
			paused = false;
		}
	}
}

const QColor LogTextEdit::levelToColor(const QtMsgType level) const
{
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
	return document()->blockCount() > maxLineCount;
}

void LogTextEdit::writeMessage(const QtMsgType level, const QString& message)
{
	QColor levelColor = levelToColor(level).darker(130); // 30% darker looks better
	QString levelStr = levelToString(level);
	moveCursor(QTextCursor::End);

	// time (black)
	setTextColor(Qt::black);
	insertPlainText(QString("[%1] [").arg(QDateTime::currentDateTime().toString()));

	// level (color)
	setTextColor(levelColor);
	insertPlainText(QString("%1").arg(levelStr));

	// message (black)
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
	int prevScrollPosition = verticalScrollBar()->value();
	QColor levelColor = levelToColor(level).darker(130); // 30% darker looks better
	QString levelStr = levelToString(level);


	if (paused && full())				//  Paused  |  Full  |  Action
	{									// ----------------------------
		               					//   True   |  True  |  do nothing
		return;							//   True   |  False |  add text, keep scroll position
	}									//   False  |  True  |  add text, remove excess blocks
										//   False  |  False |  add text
	else if (paused && !full())
	{
		int prevScrollPosition = verticalScrollBar()->value();
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
