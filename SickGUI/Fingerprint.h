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
	struct CellStat
	{
		uint16_t min;
		uint16_t max;
		double mean;
		double stdev;

		CellStat(const uint16_t& min, const uint16_t& max, double mean, double stdev)
			: min(min), max(max), mean(mean), stdev(stdev)
		{
		}

		std::string to_string()
		{
			return std::format("min: {}, max: {}, mean: {}, stdev: {}",
				min, max, mean, stdev);
		}
	};

	const inline CellStat calculateCellStatistics(int frameWidth, int frameHeight, const std::vector<uint16_t>& frame, QPoint topLeft, QPoint bottomRight)
	{
		std::vector<uint16_t> cellData;
		int cellWidth = bottomRight.x() - topLeft.x() + 1;
		int cellHeight = bottomRight.y() - topLeft.y() + 1;
		cellData.resize(cellWidth * cellHeight);

		for (auto y = topLeft.y(); y <= bottomRight.y(); ++y)
		{
			for (auto x = topLeft.x(); x <= bottomRight.x(); ++x)
			{
				auto val = frame[y * frameWidth + x];
				if (val > 0)
					cellData.push_back(frame[y * frameWidth + x]);
			}
		}
		// min
		const uint16_t min = *std::min_element(cellData.begin(), cellData.end());
		// max
		const uint16_t max = *std::max_element(cellData.begin(), cellData.end());
		double sum = std::accumulate(std::begin(cellData), std::end(cellData), 0.0);
		// mean
		const double mean = sum / cellData.size();
		double accum = 0.0;
		std::for_each(std::begin(cellData), std::end(cellData), [&](const double d) {
			accum += (d - mean) * (d - mean);
			});
		// stddev
		const double stdev = sqrt(accum / (cellData.size() - 1));
		const auto stat = CellStat(min, max, mean, stdev);
		return stat;
	}

	inline void overlayStats(QImage& qImage, int width, int height, const std::vector<uint16_t>& frame)
	{
		const auto NUM_COL = 4;
		const auto NUM_ROW = 7;
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

				painter.drawText(x, y, QString("mean:	%1").arg(std::round(stat.mean  ))); y += FONT_SIZE + TEXT_PADDING;
				painter.drawText(x, y, QString("min:	%1").arg(std::round(stat.min   ))); y += FONT_SIZE + TEXT_PADDING;
				painter.drawText(x, y, QString("max:	%1").arg(std::round(stat.max   ))); y += FONT_SIZE + TEXT_PADDING;
				painter.drawText(x, y, QString("stdev:	%1").arg(std::round(stat.stdev )));

				painter.drawRect(QRect(topLeft, bottomRight - QPoint(1, 1)));
			}
		}
		painter.end();
	}
}