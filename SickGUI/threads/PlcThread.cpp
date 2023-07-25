#include "PlcThread.h"
#include "qdebug.h"
#include <algorithm>
#include <snap7.h>
#include "..\Fingerprint.h"
#include <qelapsedtimer.h>
#include <qrandom.h>
#include <BufferManager.h>
#include <qdebug.h>
#include <array>

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
	BufferManager& bufferManager = BufferManager::instance();
	uint32_t lastNumber = 0;
	QElapsedTimer cycleTimer;
	cycleTimer.start();
	while (!_stop)
	{
		msleep(1);
		Frameset::frameset_t fs = bufferManager.peekPlcFrame();

		if (!fs.isNull())
		{
			std::array<uint32_t, WRITE_BUFFER_SIZE> data;
			int offset = 0;
			data[offset++] = Fingerprint::calculateFingerprint(fs.width, fs.height, fs.depth);
			data[offset++] = fs.number;
			data[offset++] = fs.width;
			data[offset++] = fs.height;
			data[offset++] = fs.time;
			data[offset++] = fs.isNull();
			static uint32_t loopCount = 0;
			data[offset++] = ++loopCount;
			data[offset++] = QRandomGenerator::global()->bounded(0, (qint64)std::numeric_limits<qint64>::max());
			data[offset++] = QRandomGenerator::global()->bounded(0, (qint64)std::numeric_limits<qint64>::max());
			data[offset++] = QRandomGenerator::global()->bounded(0, (qint64)std::numeric_limits<qint64>::max());

			if (!writeDB2(data))
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

bool PlcThread::readDB2()
{
	byte buffer[4];
	if (client->DBRead(2, 0, 4, &buffer) != 0)
	{
		qWarning() << "failed to read DB2";
		return false;
	}

	for (const auto& _byte : buffer)
	{
		qDebug() << "byte:" << _byte;
	}
	return true;
}

bool PlcThread::writeDB2(const std::array<uint32_t, WRITE_BUFFER_SIZE>& data)
{
	const int area = S7AreaDB;
	const int DBNumber = 3;
	const int start = 0;

	std::array<byte, WRITE_BUFFER_SIZE * 4> buffer;

	for (int i = 0; i < data.size(); ++i)
	{
		SetDWordAt(buffer.data(), i * 4, data[i]);
	}

	int ret = client->DBWrite(DBNumber, start, WRITE_BUFFER_SIZE * 4, buffer.data());
	if (ret != 0)
	{
		qWarning() << "failed to write DB2: " << CliErrorText(ret);
		return false;
	}
	return true;
}

void PlcThread::setCycleTimeTarget(const qint64 cycleTime)
{
	this->cycleTimeTarget = cycleTime;
}

const qint64 PlcThread::getCycleTimeTarget() const
{
	return cycleTimeTarget;
}