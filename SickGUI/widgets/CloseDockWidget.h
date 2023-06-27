#pragma once

#include <qdockwidget.h>
#include <qstring.h>
#include <qwidget.h>

/**
 * @brief The CloseDockWidget class represents a dockable widget with a public closed() signal.
 *
 * The CloseDockWidget class is a subclass of QDockWidget that adds a public close() signal useful for state synchronization.
 * It emits a signal when the close button is clicked or when the widget is closed through other means.
 */
class CloseDockWidget : public QDockWidget
{
	Q_OBJECT

public:
	/**
	 * @brief Constructs a CloseDockWidget with the specified title and parent widget.
	 * @param title The title of the dock widget.
	 * @param parent The parent widget of the dock widget (optional).
	 */
	explicit CloseDockWidget(const QString& title, QWidget* parent = nullptr);

	/**
	 * @brief Destructor for the CloseDockWidget class.
	 */
	~CloseDockWidget();

protected:
	/**
	 * @brief Overridden method that is called when the widget is closed.
	 * @param event The close event object.
	 */
	void closeEvent(QCloseEvent* event) override;

signals:
	/**
	 * @brief Signal emitted when the close button is clicked or when the widget is closed.
	 */
	void closed();
};
