#include "NameChildFilter.h"

#include <QObject>

namespace Banana
{

	NameChildFilter::NameChildFilter()
		: options(ContainsString)
	{

	}

	NameChildFilter::NameChildFilter(const QString &str, Options options)
		: str(str)
		, options(options)
	{

	}

	bool NameChildFilter::filterMatch(QObject *object) const
	{
		auto sensitivity = (0 != (options & CaseInsensitive)
						   ? Qt::CaseInsensitive
						   : Qt::CaseSensitive);

		QString name(object->objectName());

		if (MatchString == (options & MatchString))
		{
			return (0 == name.compare(str, sensitivity));
		} else
		if (0 != (options & StartsWithString))
		{
			return name.startsWith(str, sensitivity);
		} else
		if (0 != (options & EndsWithString))
		{
			return name.endsWith(str, sensitivity);
		}

		return name.contains(str, sensitivity);
	}

	bool NameChildFilter::shouldFilterDeeper(QObject *) const
	{
		return true;
	}

	void NameChildFilter::setOptions(Options options)
	{
		this->options = options;
	}

	NameChildFilter::Options NameChildFilter::getOptions() const
	{
		return options;
	}

	void NameChildFilter::setString(const QString &value)
	{
		str = value;
	}

	const QString &NameChildFilter::getString() const
	{
		return str;
	}

}
