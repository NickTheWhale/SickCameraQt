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

class PlcThread : public QThread
{
	Q_OBJECT

signals:
	void addTime(const int time);

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

	void setCycleTimeTarget(const qint64 cycleTime);
	const qint64 getCycleTimeTarget() const;

protected:
	void run() override;

private:
	volatile bool _stop = false;
	qint64 cycleTimeTarget = 10;

	TS7Client* client;
	
	bool readDB2();
#define WRITE_BUFFER_SIZE (10)
	bool writeDB2(const std::array<uint32_t, WRITE_BUFFER_SIZE>& data);
};

