#pragma once
#include <qwidget.h>
#include <AspectRatioPixmapLabel.h>
#include <qgridlayout.h>
#include <RangeSlider.h>

class CameraViewWidget : public QWidget
{
	Q_OBJECT

public:
	explicit CameraViewWidget(QWidget* parent = nullptr);
	~CameraViewWidget();

	void setPixmap(const QPixmap& pixmap);

signals:
	void setDepthFilterRange(const uint16_t low, const uint16_t high);
	void setDepthFilterEnable(const bool enable);
	void setDepthMaskEnable(const bool enable);

private:
	AspectRatioPixmapLabel* label = nullptr;
	QGridLayout* layout = nullptr;
	RangeSlider* slider = nullptr;
};

