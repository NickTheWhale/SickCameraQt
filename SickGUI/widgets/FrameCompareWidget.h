#pragma once
#include <qwidget.h>
#include <Frameset.h>
#include <ImageLabel.h>
#include "..\TinyColormap.hpp"
#include <qspinbox.h>
#include <ThreadInterface.h>

class ImageCompareWidget;

class FrameCompareWidget : public QWidget
{
	Q_OBJECT

public:
	explicit FrameCompareWidget(QWidget* parent = nullptr);
	~FrameCompareWidget();

private:
	const int continuousModeInterval = 100; /* ms */
	const uint16_t upperThreshold = 1000; /* mm */
	Frameset::FramesetType refFs;
	Frameset::FramesetType curFs;
	ImageLabel* refImageLabel = nullptr;
	ImageLabel* difImageLabel = nullptr;

	bool gridCompare = false;

	QSpinBox* thresholdSpinBox = nullptr;
	tinycolormap::ColormapType colorMap = tinycolormap::ColormapType::Turbo;
	ThreadInterface& threadInterface;

	void getReferenceSnapshot();
	void getCurrentSnapshot();
	void compare();
};