#include "FrameCompareWidget.h"
#include <qlayout.h>
#include <qpushbutton.h>
#include <ThreadInterface.h>

FrameCompareWidget::FrameCompareWidget(QWidget* parent) :
	QWidget(parent),
	imageLabel1(new ImageLabel(this)),
	imageLabel2(new ImageLabel(this)),
	imageLabel3(new ImageLabel(this))
{
	QPushButton* snapButton1 = new QPushButton("Snapshot", this);
	connect(snapButton1, &QPushButton::pressed, this, &FrameCompareWidget::snapshot1);
	QVBoxLayout* vbox1 = new QVBoxLayout();
	vbox1->addWidget(imageLabel1);
	vbox1->addWidget(snapButton1);

	QPushButton* snapButton2 = new QPushButton("Snapshot", this);
	connect(snapButton2, &QPushButton::pressed, this, &FrameCompareWidget::snapshot2);
	QVBoxLayout* vbox2 = new QVBoxLayout();
	vbox2->addWidget(imageLabel2);
	vbox2->addWidget(snapButton2);

	QPushButton* compareButton = new QPushButton("Compare", this);
	connect(compareButton, &QPushButton::pressed, this, &FrameCompareWidget::compare);
	QPushButton* resetButton = new QPushButton("Reset", this);
	thresholdSpinBox = new QSpinBox(this);
	thresholdSpinBox->setRange(0, 5000);
	QHBoxLayout* hbox3 = new QHBoxLayout();
	hbox3->addWidget(compareButton);
	hbox3->addWidget(thresholdSpinBox);
	hbox3->addWidget(resetButton);
	QVBoxLayout* vbox3 = new QVBoxLayout();
	vbox3->addWidget(imageLabel3);
	vbox3->addLayout(hbox3);

	QGridLayout* grid = new QGridLayout(this);
	grid->addLayout(vbox1, 0, 0);
	grid->addLayout(vbox2, 1, 0);
	grid->addLayout(vbox3, 0, 1, 2, 1);
}

FrameCompareWidget::~FrameCompareWidget()
{
}

void FrameCompareWidget::snapshot1()
{
	ThreadInterface& threadInterface = ThreadInterface::instance();
	fs1 = threadInterface.peekGuiFrame();

	QImage image;
	Frameset::depthToQImage(fs1, image, colorMap);
	imageLabel1->setPixmap(QPixmap::fromImage(image));
}

void FrameCompareWidget::snapshot2()
{
	ThreadInterface& threadInterface = ThreadInterface::instance();
	fs2 = threadInterface.peekGuiFrame();

	QImage image;
	Frameset::depthToQImage(fs2, image, colorMap);
	imageLabel2->setPixmap(QPixmap::fromImage(image));
}

void FrameCompareWidget::compare()
{
	Frameset::frameset_t fs3;
	fs3.height = fs1.height;
	fs3.width = fs1.width;

	if (fs1.depth.size() == fs2.depth.size())
	{
		const auto length = fs1.depth.size();
		for (int i = 0; i < length; ++i)
		{
			const auto delta = std::abs(fs1.depth[i] - fs2.depth[i]);
			if (delta > thresholdSpinBox->value())
				fs3.depth.push_back(std::abs(fs1.depth[i] - fs2.depth[i]));
			else
				fs3.depth.push_back(0);
		}
	}

	QImage image3;
	Frameset::depthToQImage(fs3, image3, colorMap);
	imageLabel3->setPixmap(QPixmap::fromImage(image3));
}
