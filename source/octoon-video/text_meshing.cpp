#include <octoon/video/text_meshing.h>
#include <octoon/model/mesh.h>
#include <octoon/video/text_file.h>
#include <octoon/video/text_contour_group.h>
#include <octoon/runtime/except.h>

#include <ft2build.h>
#include <freetype/ftglyph.h>

namespace octoon
{
	namespace video
	{
		TextMeshing::TextMeshing() noexcept
			: font_(nullptr)
			, bezierSteps_(6)
			, pixelSize_(12)
		{
		}

		TextMeshing::TextMeshing(const char* path, std::uint16_t pixelsSize, std::uint16_t bezierSteps) noexcept
			: font_(std::make_shared<TextFile>(path))
			, bezierSteps_(bezierSteps)
			, pixelSize_(pixelsSize)
		{
		}

		TextMeshing::TextMeshing(const std::string& path, std::uint16_t pixelsSize, std::uint16_t bezierSteps) noexcept
			: font_(std::make_shared<TextFile>(path.c_str()))
			, bezierSteps_(bezierSteps)
			, pixelSize_(pixelsSize)
		{
		}

		TextMeshing::TextMeshing(TextFilePtr&& font, std::uint16_t pixelsSize, std::uint16_t bezierSteps) noexcept
			: font_(std::move(font))
			, bezierSteps_(bezierSteps)
			, pixelSize_(pixelsSize)
		{
		}

		TextMeshing::TextMeshing(const TextFilePtr& font, std::uint16_t pixelsSize, std::uint16_t bezierSteps) noexcept
			: font_(std::move(font))
			, bezierSteps_(bezierSteps)
			, pixelSize_(pixelsSize)
		{
		}

		TextMeshing::~TextMeshing() noexcept
		{
		}

		void
		TextMeshing::setFont(TextFilePtr&& font) noexcept
		{
			font_ = std::move(font);
		}

		void
		TextMeshing::setFont(const TextFilePtr& font) noexcept
		{
			font_ = font;
		}

		const TextFilePtr&
		TextMeshing::getFont() const noexcept
		{
			return font_;
		}

		void
		TextMeshing::setBezierSteps(std::uint16_t bezierSteps) noexcept
		{
			bezierSteps_ = bezierSteps;
		}

		std::uint16_t
		TextMeshing::getBezierSteps() const noexcept
		{
			return bezierSteps_;
		}

		void
		TextMeshing::setPixelsSize(std::uint16_t pixelsSize) noexcept
		{
			pixelSize_ = pixelsSize;
		}

		std::uint16_t
		TextMeshing::getPixelsSize() const noexcept
		{
			return pixelSize_;
		}

		TextMeshingPtr
		TextMeshing::clone() const noexcept
		{
			auto instance = std::make_shared<TextMeshing>();
			instance->setFont(this->getFont());
			instance->setPixelsSize(this->getPixelsSize());
			instance->setBezierSteps(this->getBezierSteps());

			return instance;
		}

		TextContourGroups makeTextContours(const TextMeshing& params, const std::wstring& string) noexcept(false)
		{
			assert(params.getFont());
			assert(params.getPixelsSize() > 0);
			assert(params.getBezierSteps() > 1);

			auto addPoints = [](video::TextContour& contours, const FT_Vector* contour, const char* tags, std::size_t n, std::uint16_t bezierSteps) -> void
			{
				math::float3 prev;
				math::float3 cur(contour[(n - 1) % n].x / 64.0f, contour[(n - 1) % n].y / 64.0f, 0.0);
				math::float3 next(contour[0].x / 64.0f, contour[0].y / 64.0f, 0.0);

				float olddir, dir = std::atan2((next - cur).y, (next - cur).x);
				float angle = 0.0f;

				for (std::size_t i = 0; i < n; i++)
				{
					prev = cur;
					cur = next;
					next = math::float3(contour[(i + 1) % n].x / 64.0f, contour[(i + 1) % n].y / 64.0f, 0.0f);

					olddir = dir;
					dir = std::atan2((next - cur).y, (next - cur).x);

					float t = dir - olddir;
					if (t < -math::PI) t += 2 * math::PI;
					if (t > math::PI) t -= 2 * math::PI;
					angle += t;

					switch (FT_CURVE_TAG(tags[i]))
					{
					case FT_Curve_Tag_On:
						contours.addPoints(cur);
						break;
					case FT_Curve_Tag_Cubic:
						contours.addPoints(prev, cur, next, math::float3(contour[(i + 2) % n].x / 64.0f, contour[(i + 2) % n].y / 64.0f, 0.0f), bezierSteps);
						break;
					case FT_Curve_Tag_Conic:
					{
						math::float3 prev2 = prev, next2 = next;

						if (FT_CURVE_TAG(tags[(i - 1 + n) % n]) == FT_Curve_Tag_Conic)
						{
							prev2 = (cur + prev) * 0.5f;
							contours.addPoints(prev2);
						}

						if (FT_CURVE_TAG(tags[(i + 1) % n]) == FT_Curve_Tag_Conic)
							next2 = (cur + next) * 0.5f;

						contours.addPoints(prev2, cur, next2, bezierSteps);
					}
					break;
					}
				}

				contours.addPoints(contours.at(0));
				contours.isClockwise(angle < 0.0f);
			};

			auto addContours = [&](const FT_GlyphSlot glyph, float offset, std::uint16_t bezierSteps)
			{
				video::TextContours contours(glyph->outline.n_contours);

				for (short i = 0; i < glyph->outline.n_contours; i++)
					contours[i] = std::make_unique<video::TextContour>();

				for (std::size_t startIndex = 0, i = 0; i < glyph->outline.n_contours; i++)
				{
					auto points = &glyph->outline.points[startIndex];
					auto tags = &glyph->outline.tags[startIndex];
					auto index = (glyph->outline.contours[i] - startIndex) + 1;

					startIndex = glyph->outline.contours[i] + 1;

					addPoints(*contours[i], points, tags, index, bezierSteps);
				}

				for (auto& contour : contours)
				{
					for (auto& pt : contour->points())
						pt.x += offset;
				}

				auto group = std::make_shared<video::TextContourGroup>();
				group->setContours(std::move(contours));

				return group;
			};

			if (::FT_Set_Pixel_Sizes((FT_Face)(params.getFont()->getFont()), params.getPixelsSize(), params.getPixelsSize()))
				throw runtime::runtime_error::create("FT_Set_Char_Size() failed (there is probably a problem with your font size", 3);

			FT_Face ftface = (FT_Face)params.getFont()->getFont();

			auto offset = ftface->glyph->advance.x;

			video::TextContourGroups groups;

			for (auto& ch : string)
			{
				FT_UInt index = FT_Get_Char_Index(ftface, ch);
				if (::FT_Load_Glyph(ftface, index, FT_LOAD_DEFAULT))
					throw runtime::runtime_error::create("FT_Load_Glyph failed.");

				FT_Glyph glyph;
				if (::FT_Get_Glyph(ftface->glyph, &glyph))
					throw runtime::runtime_error::create("FT_Get_Glyph failed.");

				if (glyph->format != FT_GLYPH_FORMAT_OUTLINE)
					throw runtime::runtime_error::create("Invalid Glyph Format.");

				groups.push_back(addContours(ftface->glyph, offset, params.getBezierSteps()));

				offset += ftface->glyph->bitmap_left + ftface->glyph->bitmap.width;
			}

			return groups;
		}

		model::Mesh makeText(const TextMeshing& params, const std::wstring& string) noexcept(false)
		{
			model::Mesh mesh = makeText(makeTextContours(params, std::move(string)));
			mesh.computeVertexNormals();
			mesh.computeBoundingBox();

			return mesh;
		}

		model::Mesh makeTextWireframe(const TextMeshing& params, const std::wstring& string) noexcept(false)
		{
			model::Mesh mesh = makeTextWireframe(makeTextContours(params, std::move(string)));
			mesh.computeBoundingBox();

			return mesh;
		}
	}
}