/*
 * Banana Qt Libraries
 *
 * Copyright (c) 2016 Alexandra Cherdantseva
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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

