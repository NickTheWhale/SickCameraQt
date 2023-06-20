#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <string>
#include <format>
#include <qimage.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qstring.h>

#include <qdebug.h>

namespace Fingerprint
{
	const size_t GRID_ROWS = 3;
	const size_t GRID_COLS = 3;

	struct CellStat
	{
		uint16_t min;
		uint16_t max;
		uint16_t mean;
		uint16_t stdev;

		CellStat(const uint16_t& min, const uint16_t& max, const uint16_t& mean, const uint16_t& stdev)
			: min(min), max(max), mean(mean), stdev(stdev)
		{
		}

		CellStat operator+(const CellStat& other)
		{
			return CellStat(min + other.min, max + other.max, mean + other.mean, stdev + other.stdev);
		}

		CellStat& operator+=(const CellStat& other)
		{
			min += other.min;
			max += other.max;
			mean += other.mean;
			stdev += other.stdev;
			return *this;
		}

		std::string to_string()
		{
			return std::format("min: {}, max: {}, mean: {}, stdev: {}",
				min, max, mean, stdev);
		}
	};

	const inline CellStat calculateCellStatistics(int frameWidth, int frameHeight, const std::vector<uint16_t>& frame, QPoint topLeft, QPoint bottomRight)
	{
		auto cellSize = (bottomRight.x() - topLeft.x()) * (bottomRight.y() - topLeft.y());

		std::vector<uint16_t> cellData;
		for (auto y = topLeft.y(); y < bottomRight.y(); ++y)
		{
			for (auto x = topLeft.x(); x < bottomRight.x(); ++x)
			{
				auto val = frame[y * frameWidth + x];
				if (val > 0)
					cellData.push_back(val);
			}
		}
		// min
		const uint16_t min = *std::min_element(cellData.begin(), cellData.end());
		// max
		const uint16_t max = *std::max_element(cellData.begin(), cellData.end());
		// mean
		size_t sum = std::accumulate(std::begin(cellData), std::end(cellData), 0);
		const size_t mean = sum / cellData.size();
		// stdev
		size_t accum = 0;
		for (const auto& d : cellData)
		{
			accum += (d - mean) * (d - mean);
		}
		const size_t stdev = static_cast<size_t>(sqrt(accum / (cellData.size() - 1)));
		const auto stat = CellStat(min, max, mean, stdev);
		return stat;
	}

	const inline unsigned long calculateFingerprint(int frameWidth, int frameHeight, const std::vector<uint16_t>& frame)
	{
		auto cStat = CellStat(0, 0, 0, 0);
		for (auto row = 0; row < GRID_ROWS; ++row)
		{
			for (auto col = 0; col < GRID_COLS; ++col)
			{
				const auto topLeft = QPoint(col * (frameWidth / GRID_COLS), row * (frameHeight / GRID_ROWS));
				const auto bottomRight = topLeft + QPoint(frameWidth / GRID_COLS, frameHeight / GRID_ROWS);
				const auto stat = calculateCellStatistics(frameWidth, frameHeight, frame, topLeft, bottomRight);
				cStat += stat;
			}
		}
		return cStat.min + cStat.max + cStat.mean + cStat.stdev;
	}

	inline void overlayStats(QImage& qImage, int width, int height, const std::vector<uint16_t>& frame)
	{
		const auto NUM_COL = GRID_COLS;
		const auto NUM_ROW = GRID_ROWS;
		const auto FONT_SIZE = 12;
		const auto TEXT_PADDING = 2;
		const auto TEXT_ROW_OFFSET = FONT_SIZE + TEXT_PADDING;
		const auto TEXT_COL_OFFSET = 5;

		QPainter painter(&qImage);
		painter.setPen(Qt::red);
		painter.setFont(QFont("Arial", FONT_SIZE));

		for (auto row = 0; row < NUM_ROW; ++row)
		{
			for (auto col = 0; col < NUM_COL; ++col)
			{
				auto topLeft = QPoint(col * (width / NUM_COL), row * (height / NUM_ROW));
				auto bottomRight = topLeft + QPoint(width / NUM_COL, height / NUM_ROW);

				const auto stat = calculateCellStatistics(width, height, frame, topLeft, bottomRight);

				auto x = TEXT_COL_OFFSET + col * (width / NUM_COL);
				auto y = TEXT_ROW_OFFSET + row * (height / NUM_ROW);

				painter.drawText(x, y, QString("mean:	%1").arg(stat.mean)); y += FONT_SIZE + TEXT_PADDING;
				painter.drawText(x, y, QString("min:	%1").arg(stat.min)); y += FONT_SIZE + TEXT_PADDING;
				painter.drawText(x, y, QString("max:	%1").arg(stat.max)); y += FONT_SIZE + TEXT_PADDING;
				painter.drawText(x, y, QString("stdev:	%1").arg(stat.stdev));

				painter.drawRect(QRect(topLeft, bottomRight - QPoint(1, 1)));
			}
		}
		painter.end();
	}
}