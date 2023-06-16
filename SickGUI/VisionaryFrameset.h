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

	inline void depthToQImage(frameset_t fs, QImage& qImage, tinycolormap::ColormapType colorMap = tinycolormap::ColormapType::Gray, bool invert = false)
	{
		qImage = QImage(fs.width, fs.height, QImage::Format_ARGB32_Premultiplied);
		auto max = *std::max_element(fs.depth.begin(), fs.depth.end());
		auto min = *std::min_element(fs.depth.begin(), fs.depth.end());

		QRgb* qImageData = reinterpret_cast<QRgb*>(qImage.bits());
		for (auto y = 0; y < fs.height; ++y)
		{
			for (auto x = 0; x < fs.width; ++x)
			{
				auto depthVal = fs.depth[y * fs.width + x];
				double depthNorm = (depthVal - min) / static_cast<double>(max);
				tinycolormap::Color color = tinycolormap::GetColor(depthNorm, colorMap);
				if (!invert)
					qImageData[y * fs.width + x] = qRgba(color.ri(), color.gi(), color.bi(), 255);
				else
					qImageData[y * fs.width + x] = qRgba(255 - color.ri(), 255 - color.gi(), 255 - color.bi(), 255);
			}
		}
	}

	inline void intensityToQImage(frameset_t fs, QImage& qImage, tinycolormap::ColormapType colorMap = tinycolormap::ColormapType::Gray, bool invert = false)
	{
		qImage = QImage(fs.width, fs.height, QImage::Format_ARGB32_Premultiplied);
		auto max = *std::max_element(fs.intensity.begin(), fs.intensity.end());
		auto min = *std::min_element(fs.intensity.begin(), fs.intensity.end());
		QRgb* qImageData = reinterpret_cast<QRgb*>(qImage.bits());
		for (auto y = 0; y < fs.height; ++y)
		{
			for (auto x = 0; x < fs.width; ++x)
			{
				auto intensityVal = fs.intensity[y * fs.width + x];
				double intensityNorm = (intensityVal - min) / static_cast<double>(max);
				tinycolormap::Color color = tinycolormap::GetColor(intensityNorm, colorMap);
				if (!invert)
					qImageData[y * fs.width + x] = qRgba(color.ri(), color.gi(), color.bi(), 255);
				else 
					qImageData[y * fs.width + x] = qRgba(255 - color.ri(), 255 - color.gi(), 255 - color.bi(), 255);
			}
		}
	}

	inline void stateToQImage(frameset_t fs, QImage& qImage, tinycolormap::ColormapType colorMap = tinycolormap::ColormapType::Gray, bool invert = false)
	{
		qImage = QImage(fs.width, fs.height, QImage::Format_ARGB32_Premultiplied);
		auto max = *std::max_element(fs.state.begin(), fs.state.end());
		auto min = *std::min_element(fs.state.begin(), fs.state.end());
		QRgb* qImageData = reinterpret_cast<QRgb*>(qImage.bits());
		for (auto y = 0; y < fs.height; ++y)
		{
			for (auto x = 0; x < fs.width; ++x)
			{
				auto stateVal = fs.state[y * fs.width + x];
				double stateNorm = (stateVal - min) / static_cast<double>(max);
				tinycolormap::Color color = tinycolormap::GetColor(stateNorm, colorMap);
				if (!invert)
					qImageData[y * fs.width + x] = qRgba(color.ri(), color.gi(), color.bi(), 255);
				else
					qImageData[y * fs.width + x] = qRgba(255 - color.ri(), 255 - color.gi(), 255 - color.bi(), 255);
			}
		}
	}
}