#pragma once
#include <vector>
#include "qimage.h"
#include <stdexcept>



class Frame
{
public:
	Frame() {};
	virtual ~Frame() {};
	virtual std::vector<uint16_t> getData() const = 0;
	virtual size_t getHeight() const = 0;
	virtual size_t getWidth() const = 0;
	virtual QImage::Format getFormat() const = 0;
	virtual bool toQImage(QImage& qImage) const = 0;
};

class DepthFrame : Frame
{

};

class Frameset
{
public:
	Frameset();
	~Frameset();
	Frame getDepth() const;
	Frame getColor() const;
	Frame getState() const;

private:
	Frame depth;
	Frame color;
	Frame state;
};



namespace Frame
{
	static struct frame_t
	{
		std::vector<uint16_t> data;
		int width;
		int height;
		QImage::Format format;
	};

	static struct frameSet_t
	{
		frame_t depth;
		frame_t color;
	};

	static bool toQImage(frame_t image, QImage& qImage)
	{
		bool ret = false;
		switch (image.format)
		{
		case QImage::Format_Grayscale16:
		{
			qImage = QImage(image.width, image.height, QImage::Format_Grayscale16);
			for (int y = 0; y < image.height; ++y)
			{
				for (int x = 0; x < image.width; ++x)
				{
					uint16_t depth = image.data[static_cast<std::vector<uint16_t, std::allocator<uint16_t>>::size_type>(y) * image.width + x];
					depth >>= 4;
					qImage.setPixel(x, y, depth);
				}
			}
			ret = true;
			break;
		}
		default:
		{
			ret = false;
		}
		}
		return ret;
	}

}
