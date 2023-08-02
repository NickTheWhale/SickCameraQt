#pragma once
#include <vector>
#include <qimage.h>
#include "TinyColormap.hpp"

namespace frameset
{
	class Frame
	{
	public:
		struct ImageOptions
		{
			tinycolormap::ColormapType colorMap;
			bool autoExposure;
			uint16_t exposureLow;
			uint16_t exposureHigh;
			bool invert;
			bool logarithmic;

			ImageOptions(const tinycolormap::ColormapType& colorMap, bool autoExposure, const uint16_t& exposureLow, const uint16_t& exposureHigh, bool invert, bool logarithmic)
				: colorMap(colorMap), autoExposure(autoExposure), exposureLow(exposureLow), exposureHigh(exposureHigh), invert(invert), logarithmic(logarithmic)
			{}

			ImageOptions()
				: colorMap(tinycolormap::ColormapType::Gray), autoExposure(true), exposureLow(std::numeric_limits<uint16_t>::min()), exposureHigh(std::numeric_limits<uint16_t>::max()), invert(false), logarithmic(false)
			{}

			bool operator==(const ImageOptions& other) const
			{
				return colorMap == other.colorMap && autoExposure == other.autoExposure && exposureLow == other.exposureLow && exposureHigh == other.exposureHigh && invert == other.invert && logarithmic == other.logarithmic;
			}
		};

		Frame(const std::vector<uint16_t>& data, const uint32_t& height, const uint32_t& width, const uint32_t& number, const uint64_t& time);
		Frame();
		~Frame();

		const uint16_t at(const uint32_t x, const uint32_t y) const;
		const std::vector<uint16_t>& data() const;
		const QImage& toQImage(const ImageOptions& options);

		const uint32_t height() const;
		const uint32_t width() const;
		const uint32_t number() const;
		const uint64_t time() const;
		const QSize size() const;


	private:
		void updateImage(const ImageOptions& options);
		constexpr double linToLog(const double x);

		std::vector<uint16_t> buffer;
		uint32_t m_height;
		uint32_t m_width;
		uint32_t m_number;
		uint64_t m_time;

		// lazy toQImage evaluation
		QImage lastImage;
		ImageOptions lastImageOptions;
	};
}