#pragma once
#include <Frameset.h>
#include <boost/circular_buffer.hpp>
#include <qmutex.h>

class ThreadInterface
{
public:
	static ThreadInterface& instance();

	void pushPlcFrame(const frameset::Frameset& fs);
	void pushRawFrame(const frameset::Frameset& fs);

	const frameset::Frameset peekPlcFrame();
	const frameset::Frameset peekRawFrame();

	const frameset::Frameset popPlcFrame();
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

