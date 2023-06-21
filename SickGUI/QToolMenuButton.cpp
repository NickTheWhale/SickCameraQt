#include "QToolMenuButton.h"

#include <qmenu.h>
#include <qactiongroup.h>

QToolMenuButton::QToolMenuButton(QWidget* parent)
{
	menu = new QMenu(this);
	actionGroup = new QActionGroup(this);

	this->setMenu(menu);
	this->setPopupMode(QToolButton::InstantPopup);
	this->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
}

QToolMenuButton::~QToolMenuButton()
{
}
