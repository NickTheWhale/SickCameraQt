#include <iostream>
#include <vector>
#include <string>
#include <format>

void parseBuffer(std::vector<uint8_t> buffer);
std::string readFlexString(const std::vector<uint8_t> buffer, size_t& pos);
std::string readNetAddress(const std::vector<uint8_t> buffer, size_t& pos);
std::string readMacAddress(const std::vector<uint8_t> buffer, size_t& pos);
std::string readFixString4(const std::vector<uint8_t> buffer, size_t& pos);
uint32_t readUDint(const std::vector<uint8_t> buffer, size_t& pos);

const std::vector<uint8_t> testBuffer = {
	0x00,0x24,0x9b,0x13,0x73,0x3f,0x00,0x06,0x77,0x16,0x0e,0x55,0x08,0x00,0x45,0x00,0x01,0x24,0xbd,0x22,0x40,0x00,0x40,0x11,0x2c,0xa4,0xa9,0xfe,0x31,0xa1,0xa9,0xfe,0xca,0x64,0x77,0xfe,0x77,0xff,0x01,0x10,0xa5,0xfd,0x95,0x00,0x00,0xf8,0x00,0x06,0x77,0x16,0x0e,0x55,0x83,0x37,0x50,0x69,0x10,0x00,0x01,0x00,0x00,0x1d,0x56,0x69,0x73,0x69,0x6f,0x6e,0x61,0x72,0x79,0x2d,0x54,0x20,0x4d,0x69,0x6e,0x69,0x20,0x43,0x58,0x20,0x56,0x33,0x53,0x31,0x30,0x35,0x2d,0x31,0x78,0x00,0x01,0x00,0x06,0x00,0x00,0x00,0x00,0x74,0xc3,0x03,0x00,0x00,0x00,0x00,0x0c,0x56,0x69,0x73,0x69,0x6f,0x6e,0x61,0x72,0x79,0x20,0x41,0x50,0x00,0x0b,0x6e,0x6f,0x74,0x20,0x64,0x65,0x66,0x69,0x6e,0x65,0x64,0x00,0x0c,0x56,0x69,0x73,0x69,0x6f,0x6e,0x61,0x72,0x79,0x20,0x41,0x50,0x00,0x08,0x32,0x31,0x34,0x36,0x30,0x30,0x37,0x35,0x00,0x07,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x00,0x0b,0x31,0x2e,0x33,0x2e,0x30,0x2e,0x32,0x39,0x38,0x39,0x31,0x00,0x07,0x31,0x31,0x31,0x32,0x36,0x34,0x39,0x1e,0x00,0x02,0x53,0x65,0x67,0x53,0x00,0x04,0x00,0x00,0x40,0x00,0x41,0x75,0x74,0x56,0x00,0x08,0x30,0x2e,0x31,0x2e,0x30,0x2e,0x30,0x52,0x00,0x01,0x02,0x02,0x00,0x08,0x4c,0x4f,0x4f,0x50,0x42,0x41,0x43,0x4b,0x00,0x05,0x45,0x49,0x50,0x61,0x00,0x04,0xa9,0xfe,0x31,0xa1,0x45,0x4e,0x4d,0x61,0x00,0x04,0xff,0xff,0x00,0x00,0x45,0x44,0x47,0x61,0x00,0x04,0x00,0x00,0x00,0x00,0x45,0x43,0x44,0x75,0x00,0x04,0x00,0x00,0x00,0x05,0x45,0x4d,0x41,0x43,0x00,0x06,0x00,0x06,0x77,0x16,0x0e,0x55,0x00,0x02,0x01,0x00,0x01,0x44,0x50,0x4e,0x6f,0x00,0x02,0x08,0x4a,0x03,0x00,0x01,0x44,0x50,0x4e,0x6f,0x00,0x02,0x08,0x3f
};

int main()
{
	parseBuffer(testBuffer);

	std::cin.get();
	return 0;
}

std::string readFlexString(const std::vector<uint8_t> buffer, size_t& pos)
{
	std::string flexString = "";
	uint16_t size = buffer[pos++] << 8 | buffer[pos++];
	for (size_t i = 0; i < size; ++i)
	{
		flexString.push_back(static_cast<uint8_t>(buffer[pos++]));
	}
	return flexString;
}

std::string readNetAddress(const std::vector<uint8_t> buffer, size_t& pos)
{
	std::string address = "";
	// we assume the length is 4
	address.append(std::to_string(buffer[pos++]));
	address.append(":");
	address.append(std::to_string(buffer[pos++]));
	address.append(":");
	address.append(std::to_string(buffer[pos++]));
	address.append(":");
	address.append(std::to_string(buffer[pos++]));
	return address;
}

std::string readMacAddress(const std::vector<uint8_t> buffer, size_t& pos)
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

std::string readFixString4(const std::vector<uint8_t> buffer, size_t& pos)
{
	std::string id = "";
	const auto size = 4;
	for (size_t i = 0; i < size; ++i)
	{
		id.push_back(static_cast<uint8_t>(buffer[pos++]));
	}
	return id;
}

uint32_t readUDint(std::vector<uint8_t> buffer, size_t& pos)
{
	return buffer[pos++] | buffer[pos++] << 8 | buffer[pos++] << 16 | buffer[pos++] << 24;
}

void parseBuffer(std::vector<uint8_t> buffer)
{
	struct DeviceInfo
	{
		std::string DeviceName;
		std::string MacAddress;
		std::string IpAddress;
		std::string SubNet;
		std::string Port;
		std::string to_string()
		{
			return std::format(
				"DeviceName: {}, "
				"MacAddress: {}, "
				"IpAddress: {}, "
				"SubNet: {}, "
				"Port: {}",
				DeviceName, MacAddress, IpAddress, SubNet, Port);
		}
	};

	struct Header {
		size_t dataSize;
		std::string macAddress;
		uint32_t id;
		uint8_t cs;
		std::string to_string()
		{
			return std::format(
				"dataSize: {}, "
				"macAddress: {}, "
				"id: {}, "
				"cs: {}",
				dataSize, macAddress, id, cs);
		}
	};


	Header header;
	size_t pos = 0;
	// this parses the "header" which contains the mac address of the device 
	//	and most importantly the size of the reply data
	while (true)
	{
		// find the start of the data
		if (buffer[pos++] != 0x95)
			continue;

		// ignore isBC
		pos += 1;

		// get data size
		auto msb = static_cast<size_t>(buffer[pos++]);
		auto lsb = static_cast<uint8_t>(buffer[pos++]);
		header.dataSize = (msb << 8) | lsb;

		// get mac address
		header.macAddress = readMacAddress(buffer, pos);

		// get telegram request id
		header.id = readUDint(buffer, pos);

		// idk
		header.cs = buffer[pos++];

		// ignore ffu
		pos += 1;

		break;
	}

	// parse device info
	DeviceInfo dev;
	dev.DeviceName = "";
	dev.IpAddress = "";
	dev.MacAddress = "";
	dev.Port = "";
	dev.SubNet = "";

	while (true)
	{
		// ignore DeviceInfoVersion
		pos += 2;

		// get CidName
		dev.DeviceName = readFlexString(buffer, pos);

		// get CidVersion
		struct CidVersion
		{
			uint16_t major;
			uint16_t minor;
			uint16_t patch;
			uint32_t buildNumber;
			uint8_t classifier;
		};
		CidVersion ver;
		ver.major = buffer[pos++] << 8 | buffer[pos++];
		ver.minor = buffer[pos++] << 8 | buffer[pos++];
		ver.patch = buffer[pos++] << 8 | buffer[pos++];
		ver.buildNumber = buffer[pos++] << 24 | buffer[pos++] << 16 | buffer[pos++] << 8 | buffer[pos++];
		ver.classifier = buffer[pos++];

		// ignore DeviceState
		pos += 1;

		// ignore RequiredUserAction
		pos += 2;

		// get DeviceName
		std::string deviceName = readFlexString(buffer, pos);

		// get ApplicationSpecificName
		std::string applicationSpecificName = readFlexString(buffer, pos);

		// ignore ProjectName
		std::string projectName = readFlexString(buffer, pos);

		// ignore SerialNumber
		std::string serialNumberSize = readFlexString(buffer, pos);

		// ignore TypeCode
		std::string typeCode = readFlexString(buffer, pos);

		// ignore FirmwareVersion
		std::string firmwareVersion = readFlexString(buffer, pos);

		// ignore OrderNumber
		std::string orderNumber = readFlexString(buffer, pos);

		// ignore Flags
		pos += 1;

		// get Aux Entries
		struct AuxEntry
		{
			std::string id;
			std::string value;
		};

		std::vector<AuxEntry> entries;

		uint16_t auxEntrySize = buffer[pos++] << 8 | buffer[pos++];
		for (size_t i = 0; i < auxEntrySize; ++i)
		{
			AuxEntry entry;
			entry.id = readFixString4(buffer, pos);
			entry.value = readFlexString(buffer, pos);
			entries.push_back(entry);
		}

		// get Scan IF
		struct ScanIF
		{
			uint16_t number;
			std::string value;
		};

		std::vector<ScanIF> scanInterfaces;

		uint16_t scanIFSize = buffer[pos++] << 8 | buffer[pos++];
		for (size_t i = 0; i < scanIFSize; ++i)
		{
			ScanIF scan;
			scan.number = buffer[pos++] << 8 | buffer[pos++];
			scan.value = readFlexString(buffer, pos);
			scanInterfaces.push_back(scan);
		}

		//General Com Settings
		uint16_t comSettingSize = buffer[pos++] << 8 | buffer[pos++];
		for (size_t i = 0; i < comSettingSize; ++i)
		{
			std::string key = readFixString4(buffer, pos);
			// IpAddress
			if (key == "EIPa")
			{
				pos += 2;
				dev.IpAddress = readNetAddress(buffer, pos);
			}
			// SubnetMask
			else if (key == "ENMa")
			{
				pos += 2;
				dev.SubNet = readNetAddress(buffer, pos);
			}
			//MacAddress
			else if (key == "EMAC")
			{
				dev.MacAddress = readMacAddress(buffer, pos);
			}
			// advance the buffer if we don't care about the value
			else
			{
				readFlexString(buffer, pos);
			}
		}

		break;
	}
	std::cout << dev.to_string() << std::endl;
}

/*
0000   00 24 9b 13 73 3f 00 06 77 16 0e 55 08 00 45 00
0010   01 24 bd 22 40 00 40 11 2c a4 a9 fe 31 a1 a9 fe
0020   ca 64 77 fe 77 ff 01 10 a5 fd 95 00 00 f8 00 06
0030   77 16 0e 55 83 37 50 69 10 00 01 00 00 1d 56 69
0040   73 69 6f 6e 61 72 79 2d 54 20 4d 69 6e 69 20 43
0050   58 20 56 33 53 31 30 35 2d 31 78 00 01 00 06 00
0060   00 00 00 74 c3 03 00 00 00 00 0c 56 69 73 69 6f
0070   6e 61 72 79 20 41 50 00 0b 6e 6f 74 20 64 65 66
0080   69 6e 65 64 00 0c 56 69 73 69 6f 6e 61 72 79 20
0090   41 50 00 08 32 31 34 36 30 30 37 35 00 07 31 32
00a0   33 34 35 36 37 00 0b 31 2e 33 2e 30 2e 32 39 38
00b0   39 31 00 07 31 31 31 32 36 34 39 1e 00 02 53 65
00c0   67 53 00 04 00 00 40 00 41 75 74 56 00 08 30 2e
00d0   31 2e 30 2e 30 52 00 01 02 02 00 08 4c 4f 4f 50
00e0   42 41 43 4b 00 05 45 49 50 61 00 04 a9 fe 31 a1
00f0   45 4e 4d 61 00 04 ff ff 00 00 45 44 47 61 00 04
0100   00 00 00 00 45 43 44 75 00 04 00 00 00 05 45 4d
0110   41 43 00 06 00 06 77 16 0e 55 00 02 01 00 01 44
0120   50 4e 6f 00 02 08 4a 03 00 01 44 50 4e 6f 00 02
0130   08 3f
*/