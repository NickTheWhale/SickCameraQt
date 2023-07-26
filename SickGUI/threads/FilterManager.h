#pragma once
#include <qmutex.h>

enum FilterFlags
{
	NONE = 0,
	MASK = 1,
	STATE = 2
};

class FilterManager
{
public:
	static FilterManager& instance();
	
	void setFilterFlags(const FilterFlags& flags);
	const FilterFlags getFilterFlags();

	void setMask(const std::vector<bool>& mask);
	const std::vector<bool> getMask();

private:
	explicit FilterManager();
	~FilterManager();

	QMutex flagMutex;
	FilterFlags flags;

	QMutex maskMutex;
	std::vector<bool> mask;
};

