#include "CameraLabel.h"
#include <qpainter.h>
#include <qmenu.h>
#include <qfiledialog.h>

CameraLabel::CameraLabel(QWidget* parent) : QLabel(parent)
{
	this->setMargin(0);
	this->setMinimumSize(1, 1);
	setScaledContents(false);
}

void CameraLabel::setPixmap(const QPixmap& p)
{
	pix = p;

	QPainter painter(&pix);
	if (!maskPolygon.empty())
		painter.drawPolygon(maskPolygon, Qt::WindingFill);

	QLabel::setPixmap(scaledPixmap());
	QLabel::setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
}

QPixmap CameraLabel::scaledPixmap() const
{
	return pix.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void CameraLabel::clearMask()
{
	maskPolygon.clear();
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
	if (pix.isNull())
		return;

	const QSize scaledPixSize = scaledPixmap().size();
	if (scaledPixSize.isEmpty())
		return;

	const QSize thisSize = this->size();
	if (thisSize.isEmpty())
		return;

	const QPoint rawPos = event->pos();
	const QSize offsetSize = (thisSize - scaledPixSize) / 2;
	const QPoint offset = QPoint(offsetSize.width(), offsetSize.height());

	const QPoint scaledPixPos = rawPos - offset;

	const QPointF normPos = QPointF(scaledPixPos.x() / static_cast<qreal>(scaledPixSize.width()), scaledPixPos.y() / static_cast<qreal>(scaledPixSize.height()));
	constexpr QRectF bounds(0.0, 0.0, 1.0, 1.0);

	if (bounds.contains(normPos))
	{
		const qreal scaleFactor = pix.size().width() / static_cast<qreal>(scaledPixSize.width());
		const QPoint rawPixPos = scaledPixPos * scaleFactor;
		maskPolygon.push_back(rawPixPos);
		emit newMask(normPos);
	}

	QWidget::mousePressEvent(event);
}

QSize CameraLabel::minimumSizeHint() const
{
	return QSize(1, 1);
}

QSize CameraLabel::sizeHint() const
{
	return pix.size() + pix.size();
}