#include "record_dock.h"
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qevent.h>
#include <qdrag.h>
#include <qmimedata.h>
#include <qapplication.h>

namespace flower
{
	class SpinBox final : public QSpinBox
	{
	public:
		void focusInEvent(QFocusEvent* event) override
		{
			this->grabKeyboard();
			QSpinBox::focusInEvent(event);
		}

		void focusOutEvent(QFocusEvent* event) override
		{
			this->releaseKeyboard();
			QSpinBox::focusOutEvent(event);
		}
	};

	class DoubleSpinBox final : public QDoubleSpinBox
	{
	public:
		void focusInEvent(QFocusEvent* event) override
		{
			this->grabKeyboard();
			QDoubleSpinBox::focusInEvent(event);
		}

		void focusOutEvent(QFocusEvent* event) override
		{
			this->releaseKeyboard();
			QDoubleSpinBox::focusOutEvent(event);
		}
	};

	FocalTargetWindow::FocalTargetWindow() noexcept
	{
	}

	FocalTargetWindow::~FocalTargetWindow() noexcept
	{
	}

	void
	FocalTargetWindow::mouseMoveEvent(QMouseEvent *event)
	{
		if (event->buttons() & Qt::LeftButton)
		{
			QPoint length = event->pos() - startPos;
			if (length.manhattanLength() > QApplication::startDragDistance())
			{
				auto mimeData = new QMimeData;
				mimeData->setData("object/dof", "Automatic");

				auto drag = new QDrag(this);
				drag->setMimeData(mimeData);
				drag->setPixmap(QPixmap(":res/icons/target.png"));
				drag->setHotSpot(QPoint(drag->pixmap().width() / 2, drag->pixmap().height() / 2));
				drag->exec(Qt::MoveAction);
			}
		}

		QToolButton::mouseMoveEvent(event);
	}

	void
	FocalTargetWindow::mousePressEvent(QMouseEvent *event)
	{
		if (event->button() == Qt::LeftButton)
			startPos = event->pos();

		QToolButton::mousePressEvent(event);
	}

	RecordDock::RecordDock(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<FlowerProfile>& profile) noexcept
		: behaviour_(behaviour)
		, timer_(new QTimer(this))
	{
		this->setObjectName("RecordDock");
		this->setWindowTitle(u8"¼��");

		markButton_ = new QToolButton();
		markButton_->setObjectName("mark");
		markButton_->setIcon(QIcon::fromTheme(":res/icons/append2.png"));
		markButton_->setIconSize(QSize(139, 143));

		quality_ = new QLabel();
		quality_->setText(u8"��Ⱦ����");

		select1_ = new QToolButton();
		select1_->setObjectName("select1");
		select1_->setText(u8"������Ⱦ");
		select1_->setCheckable(true);
		select1_->click();

		select2_ = new QToolButton();
		select2_->setObjectName("select2");
		select2_->setText(u8"������Ⱦ");
		select2_->setCheckable(true);

		group_ = new QButtonGroup();
		group_->addButton(select1_, 0);
		group_->addButton(select2_, 1);

		videoRatio_ = new QLabel();
		videoRatio_->setText(u8"֡����");

		speed1_ = new QToolButton();
		speed1_->setObjectName("speed1");
		speed1_->setText(u8"24");
		speed1_->setCheckable(true);
		speed1_->click();

		speed2_ = new QToolButton();
		speed2_->setObjectName("speed2");
		speed2_->setText(u8"25");
		speed2_->setCheckable(true);

		speed3_ = new QToolButton();
		speed3_->setObjectName("speed3");
		speed3_->setText(u8"30");
		speed3_->setCheckable(true);

		speed4_ = new QToolButton();
		speed4_->setObjectName("speed4");
		speed4_->setText(u8"60");
		speed4_->setCheckable(true);

		speedGroup_ = new QButtonGroup();
		speedGroup_->addButton(speed1_, 0);
		speedGroup_->addButton(speed2_, 1);
		speedGroup_->addButton(speed3_, 2);
		speedGroup_->addButton(speed4_, 3);

		frame_ = new QLabel();
		frame_->setText(u8"����:");

		startLabel_ = new QLabel();
		startLabel_->setText(u8"��ʼ");

		endLabel_ = new QLabel();
		endLabel_->setText(u8"- ����");

		start_ = new SpinBox();
		start_->setObjectName("start");
		start_->setAlignment(Qt::AlignRight);
		start_->setMinimum(0);
		start_->setMaximum(99999);

		end_ = new SpinBox();
		end_->setObjectName("end");
		end_->setAlignment(Qt::AlignRight);
		end_->setMinimum(0);
		end_->setMaximum(99999);

		bouncesLabel_ = new QLabel();
		bouncesLabel_->setText(u8"ÿ���ع��ߵݹ����:");
		bouncesLabel_->setStyleSheet("color: rgb(200,200,200);");

		bouncesSpinbox_ = new SpinBox();
		bouncesSpinbox_->setMinimum(1);
		bouncesSpinbox_->setMaximum(32);
		bouncesSpinbox_->setValue(0);
		bouncesSpinbox_->setAlignment(Qt::AlignRight);
		bouncesSpinbox_->setFixedWidth(100);

		sppLabel = new QLabel();
		sppLabel->setText(u8"ÿ���ز�����:");
		sppLabel->setStyleSheet("color: rgb(200,200,200);");

		sppSpinbox_ = new SpinBox();
		sppSpinbox_->setMinimum(0);
		sppSpinbox_->setMaximum(9999);
		sppSpinbox_->setValue(0);
		sppSpinbox_->setAlignment(Qt::AlignRight);
		sppSpinbox_->setFixedWidth(100);

		crfSpinbox = new DoubleSpinBox();
		crfSpinbox->setMinimum(0);
		crfSpinbox->setMaximum(63.0);
		crfSpinbox->setValue(0);
		crfSpinbox->setAlignment(Qt::AlignRight);
		crfSpinbox->setFixedWidth(100);

		crfLabel = new QLabel();
		crfLabel->setText(u8"ѹ������(CRF):");
		crfLabel->setStyleSheet("color: rgb(200,200,200);");

		frameLayout_ = new QHBoxLayout();
		frameLayout_->addSpacing(20);
		frameLayout_->addWidget(startLabel_, 0, Qt::AlignLeft);
		frameLayout_->addWidget(start_, 0, Qt::AlignLeft);
		frameLayout_->addWidget(endLabel_, 0, Qt::AlignLeft);
		frameLayout_->addWidget(end_, 0, Qt::AlignLeft);
		frameLayout_->addStretch();

		animation_ = new QLabel();
		animation_->setContentsMargins(20, 0, 0, 0);

		summary_ = new QLabel();
		summary_->setContentsMargins(20, 0, 0, 0);

		currentFrame_ = new QLabel();
		currentFrame_->setContentsMargins(20, 0, 0, 0);

		timeTotal_ = new QLabel();
		timeTotal_->setContentsMargins(20, 0, 0, 0);

		recordButton_ = new QToolButton();
		recordButton_->setObjectName("render");
		recordButton_->setText(u8"��ʼ��Ⱦ");
		recordButton_->setContentsMargins(0, 0, 0, 0);

		dofInfoLabel_ = new QLabel();
		dofInfoLabel_->setText(u8"* ���²��������迪����Ⱦ");
		dofInfoLabel_->setStyleSheet("color: rgb(100,100,100);");

		apertureLabel_ = new QLabel();
		apertureLabel_->setText(u8"��Ȧ:");
		apertureLabel_->setStyleSheet("color: rgb(200,200,200);");

		apertureSpinbox_ = new DoubleSpinBox();
		apertureSpinbox_->setMinimum(0);
		apertureSpinbox_->setMaximum(64.0);
		apertureSpinbox_->setValue(0);
		apertureSpinbox_->setSingleStep(0.1f);
		apertureSpinbox_->setAlignment(Qt::AlignRight);
		apertureSpinbox_->setFixedWidth(100);
		apertureSpinbox_->setPrefix(u8"f/");
		apertureSpinbox_->setDecimals(1);

		focalDistanceLabel_ = new QLabel();
		focalDistanceLabel_->setText(u8"����:");
		focalDistanceLabel_->setStyleSheet("color: rgb(200,200,200);");

		focalDistanceName_ = new QLabel();
		focalDistanceName_->setText(u8"Ŀ�꣺��");
		focalDistanceName_->setStyleSheet("color: rgb(200,200,200);");

		focalTargetButton_ = new FocalTargetWindow();
		focalTargetButton_->setIcon(QIcon(":res/icons/target.png"));
		focalTargetButton_->setIconSize(QSize(48, 48));
		focalTargetButton_->setToolTip(u8"ͨ����ק��ͼ�굽Ŀ��ģ�Ϳɰ�ģ�Ͳ������Զ����");

		focalDistanceSpinbox_ = new DoubleSpinBox();
		focalDistanceSpinbox_->setMinimum(0);
		focalDistanceSpinbox_->setMaximum(std::numeric_limits<float>::infinity());
		focalDistanceSpinbox_->setValue(0);
		focalDistanceSpinbox_->setSingleStep(0.1f);
		focalDistanceSpinbox_->setAlignment(Qt::AlignRight);
		focalDistanceSpinbox_->setFixedWidth(100);
		focalDistanceSpinbox_->setSuffix(u8"m");

		videoRatioLayout_ = new QHBoxLayout();
		videoRatioLayout_->addStretch();
		videoRatioLayout_->addWidget(speed1_, 0, Qt::AlignRight);
		videoRatioLayout_->addWidget(speed2_, 0, Qt::AlignVCenter);
		videoRatioLayout_->addWidget(speed3_, 0, Qt::AlignVCenter);
		videoRatioLayout_->addWidget(speed4_, 0, Qt::AlignLeft);
		videoRatioLayout_->addStretch();
		videoRatioLayout_->setContentsMargins(0, 0, 0, 0);

		auto selectLayout_ = new QHBoxLayout();
		selectLayout_->addWidget(select1_, 0, Qt::AlignLeft);
		selectLayout_->addWidget(select2_, 0, Qt::AlignLeft);
		selectLayout_->setContentsMargins(0, 0, 0, 0);

		auto videoLayout = new QVBoxLayout;
		videoLayout->addWidget(quality_);
		videoLayout->addLayout(selectLayout_);
		videoLayout->addSpacing(10);
		videoLayout->addWidget(videoRatio_);
		videoLayout->addLayout(videoRatioLayout_);
		videoLayout->addSpacing(10);
		videoLayout->addWidget(frame_);
		videoLayout->addLayout(frameLayout_);
		videoLayout->addSpacing(10);
		videoLayout->addWidget(sppLabel);
		videoLayout->addWidget(sppSpinbox_);
		videoLayout->addSpacing(10);
		videoLayout->addWidget(bouncesLabel_);
		videoLayout->addWidget(bouncesSpinbox_);
		videoLayout->addSpacing(10);
		videoLayout->addWidget(crfLabel);
		videoLayout->addWidget(crfSpinbox);
		videoLayout->setContentsMargins(20, 10, 0, 0);

		auto markLayout = new QVBoxLayout;
		markLayout->addWidget(markButton_, 0, Qt::AlignCenter);

		auto infoLayout = new QVBoxLayout;
		infoLayout->addWidget(animation_);
		infoLayout->addWidget(summary_);
		infoLayout->addWidget(currentFrame_);
		infoLayout->addWidget(timeTotal_);

		auto apertureLayout = new QHBoxLayout;
		apertureLayout->addWidget(apertureLabel_);
		apertureLayout->addWidget(apertureSpinbox_);

		auto focalNameLayout = new QVBoxLayout;
		focalNameLayout->addWidget(focalDistanceName_, 0, Qt::AlignLeft);
		focalNameLayout->addStretch();
		focalNameLayout->setSpacing(0);
		focalNameLayout->setContentsMargins(0, 2, 0, 0);

		auto focalDistanceLayout = new QHBoxLayout;
		focalDistanceLayout->addWidget(focalTargetButton_, 0, Qt::AlignLeft);
		focalDistanceLayout->addLayout(focalNameLayout);
		focalDistanceLayout->addStretch();
		focalDistanceLayout->addWidget(focalDistanceSpinbox_, 0, Qt::AlignRight | Qt::AlignBottom);

		auto cameraLayout = new QVBoxLayout;
		cameraLayout->addWidget(dofInfoLabel_, 0, Qt::AlignLeft);
		cameraLayout->addLayout(apertureLayout);
		cameraLayout->addWidget(focalDistanceLabel_);
		cameraLayout->addLayout(focalDistanceLayout);

		markSpoiler_ = new Spoiler(u8"ˮӡ");
		markSpoiler_->setContentLayout(*markLayout);

		cameraSpoiler_ = new Spoiler(u8"�������");
		cameraSpoiler_->setContentLayout(*cameraLayout);
		cameraSpoiler_->toggleButton.click();

		videoSpoiler_ = new Spoiler(u8"��Ⱦ����");
		videoSpoiler_->setContentLayout(*videoLayout);
		videoSpoiler_->toggleButton.click();

		infoSpoiler_ = new Spoiler(u8"��Ƶ��Ϣ");
		infoSpoiler_->setContentLayout(*infoLayout);

		auto contentLayout = new QVBoxLayout();
		contentLayout->addWidget(cameraSpoiler_);
		contentLayout->addWidget(videoSpoiler_);
		contentLayout->addWidget(markSpoiler_);
		contentLayout->addWidget(infoSpoiler_);
		contentLayout->addStretch();

		auto contentWidget = new QWidget;
		contentWidget->setLayout(contentLayout);

		contentWidgetArea_ = new QScrollArea();
		contentWidgetArea_->setWidget(contentWidget);
		contentWidgetArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		contentWidgetArea_->setWidgetResizable(true);

		mainLayout_ = new QVBoxLayout();
		mainLayout_->addWidget(contentWidgetArea_);
		mainLayout_->addStretch();
		mainLayout_->addWidget(recordButton_, 0, Qt::AlignCenter);
		mainLayout_->setContentsMargins(0, 0, 0, 10);

		mainWidget_ = new QWidget;
		mainWidget_->setLayout(mainLayout_);

		this->setWidget(mainWidget_);

		connect(recordButton_, SIGNAL(clicked()), this, SLOT(clickEvent()));
		connect(select1_, SIGNAL(toggled(bool)), this, SLOT(select1Event(bool)));
		connect(select2_, SIGNAL(toggled(bool)), this, SLOT(select2Event(bool)));
		connect(speed1_, SIGNAL(toggled(bool)), this, SLOT(speed1Event(bool)));
		connect(speed2_, SIGNAL(toggled(bool)), this, SLOT(speed2Event(bool)));
		connect(speed3_, SIGNAL(toggled(bool)), this, SLOT(speed3Event(bool)));
		connect(speed4_, SIGNAL(toggled(bool)), this, SLOT(speed4Event(bool)));
		connect(start_, SIGNAL(valueChanged(int)), this, SLOT(startEvent(int)));
		connect(end_, SIGNAL(valueChanged(int)), this, SLOT(endEvent(int)));
		connect(sppSpinbox_, SIGNAL(valueChanged(int)), this, SLOT(onSppChanged(int)));
		connect(bouncesSpinbox_, SIGNAL(valueChanged(int)), this, SLOT(onBouncesChanged(int)));
		connect(crfSpinbox, SIGNAL(valueChanged(double)), this, SLOT(onCrfChanged(double)));
		connect(apertureSpinbox_, SIGNAL(valueChanged(double)), this, SLOT(onApertureChanged(double)));
		connect(focalDistanceSpinbox_, SIGNAL(valueChanged(double)), this, SLOT(onFocalDistanceChanged(double)));
		connect(timer_, SIGNAL(timeout()), this, SLOT(timeEvent()));
	}

	RecordDock::~RecordDock() noexcept
	{
	}

	void
	RecordDock::startRecord(QString fileName)
	{
		auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
		if (behaviour)
		{
			if (behaviour->startRecord(fileName.toUtf8().data()))
			{
				start_->setEnabled(false);
				end_->setEnabled(false);
				timer_->start();
				recordButton_->setText(u8"ֹͣ��Ⱦ");
			}
			else
			{
				QMessageBox msg(this);
				msg.setWindowTitle(tr("Error"));
				msg.setText(u8"�����ļ�ʧ��");
				msg.setIcon(QMessageBox::Information);
				msg.setStandardButtons(QMessageBox::Ok);

				msg.exec();
			}
		}
	}

	void
	RecordDock::stopRecord()
	{
		auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
		if (behaviour)
		{
			timer_->stop();
			start_->setEnabled(true);
			end_->setEnabled(true);
			recordButton_->setText(u8"��ʼ��Ⱦ");
			behaviour->stopRecord();
		}
	}

	void
	RecordDock::updateTarget()
	{
		auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
		if (behaviour)
		{
			auto hit = behaviour->getProfile()->dragModule->selectedItemHover_;
			if (hit)
			{
				behaviour->getProfile()->playerModule->dofTarget = hit;

				auto object = hit->object.lock();
				auto renderer = object->getComponent<octoon::MeshRendererComponent>();
				auto& materials = renderer->getMaterials();

				focalDistanceName_->setText(QString::fromStdString(u8"Ŀ�꣺" + materials[hit->mesh]->getName()));
				focalDistanceSpinbox_->setValue(0);
				focalDistanceSpinbox_->setSpecialValueText(u8"�Զ����");
			}
			else
			{
				focalDistanceName_->setText(QString::fromStdString(u8"Ŀ�꣺��"));
				focalDistanceSpinbox_->setValue(10);
				focalDistanceSpinbox_->setSpecialValueText(QString());
			}
		}
	}

	void
	RecordDock::onSppChanged(int value)
	{
		auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
		if (behaviour)
			behaviour->getProfile()->playerModule->spp = value;
	}

	void
	RecordDock::onBouncesChanged(int value)
	{
		auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
		if (behaviour)
			behaviour->getComponent<flower::OfflineComponent>()->setMaxBounces(value);
	}

	void
	RecordDock::onCrfChanged(double value)
	{
		auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
		if (behaviour)
			behaviour->getProfile()->h265Module->crf = value;
	}

	void
	RecordDock::onApertureChanged(double value)
	{
		auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
		if (behaviour)
			behaviour->getProfile()->entitiesModule->camera->getComponent<octoon::FilmCameraComponent>()->setAperture(value);
	}

	void
	RecordDock::onFocalDistanceChanged(double value)
	{
		if (!focalDistanceSpinbox_->specialValueText().isEmpty())
		{
			focalDistanceSpinbox_->setSpecialValueText(QString());

			auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
			if (behaviour)
				focalDistanceSpinbox_->setValue(behaviour->getProfile()->entitiesModule->camera->getComponent<octoon::FilmCameraComponent>()->getFocalDistance());
		}
		else
		{
			auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
			if (behaviour)
				behaviour->getProfile()->entitiesModule->camera->getComponent<octoon::FilmCameraComponent>()->setFocalDistance(value);
		}
	}

	void
	RecordDock::showEvent(QShowEvent* event)
	{
		this->repaint();
	}

	void
	RecordDock::resizeEvent(QResizeEvent* e) noexcept
	{
		contentWidgetArea_->resize(contentWidgetArea_->width(),
			mainWidget_->size().height() -
			this->recordButton_->height() - 
			mainLayout_->contentsMargins().bottom() * 2 -
			mainLayout_->contentsMargins().top() * 2);
	}

	void
	RecordDock::clickEvent()
	{
		VideoQuality quality = VideoQuality::Medium;
		if (select1_->isChecked())
			quality = VideoQuality::High;
		if (select2_->isChecked())
			quality = VideoQuality::Medium;

		auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
		if (behaviour)
		{
			behaviour->getProfile()->h265Module->setVideoQuality(quality);

			if (recordButton_->text() != u8"ֹͣ��Ⱦ")
			{
				QString fileName = QFileDialog::getSaveFileName(this, u8"¼����Ƶ", "", tr("MP4 Files (*.mp4)"));
				if (!fileName.isEmpty())
					this->startRecord(fileName);
			}
			else
			{
				this->stopRecord();
			}
		}
	}

	void
	RecordDock::select1Event(bool checked)
	{
		this->update();
	}
	
	void
	RecordDock::select2Event(bool checked)
	{
		this->update();
	}

	void
	RecordDock::speed1Event(bool checked)
	{
		if (checked)
		{
			auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
			if (behaviour)
				behaviour->getProfile()->playerModule->recordFps = 24;

			this->update();
		}
	}

	void
	RecordDock::speed2Event(bool checked)
	{
		if (checked)
		{
			auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
			if (behaviour)
				behaviour->getProfile()->playerModule->recordFps = 25;

			this->update();
		}
	}

	void
	RecordDock::speed3Event(bool checked)
	{
		if (checked)
		{
			auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
			if (behaviour)
				behaviour->getProfile()->playerModule->recordFps = 30;

			this->update();
		}
	}

	void
	RecordDock::speed4Event(bool checked)
	{
		if (checked)
		{
			auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
			if (behaviour)
				behaviour->getProfile()->playerModule->recordFps = 60;

			this->update();
		}
	}

	void
	RecordDock::startEvent(int value)
	{
		auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
		if (behaviour)
		{
			behaviour->getProfile()->playerModule->startFrame = value;
			this->update();
		}
	}

	void
	RecordDock::endEvent(int value)
	{
		auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
		if (behaviour)
		{
			behaviour->getProfile()->playerModule->endFrame = value;
			this->update();
		}
	}

	void
	RecordDock::timeEvent()
	{
		auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
		if (behaviour)
		{
			auto playerComponent = behaviour->getComponent<PlayerComponent>();
			if (playerComponent)
			{
				auto time = std::max<int>(0, std::round(behaviour->getProfile()->playerModule->curTime * 30.0f));
				currentFrame_->setText(QString(u8"��ǰ��Ƶ��Ⱦ֡����%1").arg(time));
			}
		}
	}

	void
	RecordDock::update()
	{
		auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
		if (behaviour)
		{
			auto playerComponent = dynamic_cast<PlayerComponent*>(behaviour->getComponent<PlayerComponent>());
			auto animLength = std::max<int>(1, std::round(playerComponent->timeLength() * 30.0f));

			auto startFrame = start_->value();
			auto endFrame = end_->value();
			auto time = std::max<int>(0, std::round(behaviour->getProfile()->playerModule->curTime * 30.0f));
			auto timeLength = std::max<int>(1, (endFrame - startFrame) / 30.0f * behaviour->getProfile()->playerModule->recordFps);

			animation_->setText(QString(u8"��Ƶ����֡����%1").arg(animLength));
			summary_->setText(QString(u8"��Ƶ��Ⱦ֡����%1").arg(timeLength));	
			currentFrame_->setText(QString(u8"��ǰ��Ƶ��Ⱦ֡����%1").arg(time));

			if (select1_->isChecked())
			{
				timeTotal_->setText(QString(u8"��Ƶ��ȾԤ��ʱ�䣺%1����").arg((timeLength * 10 / 60)));
			}
			else
			{
				recordButton_->setEnabled(true);
				timeTotal_->setText(QString(u8"��Ƶ��ȾԤ��ʱ�䣺%1����").arg((timeLength / 15 / 60)));
			}
		}
	}

	void 
	RecordDock::repaint()
	{
		auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
		if (behaviour)
		{
			auto playerComponent = behaviour->getComponent<PlayerComponent>();
			int timeLength = (int)std::round(playerComponent->timeLength() * 30);

			start_->setValue(0);
			end_->setValue(timeLength);

			auto profile = behaviour->getProfile();
			if (profile->playerModule->recordFps == 24)
				speed1_->click();
			else if (profile->playerModule->recordFps == 25)
				speed2_->click();
			else if (profile->playerModule->recordFps == 30)
				speed3_->click();
			else if (profile->playerModule->recordFps == 60)
				speed4_->click();

			sppSpinbox_->setValue(profile->playerModule->spp);
			crfSpinbox->setValue(profile->h265Module->crf);
			bouncesSpinbox_->setValue(behaviour->getComponent<flower::OfflineComponent>()->getMaxBounces());
			apertureSpinbox_->setValue(profile->entitiesModule->camera->getComponent<octoon::FilmCameraComponent>()->getAperture());

			if (!behaviour->getProfile()->playerModule->dofTarget)
				focalDistanceSpinbox_->setValue(profile->entitiesModule->camera->getComponent<octoon::FilmCameraComponent>()->getFocalDistance());

			this->update();
		}
	}
}