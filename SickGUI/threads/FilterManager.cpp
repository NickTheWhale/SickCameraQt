#include "FilterManager.h"

FilterManager& FilterManager::instance()
{
    static FilterManager _instance;
    return _instance;
}

void FilterManager::setFilterFlags(const FilterFlags& flags)
{
    QMutexLocker locker(&flagMutex);
    this->flags = flags;
}

const FilterFlags FilterManager::getFilterFlags()
{
    QMutexLocker locker(&flagMutex);
    return flags;
}

void FilterManager::setMask(const std::vector<bool>& mask)
{
    QMutexLocker locker(&maskMutex);
    this->mask = mask;
}

const std::vector<bool> FilterManager::getMask()
{
    QMutexLocker locker(&maskMutex);
    return mask;
}

FilterManager::FilterManager() :
    flags(FilterFlags::NONE)
{
}

FilterManager::~FilterManager()
{
}


