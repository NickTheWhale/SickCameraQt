#include "VoxelFilterModel.h"

#include <FrameNodeData.h>

#include <pcl/point_types.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/filters/statistical_outlier_removal.h>

#include <qlayout.h>
#include <qlabel.h>

VoxelFilterModel::VoxelFilterModel() :
	_widget(new QWidget()),
	sb1(new QSpinBox()),
	sb2(new QSpinBox()),
	sb3(new QSpinBox())
{
	sb1->setRange(0, 100);
	sb2->setRange(0, 100);
	sb3->setRange(0, 100);

	connect(sb1, &QSpinBox::valueChanged, this, [=]() { applyFilter(); });
	connect(sb2, &QSpinBox::valueChanged, this, [=]() { applyFilter(); });
	connect(sb3, &QSpinBox::valueChanged, this, [=]() { applyFilter(); });

	auto hbox = new QHBoxLayout();
	hbox->addWidget(new QLabel("Voxel Size"));
	hbox->addWidget(sb1);
	hbox->addWidget(sb2);
	hbox->addWidget(sb3);

	_widget->setLayout(hbox);
}

unsigned int VoxelFilterModel::nPorts(QtNodes::PortType const portType) const
{
	return 1;
}

QtNodes::NodeDataType VoxelFilterModel::dataType(QtNodes::PortType const portType, QtNodes::PortIndex const portIndex) const
{
	return FrameNodeData().type();
}

std::shared_ptr<QtNodes::NodeData> VoxelFilterModel::outData(QtNodes::PortIndex const port)
{
	return _currentNodeData;
}

void VoxelFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
	_originalNodeData = nodeData;
	_currentNodeData = nodeData;

	applyFilter();
}

void VoxelFilterModel::applyFilter()
{
	if (_originalNodeData)
	{
		using CloudPtr = pcl::PointCloud<pcl::PointXYZ>::Ptr;
		using Cloud = pcl::PointCloud<pcl::PointXYZ>;

		auto d = std::dynamic_pointer_cast<FrameNodeData>(_originalNodeData);
		CloudPtr cloud = frameset::toCloud(d->frame());
		CloudPtr filteredCloud(new Cloud);

		pcl::StatisticalOutlierRemoval<pcl::PointXYZ> filter;
		filter.setInputCloud(cloud);
		filter.setMeanK(50);
		filter.setStddevMulThresh(1.0);
		filter.filter(*filteredCloud);




		_currentNodeData = std::make_shared<FrameNodeData>(frameset::fromCloud(filteredCloud));
	}

	emit dataUpdated(0);
}
