#pragma once
#include <Frameset.h>
#include <boost/circular_buffer.hpp>
#include <qmutex.h>

class ThreadInterface
{
public:
	static ThreadInterface& instance();

	void pushFilteredFrame(const frameset::Frameset& fs);
	void pushRawFrame(const frameset::Frameset& fs);

	const frameset::Frameset peekFilteredFrame();
	const frameset::Frameset peekRawFrame();

	const frameset::Frameset popFilteredFrame();
	const frameset::Frameset popRawFrame();
	
private:
	explicit ThreadInterface();
	~ThreadInterface();

	const int bufferSize = 2;
	boost::circular_buffer<frameset::Frameset> plcBuffer;
	boost::circular_buffer<frameset::Frameset> rawBuffer;

	QMutex plcMutex;
	QMutex rawMutex;
};

