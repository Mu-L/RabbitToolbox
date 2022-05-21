#include "draggable_list_widget.h"
#include <qapplication.h>
#include <qdrag.h>
#include <qmimedata.h>
#include <qlabel.h>
#include <qlayout.h>

namespace flower
{
	DraggableListWindow::DraggableListWindow() noexcept(false)
	{
		this->setResizeMode(QListView::Adjust);
		this->setViewMode(QListView::IconMode);
		this->setMovement(QListView::Static);
		this->setDefaultDropAction(Qt::DropAction::MoveAction);
		this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	}

	DraggableListWindow::~DraggableListWindow() noexcept
	{
	}

	void
	DraggableListWindow::mouseMoveEvent(QMouseEvent *event)
	{
		if (event->buttons() & Qt::LeftButton)
		{
			QPoint length = event->pos() - startPos;
			if (length.manhattanLength() > QApplication::startDragDistance())
			{
				QListWidgetItem* item = this->itemAt(this->startPos);
				if (item)
				{
					auto widget = this->itemWidget(item);
					auto layout = widget->layout();
					auto label = dynamic_cast<QLabel*>(layout->itemAt(0)->widget());
					if (label)
					{
						auto mimeData = new QMimeData;
						mimeData->setData("object/mimeData", item->data(Qt::UserRole).toByteArray());

						auto drag = new QDrag(this);
						drag->setMimeData(mimeData);
						drag->setPixmap(label->pixmap(Qt::ReturnByValue));
						drag->setHotSpot(QPoint(drag->pixmap().width() / 2, drag->pixmap().height() / 2));
						drag->exec(Qt::MoveAction);

						emit itemSelected(item);
					}
				}
			}
		}

		QListWidget::mouseMoveEvent(event);
	}

	void
	DraggableListWindow::mousePressEvent(QMouseEvent *event)
	{
		if (event->button() == Qt::LeftButton)
			startPos = event->pos();

		QListWidget::mousePressEvent(event);
	}
}