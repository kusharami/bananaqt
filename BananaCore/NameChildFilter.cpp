/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2016 Alexandra Cherdantseva

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/

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
	auto sensitivity = (0 != (options & CaseInsensitive) ? Qt::CaseInsensitive
														 : Qt::CaseSensitive);

	QString name(object->objectName());

	if (MatchString == (options & MatchString))
	{
		return (0 == name.compare(str, sensitivity));
	} else if (0 != (options & StartsWithString))
	{
		return name.startsWith(str, sensitivity);
	} else if (0 != (options & EndsWithString))
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
