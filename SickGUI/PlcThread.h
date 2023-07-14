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
#include "VisionaryFrameset.h"
#include <boost/circular_buffer.hpp>
#include <qmutex.h>
#include <qtimer.h>

class PlcThread : public QThread
{
	Q_OBJECT


public slots:
	/**
	 * @brief Slot to receive new frameset emitted by captureThread.
	 * 
	 * @param fs Output frameset.
	 */
	void newFrameset(Frameset::frameset_t fs);

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
	void uploadDB();
	volatile bool _stop = false;
	TS7Client* client;

	Frameset::frameset_t fsBuff;
	QMutex framesetMutex;
};

