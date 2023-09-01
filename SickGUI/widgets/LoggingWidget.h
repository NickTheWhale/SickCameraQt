/*****************************************************************//**
 * @file   LoggingWidget.h
 * @brief  Displays log messages using a LogTextEdit. Output's are buffered for performance.
 * 
 * @author Nicholas Loehrke
 * @date   August 2023
 *********************************************************************/

#pragma once
#include <qwidget.h>
#include <qtextedit.h>
#include <qgridlayout.h>
#include "LogTextEdit.h"
#include <boost/circular_buffer.hpp>

class LoggingWidget : public QWidget
{
	Q_OBJECT

public slots:
	/**
	 * @brief Adds message to internal buffer to be shown later.
	 */
	void showMessage(const QtMsgType level, const QString& message);

public:
	explicit LoggingWidget(QWidget* parent = nullptr);
	~LoggingWidget();

	void setMaxLineCount(unsigned int maxLineCount);

private slots:
	/**
	 * @brief Cyclically called to show each message in the internal buffer.
	 */
	void processMessageBuffer();

private:
	LogTextEdit* textEdit = nullptr;
	QGridLayout* grid = nullptr;
	const unsigned int bufferSize = 25;
	boost::circular_buffer<std::pair<QtMsgType, QString>> buffer;
	const int processTimeout = 10;
};

