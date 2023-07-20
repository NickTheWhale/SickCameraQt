#pragma once
#include "..\VisionaryFrameset.h"
#include <boost/circular_buffer.hpp>
#include <qmutex.h>

class BufferManager
{
public:
	static BufferManager& instance();

	void pushPlcFrame(const Frameset::frameset_t& fs);
	void pushGuiFrame(const Frameset::frameset_t& fs);
	void pushWebFrame(const Frameset::frameset_t& fs);

	const Frameset::frameset_t popPlcFrame();
	const Frameset::frameset_t popGuiFrame();
	const Frameset::frameset_t popWebFrame();
	
private:
	explicit BufferManager();
	~BufferManager();

	const int bufferSize = 1;
	boost::circular_buffer<Frameset::frameset_t> plcBuffer;
	boost::circular_buffer<Frameset::frameset_t> guiBuffer;
	boost::circular_buffer<Frameset::frameset_t> webBuffer;

	QMutex plcMutex;
	QMutex guiMutex;
	QMutex webMutex;
};

