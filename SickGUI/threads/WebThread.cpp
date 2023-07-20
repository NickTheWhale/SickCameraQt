#include "WebThread.h"
#include <qbuffer.h>
#include <qtimer.h>
#include <qrandom.h>
#include <AutoWebSocket.h>

bool WebThread::startWeb(AutoWebSocket* socket)
{
	if (isRunning())
		return false;
	this->socket = socket;
	//this->socket->start();
	start(QThread::Priority::NormalPriority);
	return true;
}

void WebThread::stopWeb()
{
	_stop = true;
}

void WebThread::run()
{
	uint32_t lastFsNumber = 0;
	QElapsedTimer cycleTimer;
	cycleTimer.start();
	while (!_stop)
	{
		QByteArray bytes;

		for (int i = 0; i < 100; ++i)
			bytes.append(i);

		qint64 bytesWritten = socket->socket()->sendBinaryMessage(bytes);

		if (bytesWritten > 0)
		{
			qDebug() << "wrote" << bytesWritten << "bytes";
		}

		qint64 sleepTime = minCycleTime - cycleTimer.elapsed();
		if (sleepTime > 0)
		{
			msleep(sleepTime);
		}

		qDebug() << "socket remaining time" << socket->remainingTime() << "timer" << socket->timer() << "isnull" << (bool)(socket->timer() == nullptr) << "isactive" << socket->timer()->isActive();;

		qint64 elapsed = cycleTimer.restart();
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
