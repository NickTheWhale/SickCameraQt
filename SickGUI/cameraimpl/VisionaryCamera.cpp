#include "VisionaryCamera.h"
#include "../VisionaryAutoIPScanCustom.h"
#include "../VisionaryFrameset.h"
#include <CoLaParameterWriter.h>
#include <CoLaCommand.h>
#include <CoLaCommandType.h>
#include <qdebug.h>
#include <vector>


// ping test
#include <WinSock2.h>
#include <iphlpapi.h>
#include <IcmpAPI.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

using namespace visionary;

VisionaryCamera::VisionaryCamera(std::string ipAddress, short dataPort)
{
	this->ipAddress = ipAddress;
	this->dataPort = dataPort;

	parameters.insert_or_assign("ipAddress", this->ipAddress);
	parameters.insert_or_assign("dataPort", std::to_string(this->dataPort));

	// create camera handlers
	pDataHandler = std::make_shared<visionary::VisionaryTMiniData>();
	pDataStream = std::make_shared<VisionaryDataStream>(pDataHandler);
	pVisionaryControl = std::make_shared<VisionaryControl>();
}

VisionaryCamera::~VisionaryCamera()
{
	close();
}

OpenResult VisionaryCamera::open()
{
	// base case
	OpenResult ret;
	if (pDataStream->isConnected()) { return ret; }

	// try specified ip address first, then auto scan if failed
	if (!pDataStream->open(ipAddress, htons(dataPort)))
	{
		VisionaryAutoIPScanCustom scanner;
		auto devices = scanner.doScan(5000);
		if (devices.empty())
		{
			ret.error = ErrorCode::CAMERA_NOT_FOUND;
			ret.message = "could not find camera with specified ip address and failed to auto scan";
			return ret;
		}
		else
		{
			auto dev = devices.front();
			setIp(dev.IpAddress);
			if (!pDataStream->open(ipAddress, htons(dataPort)))
			{
				ret.error = ErrorCode::UNREACHABLE_ADDRESS;
				ret.message = "auto scan found a camera but the address is unreachable from the current network";
				return ret;
			}
		}
	}

	if (!pVisionaryControl->open(VisionaryControl::ProtocolType::COLA_2, ipAddress, openTimeout/*ms*/))
	{
		ret.error = ErrorCode::UNKNOWN;
		ret.message = "unable to open control stream with cola_2 protocol";
		return ret;
	}

	if (!pVisionaryControl->login(IAuthentication::UserLevel::AUTHORIZED_CLIENT, "PASSWORD"))
	{
		ret.error = ErrorCode::LOGIN_ERROR;
		ret.message = "unable to login as authorized client";
		return ret;
	}

	//connected = true;
	ret.error = ErrorCode::NONE_ERROR;
	ret.message = "successfully opened";
	return ret;
}

bool VisionaryCamera::close()
{
	if (pVisionaryControl)
	{
		//if (capturing)
			pVisionaryControl->stopAcquisition();
		//pVisionaryControl->logout();
		pVisionaryControl->close();
	}
	if (pDataStream)
		pDataStream->close();
	//connected = false;
	return true;
}

bool VisionaryCamera::isOpen()
{
	return pDataStream->isConnected();
}

bool VisionaryCamera::isAvailable()
{
	return ping(ipAddress);
}

bool VisionaryCamera::startCapture()
{
	//if (!connected) { return false; }
	//if (capturing) { return true; }
	pVisionaryControl->stopAcquisition();
	//capturing = pVisionaryControl->startAcquisition();
	//return capturing;
	return pVisionaryControl->startAcquisition();
}

bool VisionaryCamera::stopCapture()
{
	//if (!capturing) { return true; }
	//capturing = false;
	return pVisionaryControl->stopAcquisition();
}

bool VisionaryCamera::getNextFrameset(Frameset::frameset_t& fs)
{
	//if (!capturing)
	//	return false;

	if (!pDataStream->getNextFrame())
		return false;

	fs.depth = pDataHandler->getDistanceMap();
	fs.intensity = pDataHandler->getIntensityMap();
	fs.state = pDataHandler->getStateMap();
	fs.height = pDataHandler->getHeight();
	fs.width = pDataHandler->getWidth();
	fs.number = pDataHandler->getFrameNum();
	fs.time = pDataHandler->getTimestampMS();

	// divide depth data by 4 to convert to millimeters
	for (auto&& val : fs.depth)
	{
		val >>= 2;
	}

	return true;
}

const std::map<std::string, std::string> VisionaryCamera::getParameters()
{
	return parameters;
}

bool VisionaryCamera::available(int timeout)
{
	VisionaryAutoIPScanCustom scanner;
	auto devices = scanner.doScan(timeout);
	for (auto const& device : devices)
	{
		if (device.IpAddress == this->ipAddress)
		{
			return true;
		}
	}
	return false;
}

void VisionaryCamera::setIp(std::string ipAddress)
{
	this->ipAddress = ipAddress;

	parameters.insert_or_assign("ipAddress", this->ipAddress);
}

void VisionaryCamera::setDataPort(short dataPort)
{
	this->dataPort = dataPort;

	parameters.insert_or_assign("dataPort", std::to_string(this->dataPort));
}

const bool VisionaryCamera::ping(const std::string ip)
{
	HANDLE hIcmpFile;
	unsigned long ipaddr = INADDR_NONE;
	DWORD dwRetVal = 0;
	char SendData[32] = "Data Buffer";
	LPVOID ReplyBuffer = NULL;
	DWORD ReplySize = 0;

	ipaddr = inet_addr(ipAddress.c_str());
	if (ipaddr == INADDR_NONE)
	{
		qDebug() << "ping: ip address cannot be 'none'";
		return false;
	}

	hIcmpFile = IcmpCreateFile();
	if (hIcmpFile == INVALID_HANDLE_VALUE)
	{
		qDebug() << "ping: unable to open handle. last error:" << GetLastError();
		return false;
	}

	ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData);
	ReplyBuffer = (VOID*)malloc(ReplySize);
	if (ReplyBuffer == NULL)
	{
		qDebug() << "ping: unable to allocate memory";
		return false;
	}

	dwRetVal = IcmpSendEcho(hIcmpFile, ipaddr, SendData, sizeof(SendData), NULL, ReplyBuffer, ReplySize, 1000);
	if (dwRetVal != 0)
	{
		PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
		struct in_addr ReplyAddr;
		ReplyAddr.S_un.S_addr = pEchoReply->Address;
		qDebug() << "ping: send icmp message to" << ip;
		if (dwRetVal > 1)
		{
			qDebug() << "ping: received" << dwRetVal << "icmp message responses";
			qDebug() << "ping: information from the first response:";
		}
		else
		{
			qDebug() << "ping: received" << dwRetVal << "icmp message response";
			qDebug() << "ping: information from this response:";
		}
		qDebug() << "ping: received from" << inet_ntoa(ReplyAddr);
		qDebug() << "ping: status =" << pEchoReply->Status;
		qDebug() << "ping: roundtrip time =" << pEchoReply->RoundTripTime << "ms";
	}
	else
	{
		qDebug() << "ping: call to IcmpSendEcho failed";
		qDebug() << "ping: IcmpSendEcho returned error:" << GetLastError();
		return false;
	}
	return true;
}

