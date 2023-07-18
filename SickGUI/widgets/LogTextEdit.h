#pragma once
#include <qtextedit.h>
class LogTextEdit : public QTextEdit
{
	Q_OBJECT

public:
	explicit LogTextEdit(QWidget* parent = nullptr);

	void showMessage(const QtMsgType level, const QString& message);
	void setMaxLineCount(unsigned int maxLineCount);
	void wheelEvent(QWheelEvent* event) override;

private:
	bool paused = false;
	unsigned int maxLineCount = 1000;

	const QColor levelToColor(const QtMsgType level) const;
	const QString levelToString(const QtMsgType level) const;
	void clearExcessBlocks();
	const bool full() const;
	void writeMessage(const QtMsgType level, const QString& message);
};

