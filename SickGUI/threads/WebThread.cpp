#include "WebThread.h"
#include <qbuffer.h>
#include <qtimer.h>
#include <qrandom.h>

bool WebThread::startWeb(AutoWebSocket* socket)
{
	this->socket = socket;
	this->socket->start();
	start(QThread::Priority::NormalPriority);
	return true;
}

void WebThread::stopWeb()
{
	_stop = true;
}

void WebThread::run()
{
	QElapsedTimer timer;
	timer.start();
	while (!_stop)
	{
		QByteArray bytes;

		for (int i = 0; i < 100; ++i)
			bytes.append(i);

		qint64 bytesWritten = socket->socket()->sendBinaryMessage(bytes);

		qDebug() << "wrote" << bytesWritten << "bytes";

		if (timer.elapsed() < 10)
		{
			msleep(10);
		}

		qint64 elapsed = timer.restart();
		emit addTime(elapsed);
	}
}

const QByteArray WebThread::buildImagePacket(const Frameset::frameset_t& fs)
{
	QImage image;

	Frameset::depthToQImage(fs, image);

	QByteArray bytes;

	// height
	for (int i = 0; i < 4; ++i)
	{
		bytes.append(static_cast<uint8_t>((image.height() >> 8 * i) & 0xff));
	}

	// width
	for (int i = 0; i < 4; ++i)
	{
		bytes.append(static_cast<uint8_t>((image.width() >> 8 * i) & 0xff));
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
	image.save(&buffer, "PNG", -1);

	// image size (in bytes)
	for (int i = 0; i < 8; ++i)
	{
		bytes.append(static_cast<uint8_t>((imageBytes.size() >> 8 * i) & 0xff));
	}

	bytes.append(imageBytes);
	return bytes;
}

void WebThread::newFrameset(const Frameset::frameset_t& fs)
{
	fsBuff = fs;
}
