#include "CloseDockWidget.h"

CloseDockWidget::CloseDockWidget(const QString& title, QWidget* parent) : QDockWidget(title, parent)
{
}

CloseDockWidget::~CloseDockWidget()
{
}

void CloseDockWidget::closeEvent(QCloseEvent* event)
{
	emit closed();

	QDockWidget::closeEvent(event);
}
