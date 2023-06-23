#include "AspectRatioPixmapLabel.h"
#include <qpainter.h>
#include <qmenu.h>
#include <qfiledialog.h>

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
}

QPixmap AspectRatioPixmapLabel::scaledPixmap() const
{
	return pix.scaled(this->size(), Qt::KeepAspectRatio, Qt::FastTransformation);
}

void AspectRatioPixmapLabel::resizeEvent(QResizeEvent* e)
{
	if (!pix.isNull())
		QLabel::setPixmap(scaledPixmap());
}

void AspectRatioPixmapLabel::contextMenuEvent(QContextMenuEvent* event)
{
	if (pix.isNull())
		return;

	QPainter painter(this);
	painter.end();

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
