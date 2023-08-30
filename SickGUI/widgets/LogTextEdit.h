/*****************************************************************//**
 * @file   LogTextEdit.h
 * @brief  Scrolling text edit with pause support and maximum line count.
 * 
 * @author Nicholas Loehrke
 * @date   August 2023
 *********************************************************************/

#pragma once
#include <qtextedit.h>
class LogTextEdit : public QTextEdit
{
	Q_OBJECT

public:
	explicit LogTextEdit(QWidget* parent = nullptr);

	void showMessage(const QtMsgType level, const QString& message);
	void setMaxLineCount(unsigned int maxLineCount);

protected:
	void contextMenuEvent(QContextMenuEvent* event) override;

private:
	void wheelEvent(QWheelEvent* event) override;
	bool paused = false;
	unsigned int maxLineCount = 1000;

	const QColor levelToColor(const QtMsgType level) const;
	const QString levelToString(const QtMsgType level) const;
	void clearExcessBlocks();
	const bool full() const;
	void writeMessage(const QtMsgType level, const QString& message);
	void clear();
};

