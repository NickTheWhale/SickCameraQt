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
	// this can be nice for debugging layout issues
	//this->setFrameStyle(2);
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

QSize ImageLabel::minimumSizeHint() const
{
	return QSize(1, 1);
}

QSize ImageLabel::sizeHint() const
{
	return pix.isNull() ? QWidget::sizeHint() : pix.size();
}