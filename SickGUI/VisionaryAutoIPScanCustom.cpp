#include "VisionaryAutoIPScanCustom.h"

#include <string>
#include <random>
#include <chrono>
#include <algorithm>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <UdpSocket.h>

namespace visionary
{

	VisionaryAutoIPScanCustom::VisionaryAutoIPScanCustom()
	{
	}

	VisionaryAutoIPScanCustom::~VisionaryAutoIPScanCustom()
	{
	}

	std::vector<VisionaryAutoIPScanCustom::DeviceInfo> VisionaryAutoIPScanCustom::doScan(int timeOut, const std::string& broadcastAddress, uint16_t port)
	{
		// Init Random generator
		std::random_device rd;
		std::default_random_engine mt(rd());
		unsigned int teleIdCounter = mt();
		std::vector<VisionaryAutoIPScanCustom::DeviceInfo> deviceList;

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
		// MAC address
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
		//autoIpPacket.push_back(0x01);
		//autoIpPacket.push_back(0x02);
		//autoIpPacket.push_back(0xa9);
		//autoIpPacket.push_back(0xfe);
		//autoIpPacket.push_back(0xca);
		//autoIpPacket.push_back(0x64);
		//autoIpPacket.push_back(0xff);
		//autoIpPacket.push_back(0xff);
		// 
		// page 76 8.2.2 Commands
		autoIpPacket.push_back(0xff);
		autoIpPacket.push_back(0xff);
		autoIpPacket.push_back(0xff);
		autoIpPacket.push_back(0xff);
		autoIpPacket.push_back(0xff);
		autoIpPacket.push_back(0xff);
		autoIpPacket.push_back(0xff);
		autoIpPacket.push_back(0xff);
		// 
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
				if (receiveBuffer[pos++] != 0x95) //0x95 = answer package id and 16 bytes minsize
				{
					continue;
				}
				pos += 1; // unused byte
				unsigned int payLoadSize = receiveBuffer[pos] << 8 | receiveBuffer[pos + 1];
				pos += 2;
				pos += 6; //Skip mac address
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
					DeviceInfo deviceInfo = parseRawBuffer(receiveBuffer);
					deviceList.push_back(deviceInfo);
				}
				catch (...)
				{

				}
			}
		}
		return deviceList;
	}

	VisionaryAutoIPScanCustom::DeviceInfo VisionaryAutoIPScanCustom::parseRawBuffer(std::vector<uint8_t> buffer)
	{
		DeviceInfo device = { "", "", "", "", "" };

		size_t pos = 0;
		// find start of data indicated by 0x95
		while (true)
		{
			if (read_uint8_t(buffer, pos) != 0x95)
				continue;
			break;
		}
		// ignore isBC
		pos += 1;

		// ignore length
		pos += 2;

		// ignore MAC Addr
		pos += 6;

		// ignore RequestID
		pos += 4;

		// ignore CS
		pos += 1;

		// ignore ffu
		pos += 1;

		// ignore DeviceInfoVersion (uint16)
		pos += 2;

		// read CidName
		device.DeviceName = readFlexString(buffer, pos);

		// ignore CidVersion (11 byte struct)
		pos += 11;

		// ignore DeviceState (enum8)
		pos += 1;

		// ignore RequiredUserAction (2 byte cont)
		pos += 2;

		// ignore DeviceName 
		readFlexString(buffer, pos);

		// ignore ApplicationSpecificName
		readFlexString(buffer, pos);

		// ignore ProjectName
		readFlexString(buffer, pos);

		// ignore SerialNumber
		readFlexString(buffer, pos);

		// ignore TypeCode
		readFlexString(buffer, pos);

		// ignore FirmwareVersion
		readFlexString(buffer, pos);

		// ignore OrderNumber
		readFlexString(buffer, pos);

		// ignore Flags (1 byte scont)
		pos += 1;

		// ignore Aux Entries (flexarray128{ charkey[4], flexarray32 })
		auto numEntries = read_uint16_t(buffer, pos);
		for (auto i = 0; i < numEntries; ++i)
		{
			readFixString4(buffer, pos);
			readFlexString(buffer, pos);
		}

		// ignore Scan IF (flexarray32{ uint, flexstring64 })
		auto numScanIF = read_uint16_t(buffer, pos);
		for (auto i = 0; i < numScanIF; ++i)
		{
			read_uint16_t(buffer, pos);
			readFlexString(buffer, pos);
		}

		// read General Com Settings
		auto numComSettings = read_uint16_t(buffer, pos);
		for (auto i = 0; i < numComSettings; ++i)
		{
			std::string key = readFixString4(buffer, pos);
			if (key == ComKey.IP_ADDRESS)
			{
				pos += 2;
				device.IpAddress = readNetAddress(buffer, pos);
			}
			else if (key == ComKey.SUBNET_MASK)
			{
				pos += 2;
				device.SubNet = readNetAddress(buffer, pos);
			}
			else if (key == ComKey.MAC_ADDRESS)
			{
				pos += 2;
				device.MacAddress = readMacAddress(buffer, pos);
			}
			else
			{
				readFlexString(buffer, pos);
			}
		}

		// read Endpoints
		std::vector<Endpoint> endpoints;
		auto numEndpoints = read_uint16_t(buffer, pos);
		for (auto i = 0; i < numEndpoints; ++i)
		{
			Endpoint endpoint;
			endpoint.port = "";
			endpoint.protocol = static_cast<Protocol>(read_uint8_t(buffer, pos));
			auto arrSize = read_uint16_t(buffer, pos);
			for (auto j = 0; j < arrSize; ++j)
			{
				std::string key = readFixString4(buffer, pos);
				if (key == PORT_KEY)
				{
					pos += 2;
					endpoint.port = std::to_string(read_uint16_t(buffer, pos));
				}
				else
				{
					readFlexString(buffer, pos);
				}
			}
			endpoints.push_back(endpoint);
		}

		// assign port to device
		if (!endpoints.empty())
		{
			device.Port = endpoints[0].port;
		}



		auto _ = 0;
		return device;
	}

	std::string VisionaryAutoIPScanCustom::readFlexString(const std::vector<uint8_t> buffer, size_t& pos)
	{
		std::string flexString = "";
		uint16_t size = read_uint16_t(buffer, pos);
		for (size_t i = 0; i < size; ++i)
		{
			flexString.push_back(static_cast<uint8_t>(buffer[pos++]));
		}
		return flexString;
	}

	std::string VisionaryAutoIPScanCustom::readNetAddress(const std::vector<uint8_t> buffer, size_t& pos)
	{
		std::string address = "";
		// we assume the length is 4
		address.append(std::to_string(buffer[pos++]));
		address.append(".");
		address.append(std::to_string(buffer[pos++]));
		address.append(".");
		address.append(std::to_string(buffer[pos++]));
		address.append(".");
		address.append(std::to_string(buffer[pos++]));
		return address;
	}

	std::string VisionaryAutoIPScanCustom::readMacAddress(const std::vector<uint8_t> buffer, size_t& pos)
	{
		std::string macAddress = "";
		// we assume the length is 6
		macAddress.append(std::format("{:02X}", buffer[pos++]));
		macAddress.append(":");
		macAddress.append(std::format("{:02X}", buffer[pos++]));
		macAddress.append(":");
		macAddress.append(std::format("{:02X}", buffer[pos++]));
		macAddress.append(":");
		macAddress.append(std::format("{:02X}", buffer[pos++]));
		macAddress.append(":");
		macAddress.append(std::format("{:02X}", buffer[pos++]));
		macAddress.append(":");
		macAddress.append(std::format("{:02X}", buffer[pos++]));
		return macAddress;
	}

	std::string VisionaryAutoIPScanCustom::readFixString4(const std::vector<uint8_t> buffer, size_t& pos)
	{
		std::string id = "";
		const auto size = 4;
		for (size_t i = 0; i < size; ++i)
		{
			id.push_back(static_cast<uint8_t>(buffer[pos++]));
		}
		return id;
	}

	uint32_t VisionaryAutoIPScanCustom::read_uint32_t(std::vector<uint8_t> buffer, size_t& pos)
	{
		return buffer[pos++] << 24 | buffer[pos++] << 16 | buffer[pos++] << 8 | buffer[pos++];
	}

	uint16_t VisionaryAutoIPScanCustom::read_uint16_t(std::vector<uint8_t> buffer, size_t& pos)
	{
		return buffer[pos++] << 8 | buffer[pos++];
	}

	uint8_t VisionaryAutoIPScanCustom::read_uint8_t(std::vector<uint8_t> buffer, size_t& pos)
	{
		return buffer[pos++];
	}

	// this is not a good implementation
	/*
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
	*/

}