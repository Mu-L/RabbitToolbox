#ifndef FLOWER_DRAGGABLE_LIST_WIDGET_H_
#define FLOWER_DRAGGABLE_LIST_WIDGET_H_

#include <qevent.h>
#include <qlistwidget.h>

namespace flower
{
	class DraggableListWindow final : public QListWidget
	{
		Q_OBJECT
	public:
		DraggableListWindow() noexcept(false);
		~DraggableListWindow() noexcept;

		void mouseMoveEvent(QMouseEvent *event) override;
		void mousePressEvent(QMouseEvent *event) override;

	Q_SIGNALS:
		void itemSelected(QListWidgetItem*);

	private:
		QPoint startPos;
	};
}

#endif