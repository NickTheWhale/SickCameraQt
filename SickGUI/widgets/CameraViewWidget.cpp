#include "CameraViewWidget.h"
#include <qcheckbox.h>

CameraViewWidget::CameraViewWidget(QWidget* parent) :
	QWidget(parent),
	label(new ImageLabel(this)),
	grid(new QGridLayout(this))

{
	QCheckBox* enableMaskBtn = new QCheckBox("Enable Mask", this);
	enableMaskBtn->setCheckable(true);
	enableMaskBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(enableMaskBtn, &QCheckBox::stateChanged, this, [=]()
		{
			bool enable = enableMaskBtn->isChecked();
			emit setEnableMask(enable);
		});

	label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	connect(label, &ImageLabel::newMask, this, &CameraViewWidget::onNewMask);

	grid->addWidget(enableMaskBtn, 1, 0);
	grid->addWidget(label, 0, 0/*, Qt::AlignLeft | Qt::AlignRight | Qt::AlignBottom | Qt::AlignTop*/);
	setLayout(grid);
}

CameraViewWidget::~CameraViewWidget()
{
}

void CameraViewWidget::setPixmap(const QPixmap& pixmap)
{
	label->setPixmap(pixmap);
}

void CameraViewWidget::onNewMask(const QRectF& maskNorm)
{
	emit newMask(maskNorm);
}
