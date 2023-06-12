#include "VisionaryFrame.h"

DepthFrame::DepthFrame(std::vector<uint16_t> data, size_t height, size_t width, QImage::Format format, uint32_t number)
	: Frame(data, height, width, format, number)
{
}

bool DepthFrame::toQImage(QImage& qImage) {
	auto width = getWidth();
	auto height = getHeight();

	qImage = QImage(width, height, getFormat());
	for (auto y = 0; y < height; ++y)
	{
		for (auto x = 0; x < width; ++x)
		{
			auto depth = getData()[y * width + x];
			qImage.setPixel(x, y, depth);
		}
	}
	return true;
}

ColorFrame::ColorFrame(std::vector<uint16_t> data, size_t height, size_t width, QImage::Format format, uint32_t number)
	: Frame(data, height, width, format, number)
{
}

bool ColorFrame::toQImage(QImage& qImage) {
	return false;
}

StateFrame::StateFrame(std::vector<uint16_t> data, size_t height, size_t width, QImage::Format format, uint32_t number)
	: Frame(data, height, width, format, number)
{
}

bool StateFrame::toQImage(QImage& qImage) {
	return false;
}
