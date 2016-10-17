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

#include "UniqueFileNameScope.h"

#include "AbstractFileSystemObject.h"

namespace Banana
{

	UniqueFileNameScope::UniqueFileNameScope(const QMetaObject *meta_object,
											 QObject *parent)
		: UniqueNameScope(meta_object, Qt::CaseInsensitive, parent)
	{

	}

	bool UniqueFileNameScope::checkSiblingNameForObject(QObject *sibling,
														const QString &name,
														QObject *object) const
	{
		auto object_fsys = dynamic_cast<AbstractFileSystemObject *>(object);
		auto sibling_fsys = dynamic_cast<AbstractFileSystemObject *>(sibling);

		if (nullptr != object_fsys && nullptr != sibling_fsys)
		{
			QString check_name(name + object_fsys->getFileExtension());
			return (0 == check_name.compare(sibling_fsys->getFileName(), Qt::CaseInsensitive));
		}

		return UniqueNameScope::checkSiblingNameForObject(sibling, name, object);
	}

}
