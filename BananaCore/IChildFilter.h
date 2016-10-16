#pragma once

class QObject;

namespace Banana
{

	struct IChildFilter
	{
		virtual ~IChildFilter() {}

		virtual bool filterMatch(QObject *object) const = 0;
		virtual bool shouldFilterDeeper(QObject *object) const = 0;
	};
}
