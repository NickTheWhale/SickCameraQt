#pragma once
#include <qwidget.h>
#include <qtextedit.h>
#include <qgridlayout.h>

class LoggingWidget : public QWidget
{
public:
	enum Level 
	{
		Info,
		Warning,
		Error
	};

	explicit LoggingWidget(QWidget* parent = nullptr);
	~LoggingWidget();

	void logInfo(const QString& message);
	void logWarning(const QString& message);
	void logError(const QString& message);

	void setMaxLineCount(unsigned int maxLineCount);

private:
	unsigned int maxLineCount = 1000;
	QTextEdit* textEdit = nullptr;
	QGridLayout* grid = nullptr;

	void log(Level level, const QString& message);
};

