#pragma once
#include "qimage.h"

namespace Frameset
{

	typedef struct
	{
		std::vector<uint16_t> depth;
		std::vector<uint16_t> intensity;
		std::vector<uint16_t> state;
		size_t height;
		size_t width;
		size_t number;
	} frameset_t;

	static bool depthToQImage(frameset_t fs, QImage& qImage)
	{
		qImage = QImage(fs.width, fs.height, QImage::Format_Grayscale16);
		for (auto y = 0; y < fs.height; ++y)
		{
			for (auto x = 0; x < fs.width; ++x)
			{
				auto depth = fs.depth[y * fs.width + x];
				qImage.setPixel(x, y, depth);
			}
		}
		return true;
	}

	static bool intensityToQImage(frameset_t fs, QImage& qImage)
	{
		qImage = QImage(fs.width, fs.height, QImage::Format_Grayscale16);
		for (auto y = 0; y < fs.height; ++y)
		{
			for (auto x = 0; x < fs.width; ++x)
			{
				auto depth = fs.intensity[y * fs.width + x];
				qImage.setPixel(x, y, depth);
			}
		}
		return true;
	}

	static bool stateToQImage(frameset_t fs, QImage& qImage)
	{
		qImage = QImage(fs.width, fs.height, QImage::Format_Grayscale16);
		for (auto y = 0; y < fs.height; ++y)
		{
			for (auto x = 0; x < fs.width; ++x)
			{
				auto depth = fs.state[y * fs.width + x];
				qImage.setPixel(x, y, depth);
			}
		}
		return true;
	}
}