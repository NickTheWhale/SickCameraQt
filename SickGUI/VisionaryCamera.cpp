#include "VisionaryCamera.h"
#include "../VisionaryAutoIPScanCustom.h"
#include "../VisionaryFrameset.h"
#include <CoLaParameterWriter.h>
#include <CoLaCommand.h>
#include <CoLaCommandType.h>
#include <qdebug.h>
#include <vector>
#include <qtimer.h>


// ping test
#include <WinSock2.h>
#include <iphlpapi.h>
#include <IcmpAPI.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

using namespace visionary;


VisionaryCamera::VisionaryCamera(std::string ipAddress, short dataPort, QObject* parent) :
	QObject(parent),
	ipAddress(ipAddress), 
	dataPort(dataPort),
	frameGrabber(ipAddress, htons(dataPort), grabberTimeout)
{
	parameters.insert_or_assign("ipAddress", this->ipAddress);
	parameters.insert_or_assign("dataPort", std::to_string(this->dataPort));

	// create camera handlers
	pDataHandler = std::make_shared<visionary::VisionaryTMiniData>();
	pVisionaryControl = std::make_shared<VisionaryControl>();
}

VisionaryCamera::~VisionaryCamera()
{
	close();
}

OpenResult VisionaryCamera::open()
{
	OpenResult ret;

	if (!pVisionaryControl->open(VisionaryControl::ProtocolType::COLA_2, ipAddress, openTimeout/*ms*/))
	{
		ret.error = ErrorCode::UNKNOWN;
		ret.message = "failed to open control stream";
		return ret;
	}

	if (!pVisionaryControl->login(IAuthentication::UserLevel::SERVICE, "CUST_SERV"))
	{
		ret.error = ErrorCode::LOGIN_ERROR;
		ret.message = "failed to login with SERVICE level";
		return ret;
	}

	ret.error = ErrorCode::NONE_ERROR;
	ret.message = "successfully opened";
	return ret;
}

bool VisionaryCamera::close()
{
	if (pVisionaryControl)
	{
		pVisionaryControl->stopAcquisition();
		//pVisionaryControl->logout();
		pVisionaryControl->close();
	}
	return true;
}

bool VisionaryCamera::isOpen()
{
	return true;
}

bool VisionaryCamera::isAvailable()
{
	return ping(ipAddress);
}

bool VisionaryCamera::startCapture()
{
	pVisionaryControl->stopAcquisition();
	return pVisionaryControl->startAcquisition();
}

bool VisionaryCamera::stopCapture()
{
	return pVisionaryControl->stopAcquisition();
}

bool VisionaryCamera::getNextFrameset(Frameset::frameset_t& fs)
{
	if (!frameGrabber.getNextFrame(pDataHandler))
		return false;

	fs.depth = pDataHandler->getDistanceMap();
	fs.intensity = pDataHandler->getIntensityMap();
	fs.state = pDataHandler->getStateMap();
	fs.height = pDataHandler->getHeight();
	fs.width = pDataHandler->getWidth();
	fs.number = pDataHandler->getFrameNum();
	fs.time = pDataHandler->getTimestampMS();

	// divide depth data by 4 to convert to millimeters
	//for (auto&& val : fs.depth)
	//{
	//	val >>= 2;
	//}

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

void VisionaryCamera::setDepthFilterRange(const uint16_t low, const uint16_t high)
{
	qDebug() << __FUNCTION__ << "called";
	CoLaCommand minCmd = CoLaParameterWriter(CoLaCommandType::WRITE_VARIABLE, "minDistThresh").parameterUInt(low).build();
	CoLaCommand minResponse = pVisionaryControl->sendCommand(minCmd);

	CoLaCommand maxCmd = CoLaParameterWriter(CoLaCommandType::WRITE_VARIABLE, "maxDistThresh").parameterUInt(high).build();
	CoLaCommand maxResponse = pVisionaryControl->sendCommand(maxCmd);

	if (minResponse.getError() != CoLaError::OK || maxResponse.getError() != CoLaError::OK)
		qDebug() << __FUNCTION__ << "failed:" << minResponse.getError() << maxResponse.getError();
	else 
		qDebug() << __FUNCTION__ << "success";
}

void VisionaryCamera::setDepthFilterEnable(const bool enable)
{
	qDebug() << __FUNCTION__ << "called";
	CoLaCommand cmd = CoLaParameterWriter(CoLaCommandType::WRITE_VARIABLE, "enDistFilter").parameterBool(enable).build();
	CoLaCommand response = pVisionaryControl->sendCommand(cmd);
	if (response.getError() != CoLaError::OK)
		qDebug() << __FUNCTION__ << "failed:" << response.getError();
	else 
		qDebug() << __FUNCTION__ << "success";
}

void VisionaryCamera::setDepthMaskEnable(const bool enable)
{
	qDebug() << __FUNCTION__ << "called";
	CoLaCommand cmd = CoLaParameterWriter(CoLaCommandType::WRITE_VARIABLE, "enDepthMask").parameterBool(enable).build();
	CoLaCommand response = pVisionaryControl->sendCommand(cmd);
	if (response.getError() != CoLaError::OK)
		qDebug() << __FUNCTION__ << "failed:" << response.getError();
	else
		qDebug() << __FUNCTION__ << "success";
}
