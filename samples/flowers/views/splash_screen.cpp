﻿#include "splash_screen.h"
#include <qapplication.h>
#include <qscreen.h>

#include "flower_version.h"

namespace flower
{
	SplashScreen::SplashScreen()
	{
		this->setFrameShape(Panel);
		this->setObjectName("SplashWindow");
		this->setWindowFlags(Qt::FramelessWindowHint);

		logo = new QPushButton(this);
		logo->setObjectName("logo");

		widget = new QWidget(this);
		widget->setObjectName("text");
		widget->setFixedSize(290, 100);

		englishName = new QLabel(widget);
		englishName->setObjectName("title");
		englishName->setText(tr("Flower Render Toolbox 2022"));
		englishName->setAlignment(Qt::AlignLeft);

		copyrightInfo = new QLabel(widget);
		copyrightInfo->setObjectName("copyright");
		copyrightInfo->setText(tr("2022 Flowers-Softwares (R). All Rights Reserved."));
		copyrightInfo->setAlignment(Qt::AlignLeft);

		vlayout = new QVBoxLayout(widget);
		vlayout->addWidget(englishName);
		vlayout->addWidget(copyrightInfo);
		vlayout->setContentsMargins(0, 10, 0, 0);

		hlayout = new QHBoxLayout();
		hlayout->addWidget(logo, 0, Qt::AlignRight);
		hlayout->addWidget(widget, 0, Qt::AlignLeft);
		hlayout->setContentsMargins(20, 0, 0, 10);
		hlayout->setSpacing(10);

		versionInfo = new QLabel(this);
		versionInfo->setObjectName("version");
		versionInfo->setText(tr("Version: ") + QString::fromStdString(FLOWERS_VERSION));
		versionInfo->setAlignment(Qt::AlignRight);

		message = new QLabel(this);
		message->setObjectName("message");
		message->setText(tr("Loading..."));
		message->setAlignment(Qt::AlignLeft);

		mainLayout = new QVBoxLayout();
		mainLayout->addWidget(versionInfo, 0, Qt::AlignRight | Qt::AlignTop);
		mainLayout->addLayout(hlayout);
		mainLayout->addWidget(message, 0, Qt::AlignLeft | Qt::AlignBottom);
		mainLayout->setContentsMargins(0, 0, 0, 0);

		this->setLayout(mainLayout);
	}

	SplashScreen::~SplashScreen()
	{
	}

	void
	SplashScreen::showEvent(QShowEvent* event) noexcept
	{
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
		QRect rect = QApplication::primaryScreen()->geometry();
#else
		int currentScreen = QApplication::desktop()->screenNumber(this);
		QRect rect = QGuiApplication::screens().at(currentScreen)->geometry();
#endif
		
		this->move((rect.width() - this->width()) / 2, (rect.height() - this->height()) / 2);
	}

	SplashListener::SplashListener(SplashScreen* splash, const std::string& path)
		: splash_(splash)
		, stream_(path)
	{
	}

	void
	SplashListener::onMessage(std::string_view message) noexcept
	{
		if (splash_)
		{
			splash_->message->setText(QString::fromStdString(std::string(message)));
			splash_->repaint();
		}

		if (stream_.good())
		{
			stream_ << message;
			stream_ << "\n";
			stream_.flush();
		}
	}
}