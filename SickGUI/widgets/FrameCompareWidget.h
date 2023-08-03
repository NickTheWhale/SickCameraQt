#pragma once
#include <qwidget.h>
#include <Frameset.h>
#include <ImageLabel.h>
#include <TinyColormap.hpp>
#include <qspinbox.h>
#include <ThreadInterface.h>
#include <qcheckbox.h>

constexpr uint32_t downsampleHeight = 50;
constexpr uint32_t downsampleWidth = 100;

class FrameCompareWidget : public QWidget
{
	Q_OBJECT

public:
	explicit FrameCompareWidget(QWidget* parent = nullptr);
	~FrameCompareWidget();

private:
	const int continuousModeInterval = 100; /* ms */
	frameset::Frameset refFs;
	frameset::Frameset curFs;
	ImageLabel* refImageLabel = nullptr;
	ImageLabel* difImageLabel = nullptr;

	QCheckBox* continuousModeCheckBox = nullptr;
	QSpinBox*  lowerThresholdSpinBox = nullptr;
	QSpinBox*  upperThresholdSpinBox = nullptr;
	QCheckBox* autoExposureCheckBox = nullptr;
	QSpinBox*  lowerExposureSpinBox = nullptr;
	QSpinBox*  upperExposureSpinBox = nullptr;

	uint16_t lowerThreshold;
	uint16_t upperThreshold;
	uint16_t lowerExposure;
	uint16_t upperExposure;

	const uint16_t maxUpper = std::numeric_limits<uint16_t>::max();

	tinycolormap::ColormapType colorMap = tinycolormap::ColormapType::Turbo;
	ThreadInterface& threadInterface;

	void getReferenceSnapshot();
	void getCurrentSnapshot();
	void compare();
};