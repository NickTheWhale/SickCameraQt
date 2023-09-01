/*****************************************************************//**
 * @file   LogTextEdit.h
 * @brief  Scrolling text edit with pause-on-scroll and maximum line count.
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

	/**
	 * @brief Displays a log message.
	 *
	 * Displays a log message, clearing excess blocks if needed.
	 *
	 * @param level Log level.
	 * @param message Message to show.
	 */
	void showMessage(const QtMsgType level, const QString& message);

	void setMaxLineCount(unsigned int maxLineCount);

protected:
	/**
	 * @brief Right click menu callback.
	 *
	 * @param event Event.
	 */
	void contextMenuEvent(QContextMenuEvent* event) override;

private:
	/**
	 * @brief Mouse wheel callback. Used to pause or unpause scrolling.
	 *
	 * @param event
	 */
	void wheelEvent(QWheelEvent* event) override;

	/**
	 * @brief Converts given log level to a color.
	 * 
	 * @param Level Log level.
	 * @return Color.
	 */
	const QColor levelToColor(const QtMsgType level) const;

	/**
	 * @brief Converts given log level to a string representation.
	 * 
	 * @param Level Log level.
	 * @return Log level string.
	 */
	const QString levelToString(const QtMsgType level) const;

	/**
	 * @brief Removes oldest log lines if output is full.
	 * 
	 */
	void clearExcessBlocks();

	/**
	 * @brief Determines if the log output has more lines than maxLineCount.
	 * 
	 * @return True if current line count is greater than maxLineCount, false otherwise.
	 */
	const bool full() const;

	/**
	 * @brief Formats and writes message to output.
	 * 
	 * Message format: '[data + time] [color coded level] message'
	 * 
	 * @param level Log level.
	 * @param message Log message.
	 */
	void writeMessage(const QtMsgType level, const QString& message);

	/**
	 * @brief Clear and unpauses output.
	 * 
	 */
	void clear();

private:
	bool paused = false;
	unsigned int maxLineCount = 1000;
};

