#pragma once
#include <qdockwidget.h>
#include <qstring.h>
#include <qwidget.h>

class CloseDockWidget : public QDockWidget
{
	Q_OBJECT

public:
	explicit CloseDockWidget(const QString& title, QWidget* parent = nullptr);
	~CloseDockWidget();

protected:
	void closeEvent(QCloseEvent* event) override;

signals:
	void closed();
};

