/*****************************************************************//**
 * @file   PlcThread.h
 * @brief  QThread subclass used to communicate with a Siemens Step7 PLC
 * 
 * @author Nicholas Loehrke
 * @date   June 2023
 *********************************************************************/

#pragma once
#include <qthread.h>
#include <snap7.h>
#include <Frameset.h>
#include <boost/circular_buffer.hpp>
#include <qtimer.h>
#include <qmutex.h>

class PlcThread : public QThread
{
	Q_OBJECT

signals:
	void addTime(const int time);
	void disconnected();
	void reconnected();

public:
	/**
	 * @brief Starts the thread.
	 * 
	 * @param client Snap7 client
	 * @note  PlcThread does not take ownership of the client. You must
	 *        free the resources once the thread has stopped.
	 * @return 
	 */
	bool startPlc(TS7Client* client);

	/**
	 * @brief Stop the thread.
	 * 
	 */
	void stopPlc();

protected:
	void run() override;

private:
	volatile bool _stop = false;

	qint64 cycleTimeTarget = 10;
	qint32 dbNumber = 0;
	qint32 dbStart = 0;
	qint32 imageWidth = 1;
	qint32 imageHeight = 1;

	TS7Client* client;
	
	int write(const std::vector<uint32_t>& data);
	void loadConfiguration();
};

