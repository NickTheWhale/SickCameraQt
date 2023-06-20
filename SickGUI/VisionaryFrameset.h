#pragma once
#include <qimage.h>
#include "TinyColormap.hpp"
#include <vector>
#include <algorithm>

namespace Frameset
{
	typedef struct
	{
		std::vector<uint16_t> depth;
		std::vector<uint16_t> intensity;
		std::vector<uint16_t> state;
		int height;
		int width;
		uint32_t number;
		uint64_t time;
	} frameset_t;

	namespace _internal
	{
		inline void frameToQImage(const std::vector<uint16_t>& frame, int width, int height, QImage& qImage, tinycolormap::ColormapType colorMap, bool invert)
		{
			qImage = QImage(width, height, QImage::Format_ARGB32_Premultiplied);
			auto max = *std::max_element(frame.begin(), frame.end());
			auto min = max;
			for (const auto& val : frame)
			{
				if (val > 0 && val < min)
					min = val;
			}

			QRgb* qImageData = reinterpret_cast<QRgb*>(qImage.bits());
			for (auto y = 0; y < height; ++y)
			{
				for (auto x = 0; x < width; ++x)
				{
					auto val = frame[y * width + x];
					double valNorm = (val - min) / static_cast<double>(max);
					tinycolormap::Color color = tinycolormap::GetColor(valNorm, colorMap);
					if (!invert)
						qImageData[y * width + x] = qRgba(color.ri(), color.gi(), color.bi(), 255);
					else
						qImageData[y * width + x] = qRgba(255 - color.ri(), 255 - color.gi(), 255 - color.bi(), 255);
				}
			}
		}
	}

	inline void depthToQImage(const frameset_t& fs, QImage& qImage, tinycolormap::ColormapType colorMap = tinycolormap::ColormapType::Gray, bool invert = false)
	{
		_internal::frameToQImage(fs.depth, fs.width, fs.height, qImage, colorMap, invert);
	}

	inline void intensityToQImage(const frameset_t& fs, QImage& qImage, tinycolormap::ColormapType colorMap = tinycolormap::ColormapType::Gray, bool invert = false)
	{
		_internal::frameToQImage(fs.intensity, fs.width, fs.height, qImage, colorMap, invert);
	}

	inline void stateToQImage(const frameset_t& fs, QImage& qImage, tinycolormap::ColormapType colorMap = tinycolormap::ColormapType::Gray, bool invert = false)
	{
		_internal::frameToQImage(fs.state, fs.width, fs.height, qImage, colorMap, invert);
	}
}