#pragma once
#include <qtoolbutton.h>

class QToolMenuButton : public QToolButton
{
	Q_OBJECT
		
public:
	explicit QToolMenuButton(QWidget* parent = nullptr);
	~QToolMenuButton();

private:
	QMenu* menu = nullptr;
	QActionGroup* actionGroup = nullptr;
};

