#include "VisionaryAutoIPScanCustom.h"

#include <string>
#include <random>
#include <chrono>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <UdpSocket.h>


VisionaryAutoIPScanCustom::VisionaryAutoIPScanCustom()
{
}

VisionaryAutoIPScanCustom::~VisionaryAutoIPScanCustom()
{
}

std::vector<visionary::VisionaryAutoIPScan::DeviceInfo> VisionaryAutoIPScanCustom::doScan(int timeOut, const std::string& broadcastAddress, uint16_t port)
{
	// Init Random generator
	std::random_device rd;
	std::default_random_engine mt(rd());
	unsigned int teleIdCounter = mt();
	std::vector<VisionaryAutoIPScan::DeviceInfo> deviceList;

	std::unique_ptr<UdpSocket> pTransport(new UdpSocket());

	if (pTransport->connect(broadcastAddress, htons(port)) != 0)
	{
		return deviceList;
	}

	// AutoIP Discover Packet
	std::vector<uint8_t> autoIpPacket;

	autoIpPacket.push_back(0x10); //CMD
	autoIpPacket.push_back(0x00); //reserved
	// length of datablock
	autoIpPacket.push_back(0x00);
	autoIpPacket.push_back(0x08);
	//MAC address
	autoIpPacket.push_back(0xff);
	autoIpPacket.push_back(0xff);
	autoIpPacket.push_back(0xff);
	autoIpPacket.push_back(0xff);
	autoIpPacket.push_back(0xff);
	autoIpPacket.push_back(0xff);
	// telegram id
	autoIpPacket.push_back(0x00);
	autoIpPacket.push_back(0x00);
	autoIpPacket.push_back(0x00);
	autoIpPacket.push_back(0x00);
	// data pulled from wire shark
	autoIpPacket.push_back(0x01);
	autoIpPacket.push_back(0x02);
	autoIpPacket.push_back(0xa9);
	autoIpPacket.push_back(0xfe);
	autoIpPacket.push_back(0xca);
	autoIpPacket.push_back(0x64);
	autoIpPacket.push_back(0xff);
	autoIpPacket.push_back(0xff);
	// reserved
	autoIpPacket.push_back(0x00);
	autoIpPacket.push_back(0x00);

	// Replace telegram id in packet
	unsigned int curTelegramID = teleIdCounter++;
	memcpy(&autoIpPacket.data()[10], &curTelegramID, 4u);

	// Send Packet
	pTransport->send(autoIpPacket);

	// Check for answers to Discover Packet
	const std::chrono::steady_clock::time_point startTime(std::chrono::steady_clock::now());
	while (true)
	{
		std::vector<std::uint8_t> receiveBuffer;
		const std::chrono::steady_clock::time_point now(std::chrono::steady_clock::now());
		if ((now - startTime) > std::chrono::milliseconds(timeOut))
		{
			break;
		}
		if (pTransport->recv(receiveBuffer, 1400) > 16) // 16 bytes minsize
		{
			unsigned int pos = 0;
			if (receiveBuffer[pos++] != 0x95) //0x90 = answer package id and 16 bytes minsize
			{
				continue;
			}
			pos += 1; // unused byte
			unsigned int payLoadSize = receiveBuffer[pos] << 8 | receiveBuffer[pos + 1];
			pos += 2;
			pos += 6; //Skip mac address(part of xml)
			unsigned int recvTelegramID = receiveBuffer[pos] | receiveBuffer[pos + 1] << 8 | receiveBuffer[pos + 2] << 16 | receiveBuffer[pos + 3] << 24;
			pos += 4;
			// check if it is a response to our scan
			if (recvTelegramID != curTelegramID)
			{
				continue;
			}
			pos += 2; // unused
			
			// parse buffer
			try
			{
				DeviceInfo dI = parseRawBuffer(receiveBuffer);
				deviceList.push_back(dI);
			}
			catch (...)
			{

			}
		}
	}
	return deviceList;
}

// this is not a good implementation
VisionaryAutoIPScan::DeviceInfo VisionaryAutoIPScanCustom::parseRawBuffer(std::vector<uint8_t> buffer)
{
	DeviceInfo device;
	device.DeviceName = "";
	device.IpAddress = "";
	device.MacAddress = "";
	device.Port = "";
	device.SubNet = "";

	uint8_t byte1;
	uint8_t byte2;
	uint8_t byte3;
	uint8_t byte4;

	int i = 0;
	while (i < buffer.size())
	{
		if (i < buffer.size() - 8)
		{
			// get four bytes
			byte1 = buffer[i++];
			byte2 = buffer[i++];
			byte3 = buffer[i++];
			byte4 = buffer[i++];

			// find name
			if (byte1 == 0x01 && byte2 == 0x00 && byte3 == 0x00 && byte4 == 0x1d)
			{
				device.DeviceName = "";
				while (buffer[i] != 0x00 && i < buffer.size())
				{
					device.DeviceName.assign(std::to_string(buffer[i++]));
				}
			}

			// find ip address
			if (byte1 == 0x50 && byte2 == 0x61 && byte3 == 0x00 && byte4 == 0x04)
			{
				device.IpAddress = "";
				device.IpAddress.append(std::to_string(buffer[i++]));
				device.IpAddress.append(".");
				device.IpAddress.append(std::to_string(buffer[i++]));
				device.IpAddress.append(".");
				device.IpAddress.append(std::to_string(buffer[i++]));
				device.IpAddress.append(".");
				device.IpAddress.append(std::to_string(buffer[i++]));
			}
		}
		++i;
	}

	return device;
}