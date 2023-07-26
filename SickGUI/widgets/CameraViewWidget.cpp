#include "CameraViewWidget.h"
#include <RangeSlider.h>
#include <qpushbutton.h>

CameraViewWidget::CameraViewWidget(QWidget* parent) :
	QWidget(parent),
	layout(new QGridLayout(this)),
	slider(new RangeSlider(Qt::Orientation::Vertical, RangeSlider::Option::DoubleHandles, this)),
	label(new AspectRatioPixmapLabel(this))
{
	label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QPushButton* depthBtn = new QPushButton("Depth Filter", this);
	depthBtn->setCheckable(true);
	depthBtn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	
	QPushButton* maskBtn = new QPushButton("Mask", this);
	maskBtn->setCheckable(true);
	maskBtn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

	connect(depthBtn, &QPushButton::pressed, this, [=]()
		{
			qDebug() << "depthBtn pressed";
			emit setDepthFilterEnable(depthBtn->isChecked());
		});
	connect(maskBtn, &QPushButton::pressed, this, [=]()
		{
			qDebug() << "maskBtn pressed";
			emit setDepthMaskEnable(maskBtn->isChecked());
		});
	connect(slider, &RangeSlider::lowerValueChanged, this, [=]()
		{
			qDebug() << "slider changed";
			emit setDepthFilterRange(slider->GetLowerValue(), slider->GetUpperValue());
		});
	connect(slider, &RangeSlider::upperValueChanged, this, [=]()
		{
			qDebug() << "slider changed";
			emit setDepthFilterRange(slider->GetLowerValue(), slider->GetUpperValue());
		});


	slider->setMaximumWidth(20);
	slider->SetRange(0, std::numeric_limits<uint16_t>::max() / 4);
	slider->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	// row, col, rowspan, colspan
	layout->addWidget(slider, 0, 0, 2, 1);
	layout->addWidget(label, 0, 1, 1, 2);
	layout->addWidget(depthBtn, 1, 1, 1, 1);
	layout->addWidget(maskBtn, 1, 2, 1, 1);
	setLayout(layout);
}

CameraViewWidget::~CameraViewWidget()
{
}

void CameraViewWidget::setPixmap(const QPixmap& pixmap)
{
	label->setPixmap(pixmap);
}
