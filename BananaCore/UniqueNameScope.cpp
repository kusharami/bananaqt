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

#include "UniqueNameScope.h"

#include "NamingPolicy.h"

namespace Banana
{

	UniqueNameScope::UniqueNameScope(const QMetaObject *meta_object,
									 Qt::CaseSensitivity sensitivity, QObject *parent)
		: QObject(parent)
		, meta_object(meta_object)
		, sensitivity(sensitivity)
	{
	}

	const QMetaObject *UniqueNameScope::getObjectType() const
	{
		return meta_object;
	}

	Qt::CaseSensitivity UniqueNameScope::getCaseSensitivity() const
	{
		return sensitivity;
	}

	void UniqueNameScope::setCaseSensitivity(Qt::CaseSensitivity value)
	{
		sensitivity = value;
	}

	void UniqueNameScope::connectObject(QObject *object)
	{
		if (nullptr != object
		&&	(	nullptr == meta_object
			||	nullptr != meta_object->cast(object)))
		{
			internalConnectObject(object);

			setObjectName(object, object->objectName());
		}
	}

	void UniqueNameScope::disconnectObject(QObject *object)
	{
			if (nullptr != object
			&&	(	nullptr == meta_object
				||	nullptr != meta_object->cast(object)))
		{
			internalDisconnectObject(object);
		}
	}

	QObject *UniqueNameScope::findSibling(QObject *object, const QString &find_name) const
	{
		auto parent = object->parent();

		if (nullptr != parent)
		{
			for (auto sibling : parent->children())
			{
				if (sibling != object
				&&	checkSiblingNameForObject(sibling, find_name, object))
				{
					return sibling;
				}
			}
		}

		return nullptr;
	}

	bool UniqueNameScope::checkSiblingNameForObject(QObject *sibling, const QString &name, QObject *) const
	{
		if (nullptr == meta_object || nullptr != meta_object->cast(sibling))
			return (0 == name.compare(sibling->objectName(), sensitivity));

		return false;
	}

	void UniqueNameScope::onObjectNameChanged(const QString &name)
	{
		setObjectName(sender(), name);
	}

	QString UniqueNameScope::getNumberedFormat(QObject *object)
	{
		return QString("%1") + getNumberSeparator(object) + QString("%2");
	}

	QString UniqueNameScope::getFixedName(const QString &name, QObject *object)
	{
		QString result(name);
		auto naming = dynamic_cast<AbstractNamingPolicy *>(object);
		if (nullptr != naming)
			result = naming->getFixedName(result);

		if (result.isEmpty())
		{
			result = QString(object->metaObject()->className());
			int cut_idx = result.lastIndexOf("::");
			if (cut_idx >= 0)
				result.remove(0, cut_idx + 2);
		}

		return result;
	}

	QString UniqueNameScope::getNumberSeparator(QObject *object)
	{
		auto naming = dynamic_cast<AbstractNamingPolicy *>(object);
		if (nullptr != naming)
		{
			return naming->getNumberSeparator();
		}

		return QString(" ");
	}

	void UniqueNameScope::setObjectName(QObject *object, const QString &name)
	{
		auto parent = object->parent();

		if (nullptr != parent)
		{
			internalDisconnectObject(object);

			QString new_name(getFixedName(name, object));
			QString new_name_without_number;

			bool no_number_initialized = false;

			quint32 number = 0;

			do
			{
				auto found = findSibling(object, new_name);

				if (nullptr == found)
				{
					object->setObjectName(new_name);
					break;
				}

				if (!no_number_initialized)
				{
					no_number_initialized = true;

					int len = new_name.length();
					if (len > 0 && new_name.at(len - 1).isDigit())
					{
						int new_len = len;
						for (int j = len - 1; j >= 0; j--)
						{
							bool ok;
							QString::fromRawData(&new_name.constData()[j], len - j).toUInt(&ok);

							if (ok)
								new_len--;
							else
								break;
						}

						if (new_len > 0)
						{
							auto number_separator = getNumberSeparator(object);
							auto sep_len = number_separator.length();
							auto check_index = new_len - sep_len;
							if (sep_len > 0
							&&	check_index >= 0
							&&	number_separator == QString::fromRawData(&new_name.constData()[check_index], sep_len))
							{
								new_len = check_index;
							}
						}

						new_name_without_number = QString(new_name.constData(), new_len);
					} else
						new_name_without_number = new_name;
				}

				if (number > 0)
				{
					new_name = getNumberedFormat(object).arg(new_name_without_number, QString::number(number));
				} else
					new_name = new_name_without_number;

				number++;
			} while (true);

			internalConnectObject(object);
		}
	}

	void UniqueNameScope::internalConnectObject(QObject *object)
	{
		QObject::connect(object, &QObject::objectNameChanged, this, &UniqueNameScope::onObjectNameChanged);
	}

	void UniqueNameScope::internalDisconnectObject(QObject *object)
	{
		QObject::disconnect(object, &QObject::objectNameChanged, this, &UniqueNameScope::onObjectNameChanged);
	}
}
