#pragma once

#include "IChildFilter.h"

#include <QString>

namespace Banana
{

	class NameChildFilter : public IChildFilter
	{
	public:
		enum Options
		{
			ContainsString = 0,
			StartsWithString = 1,
			EndsWithString = 1 << 1,
			MatchString = StartsWithString | EndsWithString,
			CaseInsensitive = 1 << 2
		};

		NameChildFilter();
		NameChildFilter(const QString &str,
						Options options = ContainsString);

		virtual bool filterMatch(QObject *object) const override;
		virtual bool shouldFilterDeeper(QObject *object) const override;

		void setOptions(Options options);
		Options getOptions() const;

		void setString(const QString &value);
		const QString &getString() const;

	private:
		QString str;
		Options options;
	};

}

