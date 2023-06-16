#pragma once
#include <vector>
#include <string>

namespace visionary
{
	static const uint16_t DEFAULT_PORT = 30718;
	static const std::string DEFAULT_BROADCAST_ADDR = "255.255.255.255";

	enum class Protocol : uint8_t
	{
		COLA_B = 0,
		COLA_2_0 = 1,
		COLA_2_1 = 2,
		COLA_A = 3,
		HTTP = 4,
		HTTPS = 5
	};

	struct Endpoint
	{
		Protocol protocol;
		std::string port;
	};

	class VisionaryAutoIPScanCustom
	{
	public:
		struct DeviceInfo
		{
			std::string DeviceName;
			std::string MacAddress;
			std::string IpAddress;
			std::string SubNet;
			std::string Port;
		};

		VisionaryAutoIPScanCustom();
		~VisionaryAutoIPScanCustom();

		std::vector<DeviceInfo> doScan(int timeOut, const std::string& broadcastAddress = visionary::DEFAULT_BROADCAST_ADDR, uint16_t port = visionary::DEFAULT_PORT);

	private:
		DeviceInfo parseRawBuffer(std::vector<uint8_t> buffer);

		std::string readFlexString(const std::vector<uint8_t> buffer, size_t& pos);
		std::string readFixString4(const std::vector<uint8_t> buffer, size_t& pos);
		std::string readNetAddress(const std::vector<uint8_t> buffer, size_t& pos);
		std::string readMacAddress(const std::vector<uint8_t> buffer, size_t& pos);

		uint32_t read_uint32_t(const std::vector<uint8_t> buffer, size_t& pos);
		uint16_t read_uint16_t(const std::vector<uint8_t> buffer, size_t& pos);
		uint8_t read_uint8_t(const std::vector<uint8_t> buffer, size_t& pos);

		struct ComKey_t
		{
			std::string IP_ADDRESS = "EIPa";
			std::string SUBNET_MASK = "ENMa";
			std::string DEFUALT_GATEWAY = "EDGa";
			std::string USE_DHCP = "EDhc";
			std::string MAC_ADDRESS = "EMAC";
			std::string IP_CONFIG_DURATION = "ECDu";
			std::string ZERO_CONF = "EZeC";
		};

		const ComKey_t ComKey;
		const std::string PORT_KEY = "DPNo";
	};

}