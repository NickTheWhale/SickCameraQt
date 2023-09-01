/*****************************************************************//**
 * @file   ThreadInterface.h
 * @brief  Singleton that maintains mutex protected frameset::Frameset circular buffers for interthread communication.
 * 
 * @author Nicholas Loehrke
 * @date   August 2023
 *********************************************************************/

#pragma once
#include <Frameset.h>
#include <boost/circular_buffer.hpp>
#include <qmutex.h>

class ThreadInterface
{
public:
	static ThreadInterface& instance();

	/**
	 * @brief Pushes filtered frameset to internal buffer.
	 * 
	 * @param fs Frameset to push.
	 */
	void pushFilteredFrameset(const frameset::Frameset& fs);

	/**
	 * @brief Pushes raw frameset to internal buffer.
	 * 
	 * @param fs Frameset to push.
	 */
	void pushRawFrameset(const frameset::Frameset& fs);

	/**
	 * @brief Returns latest filtered frameset without removing it from the internal buffer.
	 * 
	 * @return Latest filtered frameset.
	 */
	const frameset::Frameset peekFilteredFrameset();

	/**
	 * @brief Returns latest raw frameset without removing it from the internal buffer.
	 * 
	 * @return Latest raw frameset.
	 */
	const frameset::Frameset peekRawFrameset();

	/**
	 * @brief Returns and removes latest filtered frameset from internal buffer.
	 * 
	 * @return Latest filtered frameset.
	 */
	const frameset::Frameset popFilteredFrameset();

	/**
	 * @brief Returns and removes latest raw frameset from internal buffer.
	 * 
	 * @return Latest raw frameset.
	 */
	const frameset::Frameset popRawFrame();
	
private:
	explicit ThreadInterface();
	~ThreadInterface();

	const int bufferSize = 2;
	boost::circular_buffer<frameset::Frameset> filteredBuffer;
	boost::circular_buffer<frameset::Frameset> rawBuffer;

	QMutex filteredMutex;
	QMutex rawMutex;
};

