#include "CameraViewWidget.h"
#include <qpushbutton.h>

CameraViewWidget::CameraViewWidget(QWidget* parent) :
	QWidget(parent),
	label(new CameraLabel(this)),
	grid(new QGridLayout(this))

{
	QPushButton* enableMaskBtn = new QPushButton("Enable Mask", this);
	enableMaskBtn->setCheckable(true);
	enableMaskBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(enableMaskBtn, &QPushButton::toggled, this, [=]()
		{
			bool enable = enableMaskBtn->isChecked();
			qDebug() << __FUNCTION__ << "setting mask:" << enable;
			emit setEnableMask(enable);
		});

	label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	connect(label, &CameraLabel::newMask, this, &CameraViewWidget::onNewMask);

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
