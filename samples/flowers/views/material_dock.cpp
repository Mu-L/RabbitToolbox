#include "material_dock.h"
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qevent.h>
#include <qscrollbar.h>
#include <qdrag.h>
#include <qmimedata.h>
#include <qapplication.h>
#include <codecvt>
#include <qpainter.h>
#include <qcolordialog.h>
#include <qtreewidget.h>

namespace flower
{
	constexpr auto imageFormat = "All Files(*.jpeg *.jpg *.png *.tga );; JPEG Files (*.jpeg *.jpg);; PNG Files (*.png);; TGA Files (*.tga)";

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

	QIcon createColorIcon(QColor color, int w = 50, int h = 26)
	{
		QPixmap pixmap(w, h);
		QPainter painter(&pixmap);
		painter.setPen(Qt::NoPen);
		painter.fillRect(QRect(0, 0, w, h), color);
		return QIcon(pixmap);
	}

	void
	MaterialEditWindow::MaterialUi::init(const QString& name, std::uint32_t flags)
	{
		this->path = nullptr;
		this->image = nullptr;
		this->check = nullptr;
		this->mapLayout = nullptr;
		this->spoiler = nullptr;
		this->label_ = nullptr;
		this->slider = nullptr;
		this->spinBox = nullptr;
		this->color = nullptr;

		if (flags & CreateFlags::TextureBit)
		{
			this->image = new QToolButton;
			this->image->setIcon(QIcon::fromTheme(":res/icons/append2.png"));
			this->image->setIconSize(QSize(48, 48));

			this->check = new QCheckBox;

			this->title = new QLabel;
			this->title->setText(name + tr(" Texture"));

			this->path = new QLabel;
			this->path->setMinimumSize(QSize(160, 20));

			this->titleLayout = new QHBoxLayout;
			this->titleLayout->addWidget(check, 0, Qt::AlignLeft);
			this->titleLayout->addWidget(title, 0, Qt::AlignLeft);
			this->titleLayout->addStretch();
			this->titleLayout->setSpacing(0);
			this->titleLayout->setContentsMargins(0, 2, 0, 0);

			auto textLayout = new QHBoxLayout;
			textLayout->setSpacing(2);
			textLayout->setContentsMargins(0, 2, 0, 0);
			textLayout->addWidget(this->path, 0, Qt::AlignLeft | Qt::AlignCenter);
			textLayout->addStretch();

			if (flags & CreateFlags::ColorBit)
			{
				this->color = new QToolButton;
				this->color->setIconSize(QSize(50, 30));

				textLayout->addWidget(this->color, 0, Qt::AlignRight);
			}

			this->rightLayout = new QVBoxLayout;
			this->rightLayout->setSpacing(0);
			this->rightLayout->setContentsMargins(0, 0, 0, 0);
			this->rightLayout->addLayout(this->titleLayout);
			this->rightLayout->addLayout(textLayout);
			this->rightLayout->addStretch();

			this->mapLayout = new QHBoxLayout;
			this->mapLayout->addWidget(image);
			this->mapLayout->addLayout(rightLayout);
		}

		if (flags & CreateFlags::ValueBit)
		{
			this->label_ = new QLabel;
			this->label_->setText(name);

			this->slider = new QSlider(Qt::Horizontal);
			this->slider->setObjectName("Value");
			this->slider->setMinimum(0);
			this->slider->setMaximum(100);
			this->slider->setValue(0);
			this->slider->setFixedWidth(260);

			this->spinBox = new DoubleSpinBox;
			this->spinBox->setFixedWidth(50);
			this->spinBox->setMaximum(1.0f);
			this->spinBox->setSingleStep(0.03f);
			this->spinBox->setAlignment(Qt::AlignRight);
			this->spinBox->setValue(0.0f);

			auto HLayout = new QHBoxLayout();
			HLayout->addWidget(this->label_, 0, Qt::AlignLeft);
			HLayout->addWidget(this->spinBox, 0, Qt::AlignRight);

			auto layout = new QVBoxLayout();
			if (this->mapLayout)
				layout->addLayout(this->mapLayout);
			layout->addLayout(HLayout);
			layout->addWidget(this->slider);
			layout->setContentsMargins(20, 5, 50, 0);
			this->mainLayout = layout;

			if (flags & CreateFlags::SpoilerBit)
			{
				this->spoiler = new Spoiler(name);
				this->spoiler->setFixedWidth(340);
				this->spoiler->setContentLayout(*this->mainLayout);
			}
		}
		else
		{
			if (this->mapLayout)
			{
				this->mapLayout->setContentsMargins(20, 5, 50, 0);
				this->mainLayout = this->mapLayout;
			}

			if (flags & CreateFlags::SpoilerBit)
			{
				this->spoiler = new Spoiler(name);
				this->spoiler->setFixedWidth(340);
				this->spoiler->setContentLayout(*mapLayout);
			}
		}
	}

	void
	MaterialEditWindow::MaterialUi::resetState()
	{
		this->texture = nullptr;
		if (this->path) this->path->clear();
		if (this->image) this->image->setIcon(QIcon::fromTheme(":res/icons/append2.png"));
	}

	octoon::hal::GraphicsTexturePtr
	MaterialEditWindow::MaterialUi::setImage(const QString& filepath)
	{
		auto textureData = octoon::TextureLoader::load(filepath.toStdString());
		auto width = textureData->getTextureDesc().getWidth();
		auto height = textureData->getTextureDesc().getHeight();

		QImage qimage;

		switch (textureData->getTextureDesc().getTexFormat())
		{
		case octoon::hal::GraphicsFormat::R8G8B8SNorm:
		case octoon::hal::GraphicsFormat::R8G8B8UNorm:
		case octoon::hal::GraphicsFormat::R8G8B8SRGB:
		{
			std::uint8_t* data_ = nullptr;
			if (textureData->map(0, 0, width, height, 0, (void**)&data_))
			{
				qimage = QImage(data_, width, height, QImage::Format::Format_RGB888);
				textureData->unmap();
			}
		}
		break;
		case octoon::hal::GraphicsFormat::R8G8B8A8SNorm:
		case octoon::hal::GraphicsFormat::R8G8B8A8UNorm:
		case octoon::hal::GraphicsFormat::R8G8B8A8SRGB:
		{
			std::uint8_t* data_ = nullptr;
			if (textureData->map(0, 0, width, height, 0, (void**)&data_))
			{
				qimage = QImage(data_, width, height, QImage::Format::Format_RGBA8888);
				textureData->unmap();
			}
		}
		break;
		case octoon::hal::GraphicsFormat::B8G8R8SNorm:
		case octoon::hal::GraphicsFormat::B8G8R8UNorm:
		case octoon::hal::GraphicsFormat::B8G8R8SRGB:
		{
			std::uint8_t* data_ = nullptr;
			if (textureData->map(0, 0, width, height, 0, (void**)&data_))
			{
				auto size = width * height * 3;
				auto pixels = std::make_unique<std::uint8_t[]>(size);

				for (std::size_t i = 0; i < size; i += 3) {
					pixels[i] = data_[i + 2];
					pixels[i + 1] = data_[i + 1];
					pixels[i + 2] = data_[i];
				}

				qimage = QImage(pixels.get(), width, height, QImage::Format::Format_RGB888);
				textureData->unmap();
			}
		}
		break;
		case octoon::hal::GraphicsFormat::B8G8R8A8SNorm:
		case octoon::hal::GraphicsFormat::B8G8R8A8UNorm:
		case octoon::hal::GraphicsFormat::B8G8R8A8SRGB:
		{
			std::uint8_t* data_ = nullptr;
			if (textureData->map(0, 0, width, height, 0, (void**)&data_))
			{
				auto size = width * height * 4;
				auto pixels = std::make_unique<std::uint8_t[]>(size);

				for (std::size_t i = 0; i < size; i += 4) {
					pixels[i] = data_[i + 2];
					pixels[i + 1] = data_[i + 1];
					pixels[i + 2] = data_[i];
					pixels[i + 3] = data_[i + 3];
				}

				qimage = QImage(pixels.get(), width, height, QImage::Format::Format_RGBA8888);
				textureData->unmap();
			}
		}
		break;
		default:
			throw std::runtime_error("Failed to open file :" + filepath.toStdString());
		}

		QFontMetrics metrics(this->path->font());
		auto name = metrics.elidedText(QFileInfo(filepath).fileName(), Qt::ElideRight, this->path->width());

		this->path->setText(name);
		this->check->setCheckState(Qt::CheckState::Checked);
		this->texture = textureData;

		if (!qimage.isNull())
			this->image->setIcon(QIcon(QPixmap::fromImage(qimage.scaled(this->image->iconSize()))));

		return textureData;
	}

	MaterialEditWindow::MaterialEditWindow(const octoon::GameObjectPtr& behaviour)
		: behaviour_(behaviour)
	{
		backButton_ = new QToolButton;
		backButton_->setObjectName("back");
		backButton_->setIconSize(QSize(20, 20));

		title_ = new QLabel();
		title_->setText(tr("Material Properties"));

		titleLayout_ = new QHBoxLayout();
		titleLayout_->addWidget(backButton_, 0, Qt::AlignLeft);
		titleLayout_->addStretch();
		titleLayout_->addWidget(title_, 0, Qt::AlignCenter);
		titleLayout_->addStretch();

		this->albedo_.init(tr("Base Color"), CreateFlags::SpoilerBit | CreateFlags::ColorBit | CreateFlags::TextureBit);
		this->opacity_.init(tr("Opacity"), CreateFlags::SpoilerBit | CreateFlags::ValueBit | CreateFlags::TextureBit);
		this->normal_.init(tr("Normal"), CreateFlags::SpoilerBit | CreateFlags::TextureBit);
		this->roughness_.init(tr("Roughness"), CreateFlags::SpoilerBit | CreateFlags::ValueBit | CreateFlags::TextureBit);
		this->metalness_.init(tr("Metal"), CreateFlags::SpoilerBit | CreateFlags::ValueBit | CreateFlags::TextureBit);
		this->specular_.init(tr("Specular"), CreateFlags::SpoilerBit | CreateFlags::ValueBit | CreateFlags::TextureBit);
		this->anisotropy_.init(tr("Anisotropy"), CreateFlags::SpoilerBit | CreateFlags::ValueBit | CreateFlags::TextureBit);
		this->sheen_.init(tr("Cloth"), CreateFlags::SpoilerBit | CreateFlags::ValueBit | CreateFlags::TextureBit);
		this->clearcoat_.init(tr("Clear Coat"), CreateFlags::ValueBit | CreateFlags::TextureBit);
		this->clearcoatRoughness_.init(tr("Clear Coat Roughness"), CreateFlags::ValueBit | CreateFlags::TextureBit);
		this->subsurface_.init(tr("Subsurface"), CreateFlags::ValueBit | CreateFlags::TextureBit);
		this->subsurfaceValue_.init(tr("Subsurface Color"), CreateFlags::ColorBit | CreateFlags::TextureBit);
		this->refraction_.init(tr("Refraction"), CreateFlags::ValueBit);
		this->refractionIor_.init(tr("Refraction Ior"), CreateFlags::ValueBit);
		this->emissive_.init(tr("Emissive"), CreateFlags::SpoilerBit | CreateFlags::ValueBit | CreateFlags::ColorBit | CreateFlags::TextureBit);

		this->clearcoat_.mainLayout->setContentsMargins(0, 0, 0, 0);
		this->clearcoatRoughness_.mainLayout->setContentsMargins(0, 0, 0, 0);

		this->subsurface_.mainLayout->setContentsMargins(0, 0, 0, 0);
		this->subsurfaceValue_.mainLayout->setContentsMargins(0, 0, 0, 0);

		this->refraction_.mainLayout->setContentsMargins(0, 0, 0, 0);
		this->refractionIor_.mainLayout->setContentsMargins(0, 0, 0, 0);
		this->refractionIor_.spinBox->setMinimum(1.0f);
		this->refractionIor_.spinBox->setMaximum(10.0f);

		auto clearlayout = new QVBoxLayout();
		clearlayout->addLayout(this->clearcoat_.mainLayout);
		clearlayout->addLayout(this->clearcoatRoughness_.mainLayout);
		clearlayout->setContentsMargins(20, 5, 50, 0);

		auto subsurfaceLayout = new QVBoxLayout();
		subsurfaceLayout->addLayout(this->subsurface_.mainLayout);
		subsurfaceLayout->addLayout(this->subsurfaceValue_.mainLayout);
		subsurfaceLayout->setContentsMargins(20, 5, 50, 0);

		auto refractionLayout = new QVBoxLayout();
		refractionLayout->addLayout(this->refraction_.mainLayout);
		refractionLayout->addLayout(this->refractionIor_.mainLayout);
		refractionLayout->setContentsMargins(20, 5, 50, 0);

		this->clearCoatSpoiler_ = new Spoiler(tr("Clear Coat"));
		this->clearCoatSpoiler_->setContentLayout(*clearlayout);

		this->subsurfaceSpoiler_ = new Spoiler(tr("Subsurface Scattering"));
		this->subsurfaceSpoiler_->setContentLayout(*subsurfaceLayout);

		this->refractionSpoiler_ = new Spoiler(tr("Refraction"));
		this->refractionSpoiler_->setContentLayout(*refractionLayout);

		this->receiveShadowCheck_ = new QCheckBox;
		this->receiveShadowCheck_->setText(tr("Receive Shadow"));

		auto otherslayout = new QVBoxLayout();
		otherslayout->addWidget(this->receiveShadowCheck_, 0, Qt::AlignLeft);
		otherslayout->setContentsMargins(20, 5, 50, 0);

		this->othersSpoiler_ = new Spoiler(tr("Other"));
		this->othersSpoiler_->setContentLayout(*otherslayout);

		this->emissive_.spinBox->setMaximum(100.f);
		this->albedoColor_.setWindowModality(Qt::ApplicationModal);
		this->emissiveColor_.setWindowModality(Qt::ApplicationModal);

		auto contentLayout = new QVBoxLayout();
		contentLayout->addWidget(this->albedo_.spoiler, 0,Qt::AlignTop);
		contentLayout->addWidget(this->opacity_.spoiler, 0, Qt::AlignTop);
		contentLayout->addWidget(this->normal_.spoiler, 0,  Qt::AlignTop);
		contentLayout->addWidget(this->roughness_.spoiler, 0,  Qt::AlignTop);
		contentLayout->addWidget(this->specular_.spoiler, 0, Qt::AlignTop);
		contentLayout->addWidget(this->metalness_.spoiler, 0, Qt::AlignTop);
		contentLayout->addWidget(this->anisotropy_.spoiler, 0, Qt::AlignTop);
		contentLayout->addWidget(this->sheen_.spoiler, 0, Qt::AlignTop);
		contentLayout->addWidget(this->clearCoatSpoiler_, 0, Qt::AlignTop);
		contentLayout->addWidget(this->subsurfaceSpoiler_, 0, Qt::AlignTop);
		contentLayout->addWidget(this->refractionSpoiler_, 0, Qt::AlignTop);
		contentLayout->addWidget(this->emissive_.spoiler, 0, Qt::AlignTop);
		contentLayout->addWidget(this->othersSpoiler_, 0, Qt::AlignTop);
		contentLayout->addStretch();

		auto contentWidget = new QWidget;
		contentWidget->setLayout(contentLayout);

		auto contentWidgetArea = new QScrollArea();
		contentWidgetArea->setWidget(contentWidget);
		contentWidgetArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		contentWidgetArea->setWidgetResizable(true);

		auto mainLayout = new QVBoxLayout(this);
		mainLayout->addLayout(titleLayout_);
		mainLayout->addWidget(this->createSummary(), 0, Qt::AlignCenter);
		mainLayout->addWidget(contentWidgetArea);
		mainLayout->setSpacing(5);
		mainLayout->setContentsMargins(10, 10, 10, 10);

		connect(albedo_.image, SIGNAL(clicked()), this, SLOT(colorMapClickEvent()));
		connect(albedo_.check, SIGNAL(stateChanged(int)), this, SLOT(colorMapCheckEvent(int)));
		connect(albedo_.color, SIGNAL(clicked()), this, SLOT(colorClickEvent()));
		connect(&albedoColor_, SIGNAL(currentColorChanged(const QColor&)), this, SLOT(colorChangeEvent(const QColor&)));
		connect(&emissiveColor_, SIGNAL(currentColorChanged(const QColor&)), this, SLOT(emissiveChangeEvent(const QColor&)));
		connect(&subsurfaceColor_, SIGNAL(currentColorChanged(const QColor&)), this, SLOT(subsurfaceColorChangeEvent(const QColor&)));
		connect(opacity_.image, SIGNAL(clicked()), this, SLOT(opacityMapClickEvent()));
		connect(opacity_.check, SIGNAL(stateChanged(int)), this, SLOT(opacityMapCheckEvent(int)));
		connect(opacity_.spinBox, SIGNAL(valueChanged(double)), this, SLOT(opacityEditEvent(double)));
		connect(opacity_.slider, SIGNAL(valueChanged(int)), this, SLOT(opacitySliderEvent(int)));
		connect(normal_.image, SIGNAL(clicked()), this, SLOT(normalMapClickEvent()));
		connect(normal_.check, SIGNAL(stateChanged(int)), this, SLOT(normalMapCheckEvent(int)));
		connect(roughness_.image, SIGNAL(clicked()), this, SLOT(smoothnessMapClickEvent()));
		connect(roughness_.check, SIGNAL(stateChanged(int)), this, SLOT(smoothnessMapCheckEvent(int)));
		connect(roughness_.spinBox, SIGNAL(valueChanged(double)), this, SLOT(roughnessEditEvent(double)));
		connect(roughness_.slider, SIGNAL(valueChanged(int)), this, SLOT(roughnessSliderEvent(int)));
		connect(specular_.image, SIGNAL(clicked()), this, SLOT(specularMapClickEvent()));
		connect(specular_.check, SIGNAL(stateChanged(int)), this, SLOT(specularMapCheckEvent(int)));
		connect(specular_.spinBox, SIGNAL(valueChanged(double)), this, SLOT(specularEditEvent(double)));
		connect(specular_.slider, SIGNAL(valueChanged(int)), this, SLOT(specularSliderEvent(int)));
		connect(metalness_.image, SIGNAL(clicked()), this, SLOT(metalnessMapClickEvent()));
		connect(metalness_.check, SIGNAL(stateChanged(int)), this, SLOT(metalnessMapCheckEvent(int)));
		connect(metalness_.spinBox, SIGNAL(valueChanged(double)), this, SLOT(metalEditEvent(double)));
		connect(metalness_.slider, SIGNAL(valueChanged(int)), this, SLOT(metalSliderEvent(int)));
		connect(anisotropy_.image, SIGNAL(clicked()), this, SLOT(anisotropyMapClickEvent()));
		connect(anisotropy_.check, SIGNAL(stateChanged(int)), this, SLOT(anisotropyMapCheckEvent(int)));
		connect(anisotropy_.spinBox, SIGNAL(valueChanged(double)), this, SLOT(anisotropyEditEvent(double)));
		connect(anisotropy_.slider, SIGNAL(valueChanged(int)), this, SLOT(anisotropySliderEvent(int)));
		connect(sheen_.image, SIGNAL(clicked()), this, SLOT(sheenMapClickEvent()));
		connect(sheen_.check, SIGNAL(stateChanged(int)), this, SLOT(sheenMapCheckEvent(int)));
		connect(sheen_.spinBox, SIGNAL(valueChanged(double)), this, SLOT(sheenEditEvent(double)));
		connect(sheen_.slider, SIGNAL(valueChanged(int)), this, SLOT(sheenSliderEvent(int)));
		connect(clearcoat_.image, SIGNAL(clicked()), this, SLOT(clearcoatMapClickEvent()));
		connect(clearcoat_.check, SIGNAL(stateChanged(int)), this, SLOT(clearcoatMapCheckEvent(int)));
		connect(clearcoat_.spinBox, SIGNAL(valueChanged(double)), this, SLOT(clearcoatEditEvent(double)));
		connect(clearcoat_.slider, SIGNAL(valueChanged(int)), this, SLOT(clearcoatSliderEvent(int)));
		connect(clearcoatRoughness_.image, SIGNAL(clicked()), this, SLOT(clearcoatRoughnessMapClickEvent()));
		connect(clearcoatRoughness_.check, SIGNAL(stateChanged(int)), this, SLOT(clearcoatRoughnessMapCheckEvent(int)));
		connect(clearcoatRoughness_.spinBox, SIGNAL(valueChanged(double)), this, SLOT(clearcoatRoughnessEditEvent(double)));
		connect(clearcoatRoughness_.slider, SIGNAL(valueChanged(int)), this, SLOT(clearcoatRoughnessSliderEvent(int)));
		connect(subsurface_.image, SIGNAL(clicked()), this, SLOT(subsurfaceMapClickEvent()));
		connect(subsurface_.check, SIGNAL(stateChanged(int)), this, SLOT(subsurfaceMapCheckEvent(int)));
		connect(subsurface_.spinBox, SIGNAL(valueChanged(double)), this, SLOT(subsurfaceEditEvent(double)));
		connect(subsurface_.slider, SIGNAL(valueChanged(int)), this, SLOT(subsurfaceSliderEvent(int)));
		connect(subsurfaceValue_.image, SIGNAL(clicked()), this, SLOT(subsurfaceColorMapClickEvent()));
		connect(subsurfaceValue_.check, SIGNAL(stateChanged(int)), this, SLOT(subsurfaceColorMapCheckEvent(int)));
		connect(subsurfaceValue_.color, SIGNAL(clicked()), this, SLOT(subsurfaceColorClickEvent()));
		connect(refraction_.spinBox, SIGNAL(valueChanged(double)), this, SLOT(refractionEditEvent(double)));
		connect(refraction_.slider, SIGNAL(valueChanged(int)), this, SLOT(refractionSliderEvent(int)));
		connect(refractionIor_.spinBox, SIGNAL(valueChanged(double)), this, SLOT(refractionIorEditEvent(double)));
		connect(refractionIor_.slider, SIGNAL(valueChanged(int)), this, SLOT(refractionIorSliderEvent(int)));
		connect(emissive_.image, SIGNAL(clicked()), this, SLOT(emissiveMapClickEvent()));
		connect(emissive_.color, SIGNAL(clicked()), this, SLOT(emissiveClickEvent()));
		connect(emissive_.check, SIGNAL(stateChanged(int)), this, SLOT(emissiveMapCheckEvent(int)));
		connect(emissive_.spinBox, SIGNAL(valueChanged(double)), this, SLOT(emissiveEditEvent(double)));
		connect(emissive_.slider, SIGNAL(valueChanged(int)), this, SLOT(emissiveSliderEvent(int)));
		connect(receiveShadowCheck_, SIGNAL(stateChanged(int)), this, SLOT(shadowCheckEvent(int)));
	}

	MaterialEditWindow::~MaterialEditWindow()
	{
	}

	void
	MaterialEditWindow::setAlbedoMap(const QString& path)
	{
		try
		{
			if (!path.isEmpty())
			{
				auto texture = this->albedo_.setImage(path);
				if (texture)
					this->material_->setColorMap(texture);
			}
			else
			{
				this->material_->setColorMap(nullptr);
			}

			this->repaint();
		}
		catch (...)
		{
		}
	}

	void
	MaterialEditWindow::setNormalMap(const QString& path)
	{
		try
		{
			if (!path.isEmpty())
			{
				auto texture = this->normal_.setImage(path);
				if (texture)
					this->material_->setNormalMap(texture);
			}
			else
			{
				this->material_->setNormalMap(nullptr);
			}

			this->repaint();
		}
		catch (...)
		{
		}
	}

	void
	MaterialEditWindow::setOpacityMap(const QString& path)
	{
		try
		{
			if (!path.isEmpty())
			{
				auto texture = this->opacity_.setImage(path);
				if (texture)
				{
					this->opacity_.spinBox->setValue(1.0f);
					this->material_->setOpacity(1.0f);
					this->material_->setOpacityMap(this->opacity_.texture);
				}
			}
			else
			{
				this->material_->setOpacityMap(nullptr);
			}

			this->repaint();
		}
		catch (...)
		{
		}
	}

	void
	MaterialEditWindow::setRoughnessMap(const QString& path)
	{
		try
		{
			if (!path.isEmpty())
			{
				auto texture = this->roughness_.setImage(path);
				if (texture)
				{
					this->roughness_.spinBox->setValue(1.0f);
					this->material_->setRoughness(1.0f);
					this->material_->setRoughnessMap(this->roughness_.texture);
				}
			}
			else
			{
				this->material_->setRoughnessMap(nullptr);
			}

			this->repaint();
		}
		catch (...)
		{
		}
	}

	void
	MaterialEditWindow::setSpecularMap(const QString& path)
	{
		try
		{
			if (!path.isEmpty())
			{
				auto texture = this->specular_.setImage(path);
				if (texture)
				{
					this->specular_.spinBox->setValue(1.0f);
					this->material_->setSpecular(1.0f);
					this->material_->setSpecularMap(this->specular_.texture);
				}
			}
			else
			{
				this->material_->setSpecularMap(nullptr);
			}

			this->repaint();
		}
		catch (...)
		{
		}
	}

	void
	MaterialEditWindow::setMetalnessMap(const QString& path)
	{
		try
		{
			if (!path.isEmpty())
			{
				auto texture = this->metalness_.setImage(path);
				if (texture)
				{
					this->metalness_.spinBox->setValue(1.0f);
					this->material_->setMetalness(1.0f);
					this->material_->setMetalnessMap(this->metalness_.texture);
				}
			}
			else
			{
				this->material_->setMetalnessMap(nullptr);
			}

			this->repaint();
		}
		catch (...)
		{
		}
	}

	void
	MaterialEditWindow::setAnisotropyMap(const QString& path)
	{
		try
		{
			if (!path.isEmpty())
			{
				auto texture = this->anisotropy_.setImage(path);
				if (texture)
				{
					this->anisotropy_.spinBox->setValue(1.0f);
					this->material_->setAnisotropy(1.0f);
					this->material_->setAnisotropyMap(this->anisotropy_.texture);
				}
			}
			else
			{
				this->material_->setAnisotropyMap(nullptr);
			}

			this->repaint();
		}
		catch (...)
		{
		}
	}

	void
	MaterialEditWindow::setSheenMap(const QString& path)
	{
		try
		{
			if (!path.isEmpty())
			{
				auto texture = this->sheen_.setImage(path);
				if (texture)
				{
					this->sheen_.spinBox->setValue(1.0f);
					this->material_->setSheen(1.0f);
					this->material_->setSheenMap(this->sheen_.texture);
				}
			}
			else
			{
				this->material_->setSheenMap(nullptr);
			}

			this->repaint();
		}
		catch (...)
		{
		}
	}

	void
	MaterialEditWindow::setClearCoatMap(const QString& path)
	{
		try
		{
			if (!path.isEmpty())
			{
				auto texture = this->clearcoat_.setImage(path);
				if (texture)
				{
					this->clearcoat_.spinBox->setValue(1.0f);
					this->material_->setClearCoat(1.0f);
					this->material_->setClearCoatMap(this->clearcoat_.texture);
				}
			}
			else
			{
				this->material_->setClearCoatMap(nullptr);
			}

			this->repaint();
		}
		catch (...)
		{
		}
	}

	void
	MaterialEditWindow::setClearCoatRoughnessMap(const QString& path)
	{
		try
		{
			if (!path.isEmpty())
			{
				auto texture = this->clearcoatRoughness_.setImage(path);
				if (texture)
				{
					this->clearcoatRoughness_.spinBox->setValue(1.0f);
					this->material_->setClearCoatRoughness(1.0f);
					this->material_->setClearCoatRoughnessMap(this->clearcoatRoughness_.texture);
				}
			}
			else
			{
				this->material_->setClearCoatRoughnessMap(nullptr);
			}

			this->repaint();
		}
		catch (...)
		{
		}
	}

	void
	MaterialEditWindow::setSubsurfaceMap(const QString& path)
	{
		try
		{
			if (!path.isEmpty())
			{
				auto texture = this->subsurface_.setImage(path);
				if (texture)
				{
					this->subsurface_.spinBox->setValue(1.0f);
					this->material_->setSubsurface(1.0f);
					this->material_->setSubsurfaceMap(this->subsurface_.texture);
				}
			}
			else
			{
				this->material_->setSubsurfaceMap(nullptr);
			}

			this->repaint();
		}
		catch (...)
		{
		}
	}

	void
	MaterialEditWindow::setSubsurfaceColorMap(const QString& path)
	{
		try
		{
			if (!path.isEmpty())
			{
				auto texture = this->subsurfaceValue_.setImage(path);
				if (texture)
				{
					this->subsurfaceValue_.spinBox->setValue(1.0f);
					this->material_->setSubsurfaceColor(octoon::math::float3::One);
					this->material_->setSubsurfaceColorMap(this->subsurfaceValue_.texture);
				}
			}
			else
			{
				this->material_->setSubsurfaceMap(nullptr);
			}

			this->repaint();
		}
		catch (...)
		{
		}
	}

	void
	MaterialEditWindow::setEmissiveMap(const QString& path)
	{
		try
		{
			if (!path.isEmpty())
			{
				auto texture = this->emissive_.setImage(path);
				if (texture)
				{
					this->emissive_.spinBox->setValue(1.0f);
					this->material_->setEmissiveMap(this->emissive_.texture);
				}
			}
			else
			{
				this->material_->setEmissiveMap(nullptr);
			}

			this->repaint();
		}
		catch (...)
		{
		}
	}

	void
	MaterialEditWindow::colorClickEvent()
	{
		auto srgb = octoon::math::linear2srgb(this->material_->getColor());
		albedoColor_.setCurrentColor(QColor::fromRgbF(srgb.x, srgb.y, srgb.z));
		albedoColor_.show();
	}

	void
	MaterialEditWindow::colorChangeEvent(const QColor &color)
	{
		this->albedo_.color->setIcon(createColorIcon(color));
		this->material_->setColor(octoon::math::srgb2linear(octoon::math::float3(color.redF(), color.greenF(), color.blueF())));
		this->repaint();
	}

	void
	MaterialEditWindow::emissiveClickEvent()
	{
		auto srgb = octoon::math::linear2srgb(this->material_->getEmissive());
		emissiveColor_.setCurrentColor(QColor::fromRgbF(srgb.x, srgb.y, srgb.z));
		emissiveColor_.show();
	}

	void
	MaterialEditWindow::emissiveChangeEvent(const QColor &color)
	{
		this->emissive_.color->setIcon(createColorIcon(color));
		this->material_->setEmissive(octoon::math::srgb2linear(octoon::math::float3(color.redF(), color.greenF(), color.blueF())));
		this->repaint();
	}

	void
	MaterialEditWindow::colorMapClickEvent()
	{
		QString path = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr(imageFormat));
		if (!path.isEmpty())
			this->setAlbedoMap(path);
	}

	void
	MaterialEditWindow::opacityMapClickEvent()
	{
		QString path = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr(imageFormat));
		if (!path.isEmpty())
			this->setOpacityMap(path);
	}

	void
	MaterialEditWindow::normalMapClickEvent()
	{
		QString path = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr(imageFormat));
		if (!path.isEmpty())
			this->setNormalMap(path);
	}

	void
	MaterialEditWindow::specularMapClickEvent()
	{
		QString path = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr(imageFormat));
		if (!path.isEmpty())
			this->setSpecularMap(path);
	}

	void
	MaterialEditWindow::smoothnessMapClickEvent()
	{
		QString path = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr(imageFormat));
		if (!path.isEmpty())
			this->setRoughnessMap(path);
	}

	void
	MaterialEditWindow::metalnessMapClickEvent()
	{
		QString path = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr(imageFormat));
		if (!path.isEmpty())
			this->setMetalnessMap(path);
	}

	void
	MaterialEditWindow::anisotropyMapClickEvent()
	{
		QString path = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr(imageFormat));
		if (!path.isEmpty())
			this->setAnisotropyMap(path);
	}

	void
	MaterialEditWindow::sheenMapClickEvent()
	{
		QString path = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr(imageFormat));
		if (!path.isEmpty())
			this->setSheenMap(path);
	}

	void
	MaterialEditWindow::clearcoatMapClickEvent()
	{
		QString path = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr(imageFormat));
		if (!path.isEmpty())
			this->setClearCoatMap(path);
	}

	void
	MaterialEditWindow::clearcoatRoughnessMapClickEvent()
	{
		QString path = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr(imageFormat));
		if (!path.isEmpty())
			this->setClearCoatRoughnessMap(path);
	}

	void
	MaterialEditWindow::subsurfaceMapClickEvent()
	{
		QString path = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr(imageFormat));
		if (!path.isEmpty())
			this->setSubsurfaceMap(path);
	}

	void
	MaterialEditWindow::subsurfaceColorMapClickEvent()
	{
		QString path = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr(imageFormat));
		if (!path.isEmpty())
			this->setSubsurfaceColorMap(path);
	}

	void
	MaterialEditWindow::emissiveMapClickEvent()
	{
		QString path = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr(imageFormat));
		if (!path.isEmpty())
			this->setEmissiveMap(path);
	}

	void
	MaterialEditWindow::colorMapCheckEvent(int state)
	{
		if (state == Qt::Unchecked)
		{
			this->setAlbedoMap("");
		}
		else if (this->albedo_.texture)
		{
			this->material_->setColorMap(this->albedo_.texture);
			this->repaint();
		}
	}

	void
	MaterialEditWindow::opacityMapCheckEvent(int state)
	{
		if (state == Qt::Unchecked)
		{
			this->setOpacityMap("");
		}
		else if (this->opacity_.texture)
		{
			this->material_->setOpacityMap(this->opacity_.texture);
			this->repaint();
		}
	}

	void
	MaterialEditWindow::normalMapCheckEvent(int state)
	{
		if (state == Qt::Unchecked)
		{
			this->setNormalMap("");
		}
		else if (this->normal_.texture)
		{
			this->material_->setNormalMap(this->normal_.texture);
			this->repaint();
		}
	}

	void
	MaterialEditWindow::smoothnessMapCheckEvent(int state)
	{
		if (state == Qt::Unchecked)
		{
			this->setRoughnessMap("");
		}
		else if (this->roughness_.texture)
		{
			this->material_->setRoughnessMap(this->roughness_.texture);
			this->repaint();
		}
	}

	void
	MaterialEditWindow::specularMapCheckEvent(int state)
	{
		if (state == Qt::Unchecked)
		{
			this->setSpecularMap("");
		}
		else if (this->specular_.texture)
		{
			this->material_->setSpecularMap(this->specular_.texture);
			this->repaint();
		}
	}

	void
	MaterialEditWindow::metalnessMapCheckEvent(int state)
	{
		if (state == Qt::Unchecked)
		{
			this->setMetalnessMap("");
		}
		else if (this->metalness_.texture)
		{
			this->material_->setMetalnessMap(this->metalness_.texture);
			this->repaint();
		}
	}

	void
	MaterialEditWindow::anisotropyMapCheckEvent(int state)
	{
		if (state == Qt::Unchecked)
		{
			this->setAnisotropyMap("");
		}
		else if (this->anisotropy_.texture)
		{
			this->material_->setAnisotropyMap(this->anisotropy_.texture);
			this->repaint();
		}
	}

	void
	MaterialEditWindow::sheenMapCheckEvent(int state)
	{
		if (state == Qt::Unchecked)
		{
			this->setSheenMap("");
		}
		else if (this->sheen_.texture)
		{
			this->material_->setSheenMap(this->sheen_.texture);
			this->repaint();
		}
	}

	void
	MaterialEditWindow::clearcoatMapCheckEvent(int state)
	{
		if (state == Qt::Unchecked)
		{
			this->setClearCoatMap("");
		}
		else if (this->clearcoat_.texture)
		{
			this->material_->setClearCoatMap(this->clearcoat_.texture);
			this->repaint();
		}
	}

	void
	MaterialEditWindow::clearcoatRoughnessMapCheckEvent(int state)
	{
		if (state == Qt::Unchecked)
		{
			this->setClearCoatRoughnessMap("");
		}
		else if (this->clearcoatRoughness_.texture)
		{
			this->material_->setClearCoatRoughnessMap(this->clearcoatRoughness_.texture);
			this->repaint();
		}
	}

	void
	MaterialEditWindow::subsurfaceMapCheckEvent(int state)
	{
		if (state == Qt::Unchecked)
		{
			this->setSubsurfaceMap("");
		}
		else if (this->subsurface_.texture)
		{
			this->material_->setSubsurfaceMap(this->subsurface_.texture);
			this->repaint();
		}
	}

	void
	MaterialEditWindow::subsurfaceColorMapCheckEvent(int state)
	{
		if (state == Qt::Unchecked)
		{
			this->setSubsurfaceMap("");
		}
		else if (this->subsurfaceValue_.texture)
		{
			this->material_->setSubsurfaceColorMap(this->subsurfaceValue_.texture);
			this->repaint();
		}
	}

	void
	MaterialEditWindow::emissiveMapCheckEvent(int state)
	{
		if (state == Qt::Unchecked)
		{
			this->setEmissiveMap("");
		}
		else if (this->emissive_.texture)
		{
			this->material_->setEmissiveMap(this->emissive_.texture);
			this->repaint();
		}
	}

	QWidget*
	MaterialEditWindow::createSummary()
	{
		imageLabel_ = new QLabel();
		imageLabel_->setFixedSize(QSize(128, 128));

		textLabel_ = new QLabel();
		textLabel_->setText(tr("material"));

		QVBoxLayout* summaryLayout = new QVBoxLayout;
		summaryLayout->setContentsMargins(0, 0, 0, 0);
		summaryLayout->setSpacing(4);
		summaryLayout->addWidget(imageLabel_, 0, Qt::AlignCenter);
		summaryLayout->addWidget(textLabel_, 0, Qt::AlignCenter);
		summaryLayout->setContentsMargins(0, 0, 10, 0);

		QWidget* summaryWidget = new QWidget;
		summaryWidget->setLayout(summaryLayout);

		return summaryWidget;
	}

	void
	MaterialEditWindow::repaint()
	{
		auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
		if (behaviour && this->material_)
		{
			auto materialComponent = behaviour->getComponent<MaterialComponent>();
			QPixmap pixmap;
			materialComponent->repaintMaterial(this->material_, pixmap, imageLabel_->width(), imageLabel_->height());
			this->material_->setDirty(true);
			imageLabel_->setPixmap(pixmap);
		}
	}

	void
	MaterialEditWindow::setMaterial(const std::shared_ptr<octoon::Material>& material)
	{
		if (this->material_ != material)
		{
			this->material_ = material->downcast_pointer<octoon::MeshStandardMaterial>();

			this->opacity_.resetState();
			this->normal_.resetState();
			this->roughness_.resetState();
			this->specular_.resetState();
			this->metalness_.resetState();
			this->anisotropy_.resetState();
			this->sheen_.resetState();
			this->clearcoat_.resetState();
			this->clearcoatRoughness_.resetState();
			this->subsurface_.resetState();
			this->subsurfaceValue_.resetState();
			this->refraction_.resetState();
			this->refractionIor_.resetState();
			this->emissive_.resetState();

			auto albedoColor = octoon::math::linear2srgb(material_->getColor());
			auto emissiveColor = octoon::math::linear2srgb(material_->getEmissive());
			auto subsurfaceColor = octoon::math::linear2srgb(material_->getSubsurfaceColor());

			this->textLabel_->setText(QString::fromStdString(material_->getName()));
			this->albedo_.color->setIcon(createColorIcon(QColor::fromRgbF(albedoColor.x, albedoColor.y, albedoColor.z)));
			this->opacity_.spinBox->setValue(material_->getOpacity());
			this->roughness_.spinBox->setValue(material_->getRoughness());
			this->specular_.spinBox->setValue(material_->getSpecular());
			this->metalness_.spinBox->setValue(material_->getMetalness());
			this->anisotropy_.spinBox->setValue(material_->getAnisotropy());
			this->sheen_.spinBox->setValue(material_->getSheen());
			this->clearcoat_.spinBox->setValue(material_->getClearCoat());
			this->clearcoatRoughness_.spinBox->setValue(material_->getClearCoatRoughness());
			this->subsurface_.spinBox->setValue(material_->getSubsurface());
			this->subsurfaceValue_.color->setIcon(createColorIcon(QColor::fromRgbF(subsurfaceColor.x, subsurfaceColor.y, subsurfaceColor.z)));
			this->refraction_.spinBox->setValue(material_->getTransmission());
			this->refractionIor_.spinBox->setValue(material_->getRefractionRatio());
			this->emissive_.color->setIcon(createColorIcon(QColor::fromRgbF(emissiveColor.x, emissiveColor.y, emissiveColor.z)));
			this->emissive_.spinBox->setValue(material_->getEmissiveIntensity());
			this->receiveShadowCheck_->setChecked(material_->getReceiveShadow());

			auto colorMap = material_->getColorMap();
			if (colorMap)
				this->setAlbedoMap(QString::fromStdString(colorMap->getTextureDesc().getName()));

			auto opacityMap = material_->getOpacityMap();
			if (opacityMap)
				this->setOpacityMap(QString::fromStdString(opacityMap->getTextureDesc().getName()));

			auto normalMap = material_->getNormalMap();
			if (normalMap)
				this->setNormalMap(QString::fromStdString(normalMap->getTextureDesc().getName()));

			auto roughnessMap = material_->getRoughnessMap();
			if (roughnessMap)
				this->setRoughnessMap(QString::fromStdString(roughnessMap->getTextureDesc().getName()));

			auto metalnessMap = material_->getMetalnessMap();
			if (metalnessMap)
				this->setMetalnessMap(QString::fromStdString(metalnessMap->getTextureDesc().getName()));

			auto sheenMap = material_->getSheenMap();
			if (sheenMap)
				this->setSheenMap(QString::fromStdString(sheenMap->getTextureDesc().getName()));

			auto clearcoatMap = material_->getClearCoatMap();
			if (clearcoatMap)
				this->setClearCoatMap(QString::fromStdString(clearcoatMap->getTextureDesc().getName()));

			auto clearcoatRoughnessMap = material_->getClearCoatRoughnessMap();
			if (clearcoatRoughnessMap)
				this->setClearCoatRoughnessMap(QString::fromStdString(clearcoatRoughnessMap->getTextureDesc().getName()));

			auto subsurfaceMap = material_->getSubsurfaceMap();
			if (subsurfaceMap)
				this->setSubsurfaceMap(QString::fromStdString(subsurfaceMap->getTextureDesc().getName()));

			auto subsurfaceColorMap = material_->getSubsurfaceColorMap();
			if (subsurfaceColorMap)
				this->setSubsurfaceColorMap(QString::fromStdString(subsurfaceColorMap->getTextureDesc().getName()));

			auto emissiveColorMap = material_->getEmissiveMap();
			if (emissiveColorMap)
				this->setEmissiveMap(QString::fromStdString(emissiveColorMap->getTextureDesc().getName()));

			this->repaint();
		}
	}

	void
	MaterialEditWindow::emissiveSliderEvent(int value)
	{
		this->emissive_.spinBox->setValue(value);
	}

	void
	MaterialEditWindow::emissiveEditEvent(double value)
	{
		this->emissive_.slider->setValue(value);

		if (this->material_)
		{
			material_->setEmissiveIntensity(value);
			this->repaint();
		}
	}

	void
	MaterialEditWindow::opacitySliderEvent(int value)
	{
		this->opacity_.spinBox->setValue(value / 100.f);
	}

	void
	MaterialEditWindow::opacityEditEvent(double value)
	{
		this->opacity_.slider->setValue(value * 100.f);

		if (this->material_)
		{
			material_->setOpacity(value);
			if (value < 1.0f)
			{
				octoon::hal::GraphicsColorBlend blend;
				blend.setBlendEnable(true);
				blend.setBlendSrc(octoon::hal::BlendMode::SrcAlpha);
				blend.setBlendDest(octoon::hal::BlendMode::OneMinusSrcAlpha);

				std::vector<octoon::hal::GraphicsColorBlend> blends;
				blends.push_back(blend);

				material_->setColorBlends(std::move(blends));
			}
			else
			{
				material_->getColorBlends().shrink_to_fit();
			}

			this->repaint();
		}
	}

	void
	MaterialEditWindow::roughnessEditEvent(double value)
	{
		this->roughness_.slider->setValue(value * 100.f);

		if (this->material_)
		{
			material_->setRoughness(value);
			this->repaint();
		}
	}

	void
	MaterialEditWindow::roughnessSliderEvent(int value)
	{
		this->roughness_.spinBox->setValue(value / 100.0f);
	}

	void
	MaterialEditWindow::metalEditEvent(double value)
	{
		this->metalness_.slider->setValue(value * 100.f);

		if (this->material_)
		{
			material_->setMetalness(value);
			this->repaint();
		}
	}

	void
	MaterialEditWindow::metalSliderEvent(int value)
	{
		this->metalness_.spinBox->setValue(value / 100.0f);
	}

	void
	MaterialEditWindow::specularEditEvent(double value)
	{
		this->specular_.slider->setValue(value * 100.f);

		if (this->material_)
		{
			material_->setSpecular(value);
			this->repaint();
		}
	}

	void
	MaterialEditWindow::specularSliderEvent(int value)
	{
		this->specular_.spinBox->setValue(value / 100.0f);
	}

	void
	MaterialEditWindow::anisotropyEditEvent(double value)
	{
		this->anisotropy_.slider->setValue(value * 100.f);

		if (this->material_)
		{
			material_->setAnisotropy(value);
			this->repaint();
		}
	}

	void
	MaterialEditWindow::anisotropySliderEvent(int value)
	{
		this->anisotropy_.spinBox->setValue(value / 100.0f);
	}

	void
	MaterialEditWindow::clearcoatEditEvent(double value)
	{
		this->clearcoat_.slider->setValue(value * 100.f);

		if (this->material_)
		{
			material_->setClearCoat(value);
			this->repaint();
		}
	}

	void
	MaterialEditWindow::clearcoatSliderEvent(int value)
	{
		this->clearcoat_.spinBox->setValue(value / 100.0f);
	}

	void
	MaterialEditWindow::clearcoatRoughnessEditEvent(double value)
	{
		this->clearcoatRoughness_.slider->setValue(value * 100.f);

		if (this->material_)
		{
			material_->setClearCoatRoughness(value);
			this->repaint();
		}
	}

	void
	MaterialEditWindow::clearcoatRoughnessSliderEvent(int value)
	{
		this->clearcoatRoughness_.spinBox->setValue(value / 100.0f);
	}

	void
	MaterialEditWindow::sheenEditEvent(double value)
	{
		this->sheen_.slider->setValue(value * 100.f);

		if (this->material_)
		{
			material_->setSheen(value);
			this->repaint();
		}
	}

	void
	MaterialEditWindow::sheenSliderEvent(int value)
	{
		this->sheen_.spinBox->setValue(value / 100.0f);
	}

	void
	MaterialEditWindow::subsurfaceEditEvent(double value)
	{
		this->subsurface_.slider->setValue(value * 100.f);

		if (this->material_)
		{
			material_->setSubsurface(value);
			this->repaint();
		}
	}

	void
	MaterialEditWindow::subsurfaceSliderEvent(int value)
	{
		this->subsurface_.spinBox->setValue(value / 100.0f);
	}

	void
	MaterialEditWindow::subsurfaceColorClickEvent()
	{
		auto srgb = octoon::math::linear2srgb(this->material_->getSubsurfaceColor());
		subsurfaceColor_.setCurrentColor(QColor::fromRgbF(srgb.x, srgb.y, srgb.z));
		subsurfaceColor_.show();
	}

	void
	MaterialEditWindow::subsurfaceColorChangeEvent(const QColor &color)
	{
		this->subsurfaceValue_.color->setIcon(createColorIcon(color));
		this->material_->setSubsurfaceColor(octoon::math::srgb2linear(octoon::math::float3(color.redF(), color.greenF(), color.blueF())));
		this->repaint();
	}

	void
	MaterialEditWindow::refractionEditEvent(double value)
	{
		this->refraction_.slider->setValue(value * 100.f);

		if (this->material_)
		{
			material_->setTransmission(value);
			this->repaint();
		}
	}

	void
	MaterialEditWindow::refractionSliderEvent(int value)
	{
		this->refraction_.spinBox->setValue(value / 100.0f);
	}

	void
	MaterialEditWindow::refractionIorEditEvent(double value)
	{
		this->refractionIor_.slider->setValue(value * 10.f);

		if (this->material_)
		{
			material_->setRefractionRatio(value);
			this->repaint();
		}
	}

	void
	MaterialEditWindow::refractionIorSliderEvent(int value)
	{
		this->refractionIor_.spinBox->setValue(value / 10.0f);
	}

	void
	MaterialEditWindow::shadowCheckEvent(int state)
	{
		if (this->material_)
			material_->setReceiveShadow(state == Qt::Checked ? true : false);
	}

	void 
	MaterialEditWindow::closeEvent()
	{
		this->hide();
	}

	MaterialListWindow::MaterialListWindow() noexcept(false)
	{
		this->setResizeMode(QListView::Adjust);
		this->setViewMode(QListView::IconMode);
		this->setMovement(QListView::Static);
		this->setDefaultDropAction(Qt::DropAction::MoveAction);
		this->setSpacing(2);
		this->setStyleSheet("background:transparent;");
		this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	}

	MaterialListWindow::~MaterialListWindow() noexcept
	{
	}

	void
	MaterialListWindow::mouseMoveEvent(QMouseEvent *event)
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
						mimeData->setData("object/material", item->data(Qt::UserRole).toByteArray());

						auto drag = new QDrag(this);
						drag->setMimeData(mimeData);
						drag->setPixmap(label->pixmap(Qt::ReturnByValue));
						drag->setHotSpot(QPoint(drag->pixmap().width() / 2, drag->pixmap().height() / 2));
						drag->exec(Qt::MoveAction);
					}
				}
			}
		}

		QListWidget::mouseMoveEvent(event);
	}

	void
	MaterialListWindow::mousePressEvent(QMouseEvent *event)
	{
		if (event->button() == Qt::LeftButton)
			startPos = event->pos();

		QListWidget::mousePressEvent(event);
	}

	MaterialListPanel::MaterialListPanel()
	{
		mainWidget_ = new MaterialListWindow;
		mainWidget_->setIconSize(QSize(100, 100));

		mainLayout_ = new QVBoxLayout(this);
		mainLayout_->addWidget(mainWidget_, 0, Qt::AlignTop | Qt::AlignCenter);
		mainLayout_->addStretch();
		mainLayout_->setContentsMargins(0, 10, 0, 5);
	}

	MaterialListPanel::~MaterialListPanel() noexcept
	{
	}

	void
	MaterialListPanel::resizeEvent(QResizeEvent* e) noexcept
	{
		QMargins margins = mainLayout_->contentsMargins();
		mainWidget_->resize(this->width(), this->height() - margins.top() - margins.bottom());
	}

	MaterialDock::MaterialDock(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<FlowerProfile>& profile) noexcept(false)
		: behaviour_(behaviour)
		, profile_(profile)
	{
		this->setObjectName("MaterialDock");
		this->setWindowTitle(tr("Material"));
		this->setMouseTracking(true);

		listPanel_ = new MaterialListPanel();
		listPanel_->mainWidget_->setFixedWidth(340);

		modifyWidget_ = new MaterialEditWindow(behaviour);
		modifyWidget_->hide();

		mainLayout_ = new QVBoxLayout();
		mainLayout_->addWidget(listPanel_, 0, Qt::AlignTop | Qt::AlignCenter);
		mainLayout_->addWidget(modifyWidget_, 0, Qt::AlignTop | Qt::AlignCenter);
		mainLayout_->addStretch();
		mainLayout_->setContentsMargins(0, 0, 0, 0);

		widget_ = new QWidget;
		widget_->setLayout(mainLayout_);

		this->setWidget(widget_);

		connect(modifyWidget_->backButton_, SIGNAL(clicked()), this, SLOT(okEvent()));
		connect(listPanel_->mainWidget_, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(itemDoubleClicked(QListWidgetItem*)));

		behaviour->addMessageListener("editor:material:change", [this](const std::any&) {
			if (this->isVisible())
				this->updateList();
		});

		behaviour->addMessageListener("editor:material:selected", [this](const std::any& any_data) {
			if (this->isVisible())
			{
				auto uuid = QString::fromStdString(std::any_cast<std::string>(any_data));
				auto count = this->listPanel_->mainWidget_->count();
				for (int i = 0; i < count; i++)
				{
					auto item = this->listPanel_->mainWidget_->item(i);
					if (item->data(Qt::UserRole).toString() == uuid)
					{
						this->listPanel_->mainWidget_->setCurrentItem(item);
						break;
					}
				}
			}
		});
	}

	MaterialDock::~MaterialDock() noexcept
	{
	}

	void
	MaterialDock::showEvent(QShowEvent* event) noexcept
	{
		QMargins margins = mainLayout_->contentsMargins();
		modifyWidget_->hide();
		modifyWidget_->resize(widget_->size());
		listPanel_->resize(widget_->size());
		listPanel_->show();
		this->updateList();
	}

	void
	MaterialDock::closeEvent(QCloseEvent* event)
	{
		if (profile_->playerModule->isPlaying)
			event->ignore();
		else
			event->accept();
	}

	void
	MaterialDock::paintEvent(QPaintEvent* e) noexcept
	{
		modifyWidget_->resize(widget_->size());
		listPanel_->resize(widget_->size());

		QDockWidget::paintEvent(e);
	}

	void
	MaterialDock::okEvent()
	{
		modifyWidget_->hide();
		listPanel_->show();
	}

	void
	MaterialDock::itemClicked(QListWidgetItem* item)
	{
		if (behaviour_)
		{
			auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
			if (behaviour->isOpen())
			{
				auto selectedItem = behaviour->getProfile()->selectorModule->selectedItem_;
				if (selectedItem)
				{
					auto hit = selectedItem.value();
					auto materialComponent = behaviour->getComponent<MaterialComponent>();
					auto material = materialComponent->getMaterial(item->data(Qt::UserRole).toString().toStdString());

					auto meshRenderer = hit.object.lock()->getComponent<octoon::MeshRendererComponent>();
					if (meshRenderer)
						meshRenderer->setMaterial(material, hit.mesh);
				}
			}
		}
	}

	void
	MaterialDock::itemDoubleClicked(QListWidgetItem* item)
	{
		if (behaviour_)
		{
			auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
			if (behaviour->isOpen())
			{
				auto materialComponent = behaviour->getComponent<MaterialComponent>();
				auto material = materialComponent->getMaterial(item->data(Qt::UserRole).toString().toStdString());
				if (material)
				{
					listPanel_->hide();
					modifyWidget_->setMaterial(material);
					modifyWidget_->show();
				}
			}
		}
	}

	void
	MaterialDock::updateList()
	{
		auto behaviour = behaviour_->getComponent<flower::FlowerBehaviour>();
		if (behaviour)
		{
			auto materialComponent = behaviour->getComponent<MaterialComponent>();
			auto& materials = materialComponent->getMaterialList();

			listPanel_->mainWidget_->clear();

			std::map<QString, std::shared_ptr<QPixmap>> imageTable;

			for (auto& it : materials)
			{
				std::string path;
				std::string normalName;
				std::string textureName;

				auto mat = it.second;
				if (mat.find("preview") != mat.end())
					textureName = mat["preview"].get<nlohmann::json::string_t>();

				QListWidgetItem* item = new QListWidgetItem;
				item->setData(Qt::UserRole, QString::fromStdString(mat["uuid"]));
				item->setSizeHint(QSize(106, 130));

				QLabel* imageLabel = new QLabel;
				if (textureName.empty())
				{
					auto material = materialComponent->getMaterial(mat["uuid"].get<nlohmann::json::string_t>());
					if (material)
					{
						QPixmap pixmap;
						materialComponent->repaintMaterial(material, pixmap, 100, 100);
						imageLabel->setPixmap(pixmap);
					}
				}
				else
				{
					auto texpath = QString::fromStdString(path + textureName);
					if (!imageTable[texpath])
						imageTable[texpath] = std::make_shared<QPixmap>(texpath);

					imageLabel->setPixmap(*imageTable[texpath]);
				}

				QLabel* txtLabel = new QLabel(QString::fromStdString(mat["name"]));
				txtLabel->setFixedHeight(30);
				txtLabel->setToolTip(txtLabel->text());

				QVBoxLayout* widgetLayout = new QVBoxLayout;
				widgetLayout->setContentsMargins(0, 0, 0, 0);
				widgetLayout->setSpacing(0);
				widgetLayout->addWidget(imageLabel, 0, Qt::AlignCenter);
				widgetLayout->addWidget(txtLabel, 0, Qt::AlignCenter);

				QWidget* widget = new QWidget;
				widget->setLayout(widgetLayout);

				listPanel_->mainWidget_->addItem(item);
				listPanel_->mainWidget_->setItemWidget(item, widget);
			}
		}
	}
}