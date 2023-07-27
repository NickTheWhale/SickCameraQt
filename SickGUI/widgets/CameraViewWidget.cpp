#include "CameraViewWidget.h"
#include <qpushbutton.h>

CameraViewWidget::CameraViewWidget(QWidget* parent) :
	QWidget(parent),
	label(new CameraLabel(this)),
	grid(new QGridLayout(this))
{
	QPushButton* clearMaskBtn = new QPushButton("Clear Mask", this);
	connect(clearMaskBtn, &QPushButton::pressed, label, &CameraLabel::clearMask);
	grid->addWidget(clearMaskBtn, 0, 0);
	grid->addWidget(label, 0, 1, Qt::AlignLeft | Qt::AlignRight | Qt::AlignBottom | Qt::AlignTop);
	setLayout(grid);
}

CameraViewWidget::~CameraViewWidget()
{
}

void CameraViewWidget::setPixmap(const QPixmap& pixmap)
{
	label->setPixmap(pixmap);
}
