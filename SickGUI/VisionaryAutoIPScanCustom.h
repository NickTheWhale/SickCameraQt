#pragma once
#include "VisionaryAutoIPScan.h"

using namespace visionary;

class VisionaryAutoIPScanCustom : public visionary::VisionaryAutoIPScan
{
public:
	VisionaryAutoIPScanCustom();
	~VisionaryAutoIPScanCustom();
	std::vector<DeviceInfo> doScan(int timeOut, const std::string& broadcastAddress = visionary::DEFAULT_BROADCAST_ADDR, uint16_t port = visionary::DEFAULT_PORT);

private:
	DeviceInfo parseRawBuffer(std::vector<uint8_t> buffer);
};

