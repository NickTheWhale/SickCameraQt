#include "WebThread.h"
#include <qbuffer.h>

bool WebThread::startWeb(AutoWebSocket* socket)
{
	this->socket = socket;
	start(QThread::Priority::NormalPriority);
	return true;
}

void WebThread::stopWeb()
{
	_stop = true;
}

void WebThread::run()
{
	while (!_stop)
	{
		// gotta use a mutex since the frameset buffer is accessed by multiple threads
		if (!lastImageMutex.tryLock())
		{
			continue;
		}
		if (lastImage.isNull())
		{
			lastImageMutex.unlock();
			continue;
		}
		else
		{
			QImage qImage = lastImage;
			lastImageMutex.unlock();
			QByteArray bytes;

			// packet start
			for (int i = 0; i < 4; ++i)
			{
				bytes.append(static_cast<char>(0));
				bytes.append(static_cast<char>(255));
			}

			// height
			for (int i = 0; i < 4; ++i)
			{
				bytes.append(static_cast<uint8_t>((qImage.height() >> 8 * i) & 0xff));
			}

			// width
			for (int i = 0; i < 4; ++i)
			{
				bytes.append(static_cast<uint8_t>((qImage.width() >> 8 * i) & 0xff));
			}

			// frame number
			for (int i = 0; i < 4; ++i)
			{
				bytes.append(static_cast<uint8_t>((1 >> 8 * i) & 0xff));
			}

			// frame time
			for (int i = 0; i < 8; ++i)
			{
				bytes.append(static_cast<uint8_t>((1 >> 8 * i) & 0xff));
			}

			// image data (as png)
			QByteArray imageBytes;
			QBuffer buffer(&imageBytes);
			buffer.open(QIODevice::WriteOnly);
			qImage.save(&buffer, "PNG", -1);

			// image size (in bytes)
			for (int i = 0; i < 8; ++i)
			{
				bytes.append(static_cast<uint8_t>((imageBytes.size() >> 8 * i) & 0xff));
			}

			bytes.append(imageBytes);

			qint64 bytesWritten = socket->socket()->sendBinaryMessage(bytes);

			qDebug() << "Wrote" << bytesWritten << "bytes";
		}
	}
}

void WebThread::newImage(QImage image)
{
	qDebug() << "Web Thread new image";
	if (!lastImageMutex.tryLock())
	{
		return;
	}

	lastImage = image;
	lastImageMutex.unlock();
}
