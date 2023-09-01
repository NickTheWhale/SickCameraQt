/*****************************************************************//**
 * @file   PlcThread.cpp
 * @brief  QThread subclass used to communicate with a Siemens Step7 PLC.
 * 
 * @author Nicholas Loehrke
 * @date   September 2023
 *********************************************************************/

#include "PlcThread.h"

#include <snap7.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <ThreadInterface.h>

#include <qelapsedtimer.h>
#include <qrandom.h>
#include <qsettings.h>
#include <qdebug.h>

#include <algorithm>
#include <array>

#include <global.h>

bool PlcThread::startPlc(TS7Client* client)
{
	this->client = client;

	// internally calls 'run()'
	start(QThread::Priority::HighPriority);
	return true;
}
void PlcThread::stopPlc()
{
	_stop = true;
}

void PlcThread::run()
{
	// get latest configuration
	loadConfiguration();
	ThreadInterface& threadInterface = ThreadInterface::instance();
	uint32_t lastNumber = 0;
	QElapsedTimer cycleTimer;
	cycleTimer.start();
	while (!_stop)
	{
		// prevent tight loop
		msleep(1);
		frameset::Frameset fs = threadInterface.peekFilteredFrameset();

		// check frameset validity and make sure its newer than the previous
		if (!frameset::isValid(fs) || fs.depth.number <= lastNumber)
			continue;

		lastNumber = fs.depth.number;

		// convert frameset to cv::Mat for resizing
		cv::Mat mat = frameset::toMat(fs.depth);

		// resize mat to configuration specified dimension
		cv::resize(mat, mat, cv::Size(imageWidth, imageHeight), 0.0, 0.0, cv::InterpolationFlags::INTER_AREA);

		// prepare write buffer
		std::vector<uint32_t> data;
		data.resize(static_cast<size_t>(mat.rows) * static_cast<size_t>(mat.cols));

		// fill write buffer
		for (int y = 0; y < mat.rows; ++y)
		{
			for (int x = 0; x < mat.cols; ++x)
			{
				data[y * mat.cols + x] = mat.at<uint16_t>(cv::Point(x, y));
			}
		}

		// add the frame number to front of write data buffer
		data.insert(data.begin(), fs.depth.number);

		int ret = write(data);
		
		// if write fails, emit disconnected() signal and try to reconnected plc
		if (ret != 0)
		{
			emit disconnected();
			auto error = CliErrorText(ret);
			qWarning() << "plc thread: write error:" << error;
			if (client->Connect() != 0)
			{
				qWarning() << "plc failed to reconnect";
			}
			else
			{
				emit reconnected();
			}
			msleep(100);
		}
		// otherwise say that we're still connected
		else
		{
			emit reconnected();
		}

		// calculate remaining loop time
		const qint64 timeLeft = cycleTimeTarget - cycleTimer.elapsed();
		if (timeLeft > 0)
		{
			msleep(timeLeft);
		}
		qint64 time = cycleTimer.restart();

		// emit time for cycle time widget
		emit addTime(static_cast<int>(time));
	}
}

int PlcThread::write(const std::vector<uint32_t>& data)
{
	const int area = S7AreaDB;

	// prepare byte buffer
	std::vector<byte> buffer;
	buffer.resize(data.size() * sizeof(uint32_t));

	// fill byte buffer from uint32_t (UDint) buffer
	for (size_t i = 0; i < data.size(); ++i)
	{
		SetDWordAt(buffer.data(), i * sizeof(uint32_t), data[i]);
	}

	// write the buffer
	return client->DBWrite(dbNumber, dbStart, buffer.size(), buffer.data());
}

void PlcThread::loadConfiguration()
{
	const auto ConfigPath = global::CONFIG_FILE_RELATIVE_PATH;
	QSettings settings(ConfigPath, QSettings::Format::IniFormat);

	cycleTimeTarget = settings.value("plc/cycle_time_target_ms", cycleTimeTarget).value<qint64>();
	dbNumber = settings.value("plc/db_number", dbNumber).value<qint32>();
	dbStart = settings.value("plc/db_start", dbStart).value<qint32>();
	imageWidth = settings.value("plc/image_width", imageWidth).value<qint32>();
	imageHeight = settings.value("plc/image_height", imageHeight).value<qint32>();
}