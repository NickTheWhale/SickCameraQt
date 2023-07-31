#include "ImageLabel.h"
#include <qpainter.h>
#include <qmenu.h>
#include <qfiledialog.h>

ImageLabel::ImageLabel(QWidget* parent) : QLabel(parent)
{
	this->setMargin(0);
	this->setMinimumSize(1, 1);
	setScaledContents(false);
	this->cursor().setShape(Qt::CrossCursor);
}

void ImageLabel::setPixmap(const QPixmap& p)
{
	pix = p;
	QLabel::setPixmap(scaledPixmap());
	QLabel::setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
}

QPixmap ImageLabel::scaledPixmap() const
{
	return !pix.isNull() ? pix.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation) : QPixmap();
}

void ImageLabel::clearMask()
{
}

void ImageLabel::resizeEvent(QResizeEvent* e)
{
	if (!pix.isNull())
		QLabel::setPixmap(scaledPixmap());
}

void ImageLabel::contextMenuEvent(QContextMenuEvent* event)
{
	if (pix.isNull())
		return;

	QMenu menu(this);
	QAction* saveAction = menu.addAction("Save as Image");
	QAction* selectedAction = menu.exec(event->globalPos());
	if (selectedAction == saveAction)
	{
		QString filePath = QFileDialog::getSaveFileName(this, "Save Image", "", "PNG Image (*.png)");

		if (!filePath.isEmpty())
		{
			QPixmap pixmap(this->size());
			this->render(&pixmap);
			pixmap.save(filePath);
		}
	}
}

void ImageLabel::mousePressEvent(QMouseEvent* event)
{
	QWidget::mousePressEvent(event);
	if (event->button() != Qt::LeftButton)
		return;

	rubberBandOrigin = event->pos();
	if (!rubberBand)
		rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
	rubberBand->setGeometry(QRect(rubberBandOrigin, QSize()));
	rubberBand->show();
}

void ImageLabel::mouseMoveEvent(QMouseEvent* event)
{
	QWidget::mouseMoveEvent(event);
	if (!rubberBand)
		return;
	rubberBand->setGeometry(QRect(rubberBandOrigin, event->pos()).normalized());
}

void ImageLabel::mouseReleaseEvent(QMouseEvent* event)
{
	QWidget::mouseReleaseEvent(event);

	if (event->button() != Qt::LeftButton)
		return;

	if (!rubberBand)
		return;

	rubberBand->hide();

	const QSize scaledPixSize = scaledPixmap().size();
	if (scaledPixSize.isEmpty())
		return;

	if (this->size().isEmpty())
		return;

	const QRect rubberBandBox = QRect(rubberBandOrigin, event->pos()).normalized();

	const QPoint pixOrigin = QPoint(((this->size() - scaledPixSize) / 2).width(), ((this->size() - scaledPixSize) / 2).height());
	const QRect pixBox = QRect(pixOrigin, scaledPixSize);

	const QRect intersection = rubberBandBox.intersected(pixBox).normalized().translated(-pixOrigin);

	const QPointF maskOrigin = QPointF(intersection.topLeft().x() / static_cast<qreal>(pixBox.width()), intersection.topLeft().y() / static_cast<qreal>(pixBox.height()));
	const QSizeF maskSize = QSizeF(intersection.size().width() / static_cast<qreal>(pixBox.size().width()), intersection.size().height() / static_cast<qreal>(pixBox.size().height()));
	const QRectF mask = QRectF(maskOrigin, maskSize).normalized();
	if (!mask.isEmpty())
	{
		emit newMask(mask);
	}
}

QSize ImageLabel::minimumSizeHint() const
{
	return QSize(1, 1);
}

QSize ImageLabel::sizeHint() const
{
	return pix.size();
}