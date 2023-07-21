#include "PlcThread.h"
#include "qdebug.h"
#include <algorithm>
#include <snap7.h>
#include "..\Fingerprint.h"
#include <qelapsedtimer.h>
#include <qrandom.h>
#include <BufferManager.h>

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
		Frameset::frameset_t fs = bufferManager.popPlcFrame();

		if (!fs.isNull())
		{
			if (client->Connected())
			{
#pragma region LOOP
				uint32_t fp = Fingerprint::calculateFingerprint(fs.width, fs.height, fs.depth);
				writeDB2(fp);
#pragma endregion
			}
			else
			{
				qWarning() << "retrying plc connection...";
				bool success = !client->Connect();
				if (success)
					qInfo() << "plc reconnected";
				else
					qWarning() << "failed to reconnect plc";
			}
		}

		const qint64 timeLeft = minimumTargetCycleTime - cycleTimer.elapsed();
		if (timeLeft > 0)
		{
			msleep(timeLeft);
		}

		qint64 time = cycleTimer.restart();
		emit addTime(static_cast<int>(time));
	}
}

void PlcThread::readDB2()
{
	byte buffer[4];
	if (client->DBRead(2, 0, 4, &buffer) != 0)
	{
		qWarning() << "failed to read DB2";
		return;
	}

	for (const auto& _byte : buffer)
	{
		qDebug() << "byte:" << _byte;
	}
}

void PlcThread::writeDB2(const uint32_t& data)
{
	const int area = S7AreaDB;
	const int DBNumber = 3;
	const int start = 0;
	const int amount = 1;
	const int wordlen = S7WLWord;

	byte buffer[4];
	SetDWordAt(buffer, 0, data);

	int ret = client->DBWrite(DBNumber, start, 4, buffer);
	if (ret != 0)
	{
		qWarning() << "failed to write DB2: " << CliErrorText(ret);
	}
}