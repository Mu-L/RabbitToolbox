#include "title_window.h"

namespace flower
{
	TitleWindow::TitleWindow(QWidget* parent, const std::shared_ptr<flower::FlowerProfile>& profile) noexcept
		: QWidget(parent)
		, maxNormal_(false)
		, profile_(profile)
	{
		this->setObjectName("TitleWindow");
		this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

		maxPix_ = QIcon::fromTheme("res", QIcon(":res/icons/maximize.png"));
		restorePix_ = QIcon::fromTheme("res", QIcon(":res/icons/restore.png"));
		vipPix_ = QIcon::fromTheme("res", QIcon(":res/icons/vip.png"));
		vipOnPix_ = QIcon::fromTheme("res", QIcon(":res/icons/vip-on.png"));

		minimizeButton_.setObjectName("minimize");
		minimizeButton_.setToolTip(tr("Minimize"));

		maximizeButton_.setObjectName("maximize");
		maximizeButton_.setToolTip(tr("Maximize"));
		maximizeButton_.setEnabled(false);

		closeButton_.setObjectName("close");
		closeButton_.setToolTip(tr("Close"));

		settingButton_.setObjectName("setting");
		settingButton_.setToolTip(tr("Global Settings"));

		vipButton_.setVisible(false);
		vipButton_.setObjectName("vip");
		vipButton_.setToolTip(tr("VIP"));

		logoButton_.setObjectName("logo");
		logoButton_.setContentsMargins(0, 0, 5, 0);

		titleLabel_.setObjectName("title");
		titleLabel_.setText(tr("Flowers Render (Alpha Version)"));

		layout_.setObjectName("titleLayout");
		layout_.addWidget(&logoButton_);
		layout_.addWidget(&titleLabel_);
		layout_.addWidget(&vipButton_);
		layout_.addWidget(&settingButton_);
		layout_.addWidget(&minimizeButton_);
		layout_.addWidget(&maximizeButton_);
		layout_.addWidget(&closeButton_);

		layout_.setSpacing(0);
		layout_.insertStretch(2, 500);
		layout_.setContentsMargins(10, 0, 10, 0);
		setLayout(&layout_);

		this->connect(&closeButton_, SIGNAL(clicked()), parent, SLOT(close()));
		this->connect(&minimizeButton_, SIGNAL(clicked()), this, SLOT(showSmall()));
		this->connect(&maximizeButton_, SIGNAL(clicked()), this, SLOT(showMaxRestore()));
		this->connect(&settingButton_, SIGNAL(clicked()), this, SLOT(showSettingPlane()));
		this->connect(&vipButton_, SIGNAL(clicked()), this, SLOT(showVipPlane()));
	}

	TitleWindow::~TitleWindow() noexcept
	{
	}

	void
	TitleWindow::showSmall() noexcept
	{
		parentWidget()->showMinimized();
	}

	void
	TitleWindow::showMaxRestore() noexcept
	{
		if (maxNormal_)
		{
			parentWidget()->setMinimumSize(size_);
			parentWidget()->showNormal();
			parentWidget()->showMaximized();
			parentWidget()->showNormal();
			maxNormal_ = !maxNormal_;
			maximizeButton_.setIcon(maxPix_);
		}
		else
		{
			size_ = parentWidget()->size();
			parentWidget()->showMaximized();
			maxNormal_ = !maxNormal_;
			maximizeButton_.setIcon(restorePix_);
		}
	}

	void
	TitleWindow::showSettingPlane() noexcept
	{
		emit settingSignal();
	}

	void
	TitleWindow::showVipPlane() noexcept
	{
		emit vipSignal();
	}

	void
	TitleWindow::loginSuccess() noexcept
	{
		vipButton_.setIcon(vipOnPix_);
	}

	void
	TitleWindow::loginOut() noexcept
	{
		vipButton_.setIcon(vipPix_);
	}

	void
	TitleWindow::mousePressEvent(QMouseEvent* e) noexcept
	{
		allowMove_ = true;
		startPos_ = e->globalPos();
		clickPos_ = mapToParent(e->pos());
	}

	void
	TitleWindow::mouseReleaseEvent(QMouseEvent* e) noexcept
	{
		allowMove_ = false;
	}

	void
	TitleWindow::mouseMoveEvent(QMouseEvent* e) noexcept
	{
		if (allowMove_ && !maxNormal_)
			parentWidget()->move(e->globalPos() - clickPos_);
	}
}