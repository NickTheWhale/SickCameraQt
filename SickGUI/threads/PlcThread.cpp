#include "PlcThread.h"
#include "qdebug.h"
#include <algorithm>
#include <snap7.h>
#include <qelapsedtimer.h>
#include <qrandom.h>
#include <ThreadInterface.h>
#include <qdebug.h>
#include <array>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>

bool PlcThread::startPlc(TS7Client* client)
{
	this->client = client;

	start(QThread::Priority::HighPriority);
	return true;
}
void PlcThread::stopPlc()
{
	_stop = true;
}

void PlcThread::run()
{
	ThreadInterface& threadInterface = ThreadInterface::instance();
	uint32_t lastNumber = 0;
	QElapsedTimer cycleTimer;
	cycleTimer.start();
	while (!_stop)
	{
		msleep(1);
		frameset::Frameset fs = threadInterface.peekPlcFrame();

		if (frameset::isValid(fs))
		{
			std::array<uint32_t, WRITE_BUFFER_SIZE> data;

			cv::Mat mat = frameset::toMat(fs.depth);
			cv::resize(mat, mat, cv::Size(WRITE_IMAGE_WIDTH, WRITE_IMAGE_HEIGHT), 0.0, 0.0, cv::InterpolationFlags::INTER_AREA);

			for (int y = 0; y < mat.rows; ++y)
			{
				for (int x = 0; x < mat.cols; ++x)
				{
					data[y * mat.cols + x] = mat.at<uint16_t>(cv::Point(x, y));
				}
			}

			if (!write(data))
			{
				if (client->Connect() != 0)
					qWarning() << "plc failed to reconnect";
				else
					qInfo() << "plc reconnected";
				msleep(100);
			}
		}

		const qint64 timeLeft = cycleTimeTarget - cycleTimer.elapsed();
		if (timeLeft > 0)
		{
			msleep(timeLeft);
		}
		qint64 time = cycleTimer.restart();
		emit addTime(static_cast<int>(time));
	}
}

//bool PlcThread::readDB2()
//{
//	byte buffer[4];
//	if (client->DBRead(2, 0, 4, &buffer) != 0)
//	{
//		qWarning() << "failed to read DB2";
//		return false;
//	}
//
//	for (const auto& _byte : buffer)
//	{
//		qDebug() << "byte:" << _byte;
//	}
//	return true;
//}

bool PlcThread::write(const std::array<uint32_t, WRITE_BUFFER_SIZE>& data)
{
	const int area = S7AreaDB;
	const int DBNumber = 2;
	const int start = 0;

	std::array<byte, WRITE_BUFFER_SIZE * 4> buffer;

	for (int i = 0; i < data.size(); ++i)
	{
		SetDWordAt(buffer.data(), i * 4, data[i]);
	}

	return client->DBWrite(DBNumber, start, WRITE_BUFFER_SIZE * 4, buffer.data()) == 0;
}

void PlcThread::setCycleTimeTarget(const qint64 cycleTime)
{
	QMutexLocker locker(&cycleTimeMutex);
	this->cycleTimeTarget = cycleTime;
}

const qint64 PlcThread::getCycleTimeTarget() const
{
	QMutexLocker locker(&cycleTimeMutex);
	return cycleTimeTarget;
}