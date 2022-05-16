#ifndef FLOWER_RECORD_DOCK_H_
#define FLOWER_RECORD_DOCK_H_

#include <qdockwidget.h>
#include <qdialog.h>
#include <qboxlayout.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <optional>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qtimer.h>
#include "flower_behaviour.h"
#include "spoiler.h"
#include <octoon/game_object.h>

namespace flower
{
	class RecordDock final : public QDockWidget
	{
		Q_OBJECT
	public:
		RecordDock(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<FlowerProfile>& profile) noexcept;
		~RecordDock() noexcept;

		void update();
		void repaint();

		void startRecord(QString fileName);
		void stopRecord();

		void showEvent(QShowEvent* event) override;
		void paintEvent(QPaintEvent* e) noexcept override;
		void resizeEvent(QResizeEvent* e) noexcept override;

	private Q_SLOTS:
		void clickEvent();
		void select1Event(bool checked);
		void select2Event(bool checked);
		void speed1Event(bool checked);
		void speed2Event(bool checked);
		void speed3Event(bool checked);
		void speed4Event(bool checked);
		void startEvent(int);
		void endEvent(int);
		void timeEvent();
		void onSppChanged(int);
		void onBouncesChanged(int);
		void onCrfChanged(double);

	public:
		QLabel* quality_;
		QLabel* videoRatio_;
		QLabel* frame_;
		QLabel* animation_;
		QLabel* summary_;
		QLabel* currentFrame_;
		QLabel* timeTotal_;
		QLabel* sppLabel;
		QLabel* bouncesLabel_;
		QLabel* crfLabel;
		QLabel* startLabel_;
		QLabel* endLabel_;

		QButtonGroup* group_;
		QButtonGroup* speedGroup_;

		QToolButton* select1_;
		QToolButton* select2_;
		QToolButton* speed1_;
		QToolButton* speed2_;
		QToolButton* speed3_;
		QToolButton* speed4_;
		QToolButton* recordButton_;
		QToolButton* markButton_;

		QSpinBox* start_;
		QSpinBox* end_;
		QSpinBox* sppSpinbox_;
		QSpinBox* bouncesSpinbox_;
		
		QDoubleSpinBox* crfSpinbox;

		QHBoxLayout* videoRatioLayout_;
		QHBoxLayout* frameLayout_;
		QVBoxLayout* mainLayout_;
		QTimer* timer_;

		QWidget* mainWidget_;

		Spoiler* markSpoiler_;
		Spoiler* videoSpoiler_;
		Spoiler* infoSpoiler_;
		QScrollArea* contentWidgetArea_;

		octoon::GameObjectPtr behaviour_;
	};
}

#endif