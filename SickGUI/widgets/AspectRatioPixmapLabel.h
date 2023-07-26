/*****************************************************************//**
 * @file   AspectRatioPixmapLabel.h
 * @brief  QLabel subclass used to auto-scale a pixmap.
 * 
 * @author Nicholas Loehrke
 * @date   June 2023
 *********************************************************************/

#pragma once

#include <qlabel>
#include <qpixmap>
#include <qresizeevent>

/**
 * @brief The AspectRatioPixmapLabel class is a QLabel subclass that displays a resizable pixmap while maintaining aspect ratio.
 *
 * The AspectRatioPixmapLabel class provides a QLabel with additional functionality to display a pixmap while preserving its aspect ratio.
 * It automatically scales the pixmap to fit within the label's size, maintaining the original aspect ratio of the pixmap.
 * It also overrides certain event handling methods to customize the behavior of the label.
 */
class AspectRatioPixmapLabel : public QLabel
{
	Q_OBJECT

public:
	/**
	 * @brief Constructs an AspectRatioPixmapLabel object with the specified parent widget.
	 * @param parent The parent widget of the label (optional).
	 */
	explicit AspectRatioPixmapLabel(QWidget* parent = nullptr);

	/**
	 * @brief Returns the scaled pixmap currently displayed by the label.
	 * @return The scaled pixmap.
	 */
	QPixmap scaledPixmap() const;

public slots:
	/**
	 * @brief Sets the pixmap to be displayed by the label.
	 * @param pixmap The pixmap to be displayed.
	 */
	void setPixmap(const QPixmap& p);

protected:
	/**
	 * @brief Overridden method that is called when the label is resized.
	 * @param event The resize event object.
	 */
	void resizeEvent(QResizeEvent*) override;

	/**
	 * @brief Overridden method that is called when a context menu event occurs.
	 * @param event The context menu event object.
	 */
	void contextMenuEvent(QContextMenuEvent* event) override;

private:
	QPixmap pix; /**< The original pixmap. */
};
