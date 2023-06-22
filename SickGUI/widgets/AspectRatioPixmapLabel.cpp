#include "AspectRatioPixmapLabel.h"
//#include <QDebug>

AspectRatioPixmapLabel::AspectRatioPixmapLabel(QWidget* parent) : QLabel(parent)
{
	this->setMinimumSize(1, 1);
	setScaledContents(false);
}

void AspectRatioPixmapLabel::setPixmap(const QPixmap& p)
{
	pix = p;
	QLabel::setPixmap(scaledPixmap());
	QLabel::setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	updateGeometry();
}

QPixmap AspectRatioPixmapLabel::scaledPixmap() const
{
	return pix.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void AspectRatioPixmapLabel::resizeEvent(QResizeEvent* e)
{
	//adjustSize();
	if (!pix.isNull())
		QLabel::setPixmap(scaledPixmap());
}