#pragma once
#include <qwidget.h>
#include <CameraLabel.h>
#include <qgridlayout.h>
#include <qrect.h>

class CameraViewWidget : public QWidget
{
	Q_OBJECT

public:
	explicit CameraViewWidget(QWidget* parent = nullptr);
	~CameraViewWidget();

	void setPixmap(const QPixmap& pixmap);

private slots:
	void onNewMask(const QRectF& maskNorm);

signals:
	void newMask(const QRectF& maskNorm);
	void setEnableMask(const bool enable);

private:
	QGridLayout* grid = nullptr;
	CameraLabel* label = nullptr;
};

