#include "CameraLabel.h"
#include <qpainter.h>
#include <qmenu.h>
#include <qfiledialog.h>

CameraLabel::CameraLabel(QWidget* parent) : QLabel(parent)
{
	this->setMargin(0);
	this->setMinimumSize(1, 1);
	setScaledContents(false);
	this->cursor().setShape(Qt::CrossCursor);
}

void CameraLabel::setPixmap(const QPixmap& p)
{
	pix = p;

	QPainter painter(&pix);
	if (!mouseDown)
		painter.drawRect(rubberBandDrawRect);

	QLabel::setPixmap(scaledPixmap());
	QLabel::setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
}

QPixmap CameraLabel::scaledPixmap() const
{
	return pix.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void CameraLabel::clearMask()
{
}

void CameraLabel::resizeEvent(QResizeEvent* e)
{
	if (!pix.isNull())
		QLabel::setPixmap(scaledPixmap());
}

void CameraLabel::contextMenuEvent(QContextMenuEvent* event)
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

void CameraLabel::mousePressEvent(QMouseEvent* event)
{
	QWidget::mousePressEvent(event);
	if (event->button() != Qt::LeftButton)
		return;

	mouseDown = true;
	const QSize scaledPixSize = scaledPixmap().size();
	if (scaledPixSize.isEmpty())
		return;

	const QSize thisSize = this->size();
	if (thisSize.isEmpty())
		return;

	const QPoint pos = event->pos();
	const QSize offsetSize = (thisSize - scaledPixSize) / 2;
	const QPoint offset = QPoint(offsetSize.width(), offsetSize.height());
	const QPoint scaledPixPos = pos - offset;
	const QPointF normPos = QPointF(scaledPixPos.x() / static_cast<qreal>(scaledPixSize.width()), scaledPixPos.y() / static_cast<qreal>(scaledPixSize.height()));
	maskNorm.setTopLeft(normPos);
	const qreal scaleFactor = pix.size().width() / static_cast<qreal>(scaledPixSize.width());
	const QPoint rawPixPos = scaledPixPos * scaleFactor;
	rubberBandDrawRect.setTopLeft(rawPixPos);
	rubberBandOrigin = pos;
	if (!rubberBand)
		rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
	rubberBand->setGeometry(QRect(rubberBandOrigin, QSize()));
	rubberBand->show();
}

void CameraLabel::mouseMoveEvent(QMouseEvent* event)
{
	QWidget::mouseMoveEvent(event);
	if (!rubberBand)
		return;
	rubberBand->setGeometry(QRect(rubberBandOrigin, event->pos()).normalized());
}

void CameraLabel::mouseReleaseEvent(QMouseEvent* event)
{
	QWidget::mouseReleaseEvent(event);
	
	if (event->button() != Qt::LeftButton)
		return;

	mouseDown = false;
	if (!rubberBand)
		return;

	rubberBand->hide();

	const QSize scaledPixSize = scaledPixmap().size();
	if (scaledPixSize.isEmpty())
		return;

	const QSize thisSize = this->size();
	if (thisSize.isEmpty())
		return;

	const QPoint pos = event->pos();
	const QSize offsetSize = (thisSize - scaledPixSize) / 2;
	const QPoint offset = QPoint(offsetSize.width(), offsetSize.height());

	const QPoint scaledPixPos = pos - offset;

	const QPointF normPos = QPointF(scaledPixPos.x() / static_cast<qreal>(scaledPixSize.width()), scaledPixPos.y() / static_cast<qreal>(scaledPixSize.height()));
	constexpr QRectF bounds(0.0, 0.0, 1.0, 1.0);

	if (bounds.contains(normPos))
	{
		maskNorm.setBottomRight(normPos);
		maskNorm = maskNorm.normalized();
		const qreal scaleFactor = pix.size().width() / static_cast<qreal>(scaledPixSize.width());
		const QPoint rawPixPos = scaledPixPos * scaleFactor;
		rubberBandDrawRect.setBottomRight(rawPixPos);
		rubberBandDrawRect = rubberBandDrawRect.normalized();
		rubberBandDrawRect.translate(-1, -1);
		qDebug() << rubberBandDrawRect;
		emit newMask(maskNorm);
	}
}

QSize CameraLabel::minimumSizeHint() const
{
	return QSize(1, 1);
}

QSize CameraLabel::sizeHint() const
{
	return pix.size();
}