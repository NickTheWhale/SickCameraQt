#pragma once
#include <vector>
#include "qimage.h"
#include <stdexcept>



class Frame
{
public:
	Frame() {};
	Frame(std::vector<uint16_t> data, size_t height, size_t width, QImage::Format format, uint32_t number)
		: data(data), height(height), width(width), format(format), number(number) {};
	virtual ~Frame() {};

	std::vector<uint16_t> getData() { return data; };
	size_t getHeight() { return height; };
	size_t getWidth() { return width; };
	QImage::Format getFormat() { return format; };

	void setData(std::vector<uint16_t> data) { this->data = data; };
	void setHeight(size_t height) { this->height = height; };
	void setWidth(size_t width) { this->width = width; };
	void setFormat(QImage::Format format) { this->format = format; };

	virtual bool toQImage(QImage& qImage) = 0;

private:
	std::vector<uint16_t> data;
	size_t height;
	size_t width;
	QImage::Format format;
	uint32_t number;
};



class DepthFrame : public Frame
{
public:
	DepthFrame() {};
	DepthFrame(std::vector<uint16_t> data, size_t height, size_t width, QImage::Format format, uint32_t number);
	~DepthFrame() override {};
	bool toQImage(QImage& qImage) override;
};


class ColorFrame : public Frame
{
public:
	ColorFrame() {};
	ColorFrame(std::vector<uint16_t> data, size_t height, size_t width, QImage::Format format, uint32_t number);
	~ColorFrame() override {};
	bool toQImage(QImage& qImage) override;
};



class StateFrame : public Frame
{
public:
	StateFrame() {};
	StateFrame(std::vector<uint16_t> data, size_t height, size_t width, QImage::Format format, uint32_t number);
	~StateFrame() override {};
	bool toQImage(QImage& qImage) override;
};