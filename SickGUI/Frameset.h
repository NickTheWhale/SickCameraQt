#pragma once
#include <vector>
#include <qrect.h>
#include <qimage.h>
#include <TinyColormap.hpp>

namespace frameset
{
	struct ImageOptions
	{
		tinycolormap::ColormapType colormap;
		bool autoExposure;
		uint16_t exposureLow;
		uint16_t exposureHigh;
		bool invert;
		bool logarithmic;

		ImageOptions(
			const tinycolormap::ColormapType& colormap,
			bool autoExposure,
			const uint16_t& exposureLow,
			const uint16_t& exposureHigh,
			bool invert,
			bool logarithmic)
			: colormap(colormap), autoExposure(autoExposure), exposureLow(exposureLow), exposureHigh(exposureHigh), invert(invert), logarithmic(logarithmic)
		{}

		ImageOptions()
			: colormap(tinycolormap::ColormapType::Gray), autoExposure(true), exposureLow(std::numeric_limits<uint16_t>::min()), exposureHigh(std::numeric_limits<uint16_t>::max()), invert(false), logarithmic(false)
		{}

		bool operator==(const ImageOptions& other) const
		{
			return colormap == other.colormap && autoExposure == other.autoExposure && exposureLow == other.exposureLow && exposureHigh == other.exposureHigh && invert == other.invert && logarithmic == other.logarithmic;
		}
	};

	struct Frame
	{
		std::vector<uint16_t> data;
		uint32_t height;
		uint32_t width;
		uint32_t number;
		uint64_t time;
	};

	struct Frameset
	{
		Frame depth;
		Frame intensity;
		Frame state;
	};

	// frame functions
	const uint16_t at(const Frame& frame, const uint32_t x, const uint32_t y);
	const QSize size(const Frame& frame);
	const bool isEmpty(const Frame& frame);
	const bool isValid(const Frame& frame);
	void clip(Frame& frame, uint16_t lower, uint16_t upper);
	void clamp(Frame& frame, uint16_t lower, uint16_t upper);
	void mask(Frame& frame, const QRectF& maskNorm);
	const QImage toQImage(const Frame& frame, const ImageOptions& options);
	const Frame difference(const Frame& lhs, const Frame& rhs);

	// frameset functions
	const bool isUniform(const Frameset& fs);
	const bool isEmpty(const Frameset& fs);
	const bool isValid(const Frameset& fs);
	void mask(Frameset& fs, const QRectF& maskNorm);
}