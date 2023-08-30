/*****************************************************************//**
 * @file   CameraViewLabel.h
 * @brief  QLabel subclass used to auto-scale a pixmap.
 * 
 * @author Nicholas Loehrke
 * @date   June 2023
 *********************************************************************/

#pragma once

#include <vector>

#include <qlabel>
#include <qpixmap>
#include <qresizeevent>

/**
 * @brief The CameraViewLabel class is a QLabel subclass that displays a resizable pixmap while maintaining aspect ratio.
 *
 * The CameraViewLabel class provides a QLabel with additional functionality to display a pixmap while preserving its aspect ratio.
 * It automatically scales the pixmap to fit within the label's size, maintaining the original aspect ratio of the pixmap.
 * It also overrides certain event handling methods to customize the behavior of the label.
 */
class ImageLabel : public QLabel
{
	Q_OBJECT

public:
	/**
	 * @brief Constructs an CameraViewLabel object with the specified parent widget.
	 * @param parent The parent widget of the label (optional).
	 */
	explicit ImageLabel(QWidget* parent = nullptr);

	/**
	 * @brief Returns the scaled pixmap currently displayed by the label.
	 * @return The scaled pixmap.
	 */
	QPixmap scaledPixmap() const;

	QSize minimumSizeHint() const override;
	QSize sizeHint() const override;

public slots:
	/**
	 * @brief Sets the pixmap to be displayed by the label.
	 * @param pixmap The pixmap to be displayed.
	 */
	void setPixmap(const QPixmap& p);

protected:
	void resizeEvent(QResizeEvent*) override;
	void contextMenuEvent(QContextMenuEvent* event) override;

private:
	QPixmap pix;
};
