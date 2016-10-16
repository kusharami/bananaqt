#pragma once

#include "IChildFilter.h"

#include <QList>

#include <memory>
#include <map>

namespace Core
{
	typedef QList<IChildFilter *> ChildFilters;

	class ChildFilter : public IChildFilter
	{
	public:
		virtual bool filterMatch(QObject *object) const override;
		virtual bool shouldFilterDeeper(QObject *object) const override;

		ChildFilters getFilters() const;
		int getFiltersCount() const;
		void addFilter(IChildFilter *filter, bool own = false);
		void removeFilter(IChildFilter *filter);

		virtual void clearFilters();

	protected:
		virtual void beforeChangeFilters();
		virtual void afterChangeFilters();

		typedef std::unique_ptr<IChildFilter> FilterPtr;

		std::map<IChildFilter *, FilterPtr> filters;
	};
}
