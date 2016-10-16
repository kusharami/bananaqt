#include "ChildFilter.h"

namespace Core
{

	bool ChildFilter::filterMatch(QObject *object) const
	{
		for (auto &item : filters)
		{
			if (!item.first->filterMatch(object))
				return false;
		}

		return true;
	}

	bool ChildFilter::shouldFilterDeeper(QObject *) const
	{
		return true;
	}

	ChildFilters ChildFilter::getFilters() const
	{
		ChildFilters result;

		for (auto &item : filters)
		{
			result.push_back(item.first);
		}

		return result;
	}

	int ChildFilter::getFiltersCount() const
	{
		return static_cast<int>(filters.size());
	}

	void ChildFilter::addFilter(IChildFilter *filter, bool own)
	{
		auto it = filters.find(filter);
		if (filters.end() == it)
		{
			beforeChangeFilters();

			filters[filter] = own ? FilterPtr(filter) : FilterPtr();

			afterChangeFilters();
		} else
		if ((it->second != nullptr) != own)
		{
			if (own)
			{
				it->second = FilterPtr(filter);
			} else
			{
				it->second.release();
			}
		}
	}

	void ChildFilter::removeFilter(IChildFilter *filter)
	{
		auto it = filters.find(filter);
		if (filters.end() != it)
		{
			beforeChangeFilters();

			filters.erase(it);

			afterChangeFilters();
		}
	}

	void ChildFilter::clearFilters()
	{
		filters.clear();
	}

	void ChildFilter::beforeChangeFilters()
	{

	}

	void ChildFilter::afterChangeFilters()
	{

	}

}
