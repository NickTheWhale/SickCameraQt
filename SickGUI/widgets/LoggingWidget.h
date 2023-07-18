#pragma once
#include <qwidget.h>
#include <qtextedit.h>
#include <qgridlayout.h>
#include "LogTextEdit.h"

class LoggingWidget : public QWidget
{
	Q_OBJECT

public slots:
	void showMessage(const QtMsgType level, const QString& message);

public:
	explicit LoggingWidget(QWidget* parent = nullptr);
	~LoggingWidget();

	void setMaxLineCount(unsigned int maxLineCount);

private:
	LogTextEdit* textEdit = nullptr;
	QGridLayout* grid = nullptr;
};

