//
// SPDX-License-Identifier: Unlicense
// 
// Created: November 2019
// 
// SICK AG, Waldkirch
// email: TechSupport0905@sick.de

#include <cstdio>

#include "VisionaryDataStream.h"

#include <iostream>

#include "VisionaryEndian.h"

namespace visionary
{

	VisionaryDataStream::VisionaryDataStream(std::shared_ptr<VisionaryData> dataHandler) :
		m_dataHandler(std::move(dataHandler))
	{
	}

	VisionaryDataStream::~VisionaryDataStream()
	{
		close();
	}

	bool VisionaryDataStream::open(const std::string& hostname, std::uint16_t port, std::uint64_t timeoutMs)
	{
		m_pTransport = nullptr;

		std::unique_ptr<TcpSocket> pTransport(new TcpSocket());

		if (pTransport->connect(hostname, port, static_cast<long>(timeoutMs)) != 0)
		{
			return false;
		}

		m_pTransport = std::move(pTransport);

		return true;
	}

	bool VisionaryDataStream::open(std::unique_ptr<ITransport>& pTransport)
	{
		m_pTransport = std::move(pTransport);
		return true;
	}

	void VisionaryDataStream::close()
	{
		if (m_pTransport)
		{
			m_pTransport->shutdown();
			m_pTransport = nullptr;
		}
	}

	bool VisionaryDataStream::syncCoLa() const
	{
		size_t elements = 0;
		std::vector<std::uint8_t> buffer;

		while (elements < 4)
		{
			if (m_pTransport->read(buffer, 1) < 1)
			{
				return false;
			}
			if (0x02 == buffer[0])
			{
				elements++;
			}
			else
			{
				elements = 0;
			}
		}

		return true;
	}

	bool VisionaryDataStream::getNextFrame()
	{
		if (!syncCoLa())
		{
			return false;
		}

		std::vector<uint8_t> buffer;

		// Read package length
		if (m_pTransport->read(buffer, sizeof(uint32_t)) < static_cast<TcpSocket::recv_return_t>(sizeof(uint32_t)))
		{
			std::cout << "Received less than the required 4 package length bytes." << std::endl;
			return false;
		}

		const auto packageLength = readUnalignBigEndian<uint32_t>(buffer.data());

		if (packageLength < 3u)
		{
			std::cout << "Invalid package length " << packageLength << ". Should be at least 3" << std::endl;
			return false;
		}

		// Receive the FrameType data
		size_t remainingBytesToReceive = packageLength;
		if (m_pTransport->read(buffer, remainingBytesToReceive) < static_cast<ITransport::recv_return_t>(remainingBytesToReceive))
		{
			std::cout << "Received less than the required " << remainingBytesToReceive << " bytes." << std::endl;
			return false;
		}

		// Check that protocol version and packet type are correct
		const auto protocolVersion = readUnalignBigEndian<uint16_t>(buffer.data());
		const auto packetType = readUnalignBigEndian<uint8_t>(buffer.data() + 2);
		if (protocolVersion != 0x001)
		{
			std::cout << "Received unknown protocol version " << protocolVersion << "." << std::endl;
			return false;
		}
		if (packetType != 0x62)
		{
			std::cout << "Received unknown packet type " << packetType << "." << std::endl;
			return false;
		}
		return parseSegmentBinaryData(buffer.begin() + 3, buffer.size() - 3u); // Skip protocolVersion and packetType
	}

	bool VisionaryDataStream::parseSegmentBinaryData(std::vector<uint8_t>::iterator itBuf, size_t bufferSize)
	{
		if (m_dataHandler == nullptr)
		{
			std::cout << "No datahandler is set -> cant parse blob data" << std::endl;
			return false;
		}
		bool result = false;
		auto itBufSegment = itBuf;
		auto remainingSize = bufferSize;

		if (remainingSize < 4)
		{
			std::cout << "Received not enough data to parse segment description. Connection issues?" << std::endl;
			return false;
		}


		//-----------------------------------------------
		// Extract informations in Segment-Binary-Data
		//const uint16_t blobID = readUnalignBigEndian<uint16_t>(&*itBufSegment);
		itBufSegment += sizeof(uint16_t);
		const auto numSegments = readUnalignBigEndian<uint16_t>(&*itBufSegment);
		itBufSegment += sizeof(uint16_t);
		remainingSize -= 4;

		//offset and changedCounter, 4 bytes each per segment
		std::vector<uint32_t> offset(numSegments);
		std::vector<uint32_t> changeCounter(numSegments);
		const uint16_t segmentDescriptionSize = 4u + 4u;
		const size_t totalSegmentDescriptionSize = static_cast<size_t>(numSegments * segmentDescriptionSize);
		if (remainingSize < totalSegmentDescriptionSize)
		{
			std::cout << "Received not enough data to parse segment description. Connection issues?" << std::endl;
			return false;
		}
		if (numSegments < 3)
		{
			std::cout << "Invalid number of segments. Connection issues?" << std::endl;
			return false;
		}
		for (uint16_t i = 0; i < numSegments; i++)
		{
			offset[i] = readUnalignBigEndian<uint32_t>(&*itBufSegment);
			itBufSegment += sizeof(uint32_t);
			changeCounter[i] = readUnalignBigEndian<uint32_t>(&*itBufSegment);
			itBufSegment += sizeof(uint32_t);
		}
		remainingSize -= totalSegmentDescriptionSize;

		//-----------------------------------------------
		// First segment contains the XML Metadata
		const size_t xmlSize = offset[1] - offset[0];
		if (remainingSize < xmlSize)
		{
			std::cout << "Received not enough data to parse xml Description. Connection issues?" << std::endl;
			return false;
		}
		remainingSize -= xmlSize;
		std::string xmlSegment((itBuf + offset[0]), (itBuf + offset[1]));
		if (m_dataHandler->parseXML(xmlSegment, changeCounter[0]))
		{
			//-----------------------------------------------
			// Second segment contains Binary data
			size_t binarySegmentSize = offset[2] - offset[1];
			if (remainingSize < binarySegmentSize)
			{
				std::cout << "Received not enough data to parse binary Segment. Connection issues?" << std::endl;
				return false;

			}
			result = m_dataHandler->parseBinaryData((itBuf + offset[1]), binarySegmentSize);
			remainingSize -= binarySegmentSize;
		}
		return result;
	}

	std::shared_ptr<VisionaryData> VisionaryDataStream::getDataHandler()
	{
		return m_dataHandler;
	}

	void VisionaryDataStream::setDataHandler(std::shared_ptr<VisionaryData> dataHandler)
	{
		m_dataHandler = std::move(dataHandler);
	}

	bool VisionaryDataStream::isConnected() const
	{
		const std::vector<char> data{'B', 'l', 'b', 'R', 'q', 's', 't' };
		if (m_pTransport == nullptr)
			return false;
		const auto ret = m_pTransport->send(data);
		// getLastError does not return an error code on windows if send fails
#ifdef _WIN32
		if (ret < 0)
		{
			return false;
		}
#else
		(void)ret;
#endif
		const auto err = m_pTransport->getLastError();
		return err == 0;
	}

}
