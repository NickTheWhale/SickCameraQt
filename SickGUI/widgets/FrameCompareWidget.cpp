#include "FrameCompareWidget.h"
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qsplitter.h>
#include <qformlayout.h>

FrameCompareWidget::FrameCompareWidget(QWidget* parent) :
	QWidget(parent),
	refImageLabel(new ImageLabel(this)),
	difImageLabel(new ImageLabel(this)),
	continuousModeCheckBox(new QCheckBox(this)),
	lowerThresholdSpinBox(new QSpinBox(this)),
	upperThresholdSpinBox(new QSpinBox(this)),
	autoExposureCheckBox(new QCheckBox(this)),
	lowerExposureSpinBox(new QSpinBox(this)),
	upperExposureSpinBox(new QSpinBox(this)),
	threadInterface(ThreadInterface::instance()),
	lowerThreshold(0),
	upperThreshold(std::numeric_limits<uint16_t>::max()),
	lowerExposure(0),
	upperExposure(std::numeric_limits<uint16_t>::max())
{
	// make stuff
	QHBoxLayout* hboxMain = new QHBoxLayout(this);
	QSplitter* splitter = new QSplitter(this);
	splitter->setOrientation(Qt::Orientation::Horizontal);
	QVBoxLayout* vboxReference = new QVBoxLayout();
	QVBoxLayout* vboxDifference = new QVBoxLayout();
	QFormLayout* formSettings = new QFormLayout();

	QPushButton* referenceSnapshotButton = new QPushButton("Snapshot", this);
	QPushButton* currentSnapshotButton = new QPushButton("Snapshot", this);
	QTimer* timer = new QTimer(this);

	lowerThresholdSpinBox->setRange(0, std::numeric_limits<uint16_t>::max());
	upperThresholdSpinBox->setRange(0, std::numeric_limits<uint16_t>::max());
	lowerExposureSpinBox->setRange(0, std::numeric_limits<uint16_t>::max());
	upperExposureSpinBox->setRange(0, std::numeric_limits<uint16_t>::max());

	// connect stuff
	connect(referenceSnapshotButton, &QPushButton::pressed, this, &FrameCompareWidget::getReferenceSnapshot);
	connect(currentSnapshotButton, &QPushButton::pressed, this, &FrameCompareWidget::getCurrentSnapshot);
	connect(continuousModeCheckBox, &QCheckBox::stateChanged, this, [=]()
		{
			bool isChecked = continuousModeCheckBox->isChecked();
			currentSnapshotButton->setEnabled(!isChecked);
			isChecked ? timer->start(continuousModeInterval) : timer->stop();
		});
	connect(autoExposureCheckBox, &QCheckBox::stateChanged, this, [=]()
		{
			bool isChecked = autoExposureCheckBox->isChecked();
			lowerExposureSpinBox->setEnabled(!isChecked);
			upperExposureSpinBox->setEnabled(!isChecked);
		});
	connect(lowerThresholdSpinBox, &QSpinBox::valueChanged, this, &FrameCompareWidget::compare);
	connect(upperThresholdSpinBox, &QSpinBox::valueChanged, this, &FrameCompareWidget::compare);
	connect(lowerExposureSpinBox, &QSpinBox::valueChanged, this, &FrameCompareWidget::compare);
	connect(upperExposureSpinBox, &QSpinBox::valueChanged, this, &FrameCompareWidget::compare);
	connect(autoExposureCheckBox, &QCheckBox::stateChanged, this, &FrameCompareWidget::compare);
	connect(timer, &QTimer::timeout, this, &FrameCompareWidget::getCurrentSnapshot);

	// layout stuff
	vboxReference->addWidget(refImageLabel);
	vboxReference->addWidget(referenceSnapshotButton);

	vboxDifference->addWidget(difImageLabel);
	vboxDifference->addWidget(currentSnapshotButton);

	formSettings->addRow("Continuous", continuousModeCheckBox);
	formSettings->addRow("Lower Threshold (mm)", lowerThresholdSpinBox);
	formSettings->addRow("Upper Threshold (mm)", upperThresholdSpinBox);
	formSettings->addRow("Auto Exposure", autoExposureCheckBox);
	formSettings->addRow("Lower Exposure (mm)", lowerExposureSpinBox);
	formSettings->addRow("Upper Exposure (mm)", upperExposureSpinBox);

	formSettings->setFieldGrowthPolicy(QFormLayout::FieldGrowthPolicy::ExpandingFieldsGrow);

	QWidget* vboxLeftWidget = new QWidget(this);
	vboxLeftWidget->setLayout(vboxReference);

	QWidget* vboxRightWidget = new QWidget(this);
	vboxRightWidget->setLayout(vboxDifference);

	splitter->addWidget(vboxLeftWidget);
	splitter->addWidget(vboxRightWidget);

	hboxMain->addWidget(splitter);
	hboxMain->addLayout(formSettings);
}

FrameCompareWidget::~FrameCompareWidget()
{
}

void FrameCompareWidget::getReferenceSnapshot()
{
	refFs = threadInterface.peekGuiFrame();
	frameset::ImageOptions options(tinycolormap::Turbo, true, 0, 0, false, false);
	QImage image = frameset::toQImage(refFs.depth, options);
	refImageLabel->setPixmap(QPixmap::fromImage(image));
}

void FrameCompareWidget::getCurrentSnapshot()
{
	curFs = threadInterface.peekGuiFrame();
	if (!frameset::isEmpty(refFs) && !frameset::isEmpty(curFs))
		compare();
}

void FrameCompareWidget::compare()
{
	if (!frameset::isValid(refFs) || !frameset::isValid(curFs))
		return;

	if (frameset::size(refFs.depth) != frameset::size(curFs.depth))
		return;

	if (frameset::isEmpty(refFs))
		return;

	frameset::Frame difference = frameset::difference(curFs.depth, refFs.depth);
	frameset::clip(difference, lowerThresholdSpinBox->value(), upperThresholdSpinBox->value());
	frameset::ImageOptions options(
		colorMap, 
		autoExposureCheckBox->isChecked(), 
		lowerExposureSpinBox->value(),
		upperExposureSpinBox->value(), 
		false,
		false);
	QImage image = frameset::toQImage(difference, options);
	difImageLabel->setPixmap(QPixmap::fromImage(image));
}
