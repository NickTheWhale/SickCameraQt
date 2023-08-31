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
	// this can be nice for debugging layout issues since it makes a think border
	//this->setFrameStyle(2);
}

void ImageLabel::setPixmap(const QPixmap& p)
{
	// cache the latest pixmap and display the scalled version
	pix = p;
	QLabel::setPixmap(scaledPixmap());
	QLabel::setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
}

QPixmap ImageLabel::scaledPixmap() const
{
	// scale cached pixmap to the size of the widget
	return !pix.isNull() ? pix.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation) : QPixmap();
}

void ImageLabel::resizeEvent(QResizeEvent* e)
{
	// resize the pixmap
	if (!pix.isNull())
		QLabel::setPixmap(scaledPixmap());
}

void ImageLabel::contextMenuEvent(QContextMenuEvent* event)
{
	if (pix.isNull())
		return;

	QMenu menu(this);
	// add option to save the displayed pixmap to file
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
	// needed for proper layouting
	return QSize(1, 1);
}

QSize ImageLabel::sizeHint() const
{
	// give qt an idea of what size to make this
	return pix.isNull() ? QWidget::sizeHint() : pix.size();
}