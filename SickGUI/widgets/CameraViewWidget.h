#pragma once
#include <qwidget.h>
#include <CameraLabel.h>
#include <qgridlayout.h>

class CameraViewWidget : public QWidget
{
	Q_OBJECT

public:
	explicit CameraViewWidget(QWidget* parent = nullptr);
	~CameraViewWidget();

	void setPixmap(const QPixmap& pixmap);

private:
	QGridLayout* grid = nullptr;
	CameraLabel* label = nullptr;
};

