#include "FrameCompareWidget.h"
#include <qlayout.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qtimer.h>
#include <qsplitter.h>

FrameCompareWidget::FrameCompareWidget(QWidget* parent) :
	QWidget(parent),
	refImageLabel(new ImageLabel(this)),
	difImageLabel(new ImageLabel(this)),
	thresholdSpinBox(new QSpinBox(this)),
	threadInterface(ThreadInterface::instance())
{
	// make stuff
	QHBoxLayout* hboxMain = new QHBoxLayout(this);
	QSplitter* splitter = new QSplitter(this);
	splitter->setOrientation(Qt::Orientation::Horizontal);
	QVBoxLayout* vboxLeft = new QVBoxLayout();
	QVBoxLayout* vboxRight = new QVBoxLayout();
	QHBoxLayout* hboxRight = new QHBoxLayout();

	QCheckBox* gridCompareButton = new QCheckBox("Grid Compare", this);
	QPushButton* referenceSnapshotButton = new QPushButton("Snapshot", this);
	QCheckBox* continuousModeToggle = new QCheckBox("Continuous", this);
	thresholdSpinBox->setRange(0, 1000);
	QLabel* thresholdLabel = new QLabel("Threshold (mm)", this);
	QPushButton* currentSnapshotButton = new QPushButton("Snapshot", this);
	QTimer* timer = new QTimer(this);

	// connect stuff
	connect(referenceSnapshotButton, &QPushButton::pressed, this, &FrameCompareWidget::getReferenceSnapshot);
	connect(currentSnapshotButton, &QPushButton::pressed, this, &FrameCompareWidget::getCurrentSnapshot);
	connect(continuousModeToggle, &QCheckBox::stateChanged, this, [=]()
		{
			bool isChecked = continuousModeToggle->isChecked();
			currentSnapshotButton->setEnabled(!isChecked);
			isChecked ? timer->start(continuousModeInterval) : timer->stop();
		});
	connect(timer, &QTimer::timeout, this, &FrameCompareWidget::getCurrentSnapshot);
	connect(thresholdSpinBox, &QSpinBox::valueChanged, this, &FrameCompareWidget::compare);
	connect(gridCompareButton, &QCheckBox::stateChanged, this, [=]() {gridCompare = gridCompareButton->isChecked(); });

	// layout stuff
	vboxLeft->addWidget(refImageLabel);
	vboxLeft->addWidget(referenceSnapshotButton);

	hboxRight->addWidget(gridCompareButton);
	hboxRight->addWidget(continuousModeToggle);
	hboxRight->addWidget(currentSnapshotButton);
	hboxRight->addWidget(thresholdLabel);
	hboxRight->addWidget(thresholdSpinBox);

	vboxRight->addWidget(difImageLabel);
	vboxRight->addLayout(hboxRight);

	QWidget* vboxLeftWidget = new QWidget(this);
	vboxLeftWidget->setLayout(vboxLeft);

	QWidget* vboxRightWidget = new QWidget(this);
	vboxRightWidget->setLayout(vboxRight);

	splitter->addWidget(vboxLeftWidget);
	splitter->addWidget(vboxRightWidget);

	hboxMain->addWidget(splitter);
}

FrameCompareWidget::~FrameCompareWidget()
{
}

void FrameCompareWidget::getReferenceSnapshot()
{
	refFs = threadInterface.peekGuiFrame();
	QImage image;
	Frameset::depthToQImage(refFs, image, colorMap);
	refImageLabel->setPixmap(QPixmap::fromImage(image));
}

void FrameCompareWidget::getCurrentSnapshot()
{
	curFs = threadInterface.peekGuiFrame();
	if (!refFs.isNull() && !curFs.isNull())
		compare();
}

void FrameCompareWidget::compare()
{
	if (refFs.depth.size() != curFs.depth.size())
		return;

	if (refFs.depth.size() <= 0)
		return;
	if (gridCompare)
	{

	}
	else
	{
		Frameset::frameset_t difFs = refFs;
		difFs.depth.clear();

		const size_t size = refFs.depth.size();
		for (size_t i = 0; i < size; ++i)
		{
			const uint16_t delta = std::abs(refFs.depth[i] - curFs.depth[i]);
			if (delta > thresholdSpinBox->value() && delta < upperThreshold)
			{
				difFs.depth.push_back(delta);
			}
			else
			{
				difFs.depth.push_back(0);
			}
		}
		QImage image;
		Frameset::depthToQImage(difFs, image, colorMap);
		difImageLabel->setPixmap(QPixmap::fromImage(image));
	}
}
