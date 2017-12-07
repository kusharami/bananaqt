/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2017 Alexandra Cherdantseva

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

#include "ScriptUtils.h"

#include <QScriptEngine>
#include <QScriptValueIterator>
#include <QVariant>
#include <QMetaProperty>

#include <QCoreApplication>
#include <QDebug>

namespace Banana
{
namespace Script
{
static const QString NULL_KEY = QStringLiteral("null");

static const char BAD_NUMBER_OF_ARGUMENTS_MESSAGE[] =
	QT_TRANSLATE_NOOP("Script", "Bad number of arguments.");

static const char INCOMPATIBLE_ARG_TYPE_MESSAGE[] =
	QT_TRANSLATE_NOOP("Script", "Type of argument #%1 is incompatible.");

static const char UNKNOWN_ERROR_MESSAGE[] =
	QT_TRANSLATE_NOOP("Script", "Unknown error.");

QScriptValue VariantToScriptValue(
	const QVariant &variant, QScriptEngine *engine)
{
	QScriptValue result;

	switch (variant.type())
	{
		case QVariant::Map:
		{
			auto vmap = variant.toMap();

			result = engine->newObject();

			for (auto it = vmap.begin(); it != vmap.end(); ++it)
			{
				auto &key = it.key();
				auto &value = it.value();

				result.setProperty(key, VariantToScriptValue(value, engine));
			}
			break;
		}

		case QVariant::List:
		case QVariant::StringList:
		{
			auto vlist = variant.toList();

			int len = vlist.length();
			result = engine->newArray(len);

			for (int i = 0; i < len; i++)
			{
				auto &value = vlist.at(i);

				result.setProperty(i, VariantToScriptValue(value, engine));
			}
			break;
		}

		default:
		{
			if (variant.isNull() || !variant.isValid())
				result = QScriptValue(engine, QScriptValue::NullValue);
			else
				switch (variant.type())
				{
					case QVariant::Bool:
						result = QScriptValue(engine, variant.toBool());
						break;

					case QVariant::Int:
						result = QScriptValue(engine, variant.toInt());
						break;

					case QVariant::UInt:
						result = QScriptValue(engine, variant.toUInt());
						break;

					case QVariant::LongLong:
					case QVariant::ULongLong:
					case QVariant::Double:
						result = QScriptValue(engine, variant.toDouble());
						break;

					case QVariant::Char:
					case QVariant::String:
						result = QScriptValue(engine, variant.toString());
						break;

					default:
						result = engine->newVariant(variant);
						break;
				}

			break;
		}
	}

	return result;
}

QScriptValue ThrowBadNumberOfArguments(QScriptContext *context)
{
	return context->throwError(QCoreApplication::translate(
		CSTRKEY(Script), BAD_NUMBER_OF_ARGUMENTS_MESSAGE));
}

QScriptValue ThrowIncompatibleArgumentType(QScriptContext *context, int idx)
{
	return context->throwError(QCoreApplication::translate(
		CSTRKEY(Script), INCOMPATIBLE_ARG_TYPE_MESSAGE)
								   .arg(idx));
}

enum
{
	GET_SET =
		(int) QScriptValue::PropertyGetter | (int) QScriptValue::PropertySetter
};

static bool IsStoredProperty(
	const QScriptValueIterator &it, const QMetaObject *metaObject = nullptr)
{
	auto flags = it.flags();
	bool readOnly = (0 != (flags & QScriptValue::ReadOnly));
	if (readOnly)
		return false;

	auto getSet = flags & GET_SET;
	if (getSet != 0 && getSet != GET_SET)
		return false;

	if (metaObject != nullptr)
	{
		if (getSet == 0 && it.value().isFunction())
			return false;
		int idx = metaObject->indexOfProperty(it.name().toLatin1());
		auto metaProperty = metaObject->property(idx);
		if (!metaProperty.isValid())
			return false;

		if (!metaProperty.isStored())
			return false;
	}

	return true;
}

static QScriptValue ValuePropertyGet(
	const QScriptValue &owner, const QScriptValueIterator &it)
{
	auto v = it.value();
	if (GET_SET == (it.flags() & GET_SET))
	{
		qDebug() << it.name();
		v = owner.property(it.name());
	}

	return v;
}

QVariant ScriptValueToVariant(const QScriptValue &value, bool links)
{
	if (links && value.isQObject())
	{
		auto name = value.property(QSTRKEY(name));
		if (!name.isString())
			return value.toQObject()->objectName();

		return name.toVariant();
	} else if (links && value.isQMetaObject())
	{
		return QLatin1String(value.toQMetaObject()->className());
	} else if (value.isArray())
	{
		QVariantList vlist;

		int len = value.property("length").toInt32();

		for (int i = 0; i < len; i++)
		{
			auto v = value.property(i);

			vlist.push_back(ScriptValueToVariant(v, true));
		}

		return vlist;
	} else if (value.isQObject() || value.isObject())
	{
		QVariantMap vmap;

		auto object = value.toQObject();
		const QMetaObject *metaObject = nullptr;
		if (nullptr != object)
			metaObject = object->metaObject();

		QScriptValueIterator it(value);

		while (it.hasNext())
		{
			it.next();

			if (IsStoredProperty(it, metaObject))
			{
				vmap.insert(it.name(),
					ScriptValueToVariant(ValuePropertyGet(value, it), true));
			}
		}

		return vmap;
	}

	return value.toVariant();
}

void CopyScriptProperties(const QScriptValue &from, QScriptValue &to)
{
	auto object = from.toQObject();
	const QMetaObject *metaObject = nullptr;
	if (nullptr != object)
		metaObject = object->metaObject();

	auto engine = to.engine();

	QScriptValueIterator it(from);

	int i = 0;

	while (it.hasNext())
	{
		it.next();

		if (IsStoredProperty(from, metaObject))
		{
			auto value = ValuePropertyGet(from, it);

			if (to.isQObject())
			{
				to.setProperty(it.name(), value);
			} else if (to.isArray())
			{
				if (from.isArray() && it.name() == "length")
				{
					to.setProperty(it.name(), value);
					continue;
				}

				if (!value.isArray() && !value.isObject())
				{
					to.setProperty(i, value);
				} else if (value.isArray())
				{
					auto arr = engine->newArray();
					CopyScriptProperties(value, arr);
					to.setProperty(i, arr);
				} else
				{
					auto obj = engine->newObject();
					CopyScriptProperties(value, obj);
					to.setProperty(i, obj);
				}
				i++;
			} else
			{
				auto value = it.value();
				if (!value.isArray() && !value.isObject())
				{
					to.setProperty(i, value);
				} else if (value.isArray())
				{
					auto arr = engine->newArray();
					CopyScriptProperties(value, arr);
					to.setProperty(it.name(), arr);
				} else
				{
					auto obj = engine->newObject();
					CopyScriptProperties(value, obj);
					to.setProperty(it.name(), obj);
				}
			}
		}
	}
}

QScriptValue ConstructQObject(QObject *object, QScriptContext *context,
	QScriptEngine *engine, QScriptEngine::ValueOwnership ownership,
	QScriptEngine::QObjectWrapOptions options)
{
	if (not context->isCalledAsConstructor())
	{
		return engine->newQObject(
			context->thisObject(), object, ownership, options);
	}

	return engine->newQObject(object, ownership, options);
}

void RegisterQMetaObject(QScriptEngine *engine, const QMetaObject *metaObject,
	QScriptValue (*constructor)(QScriptContext *, QScriptEngine *))
{
	auto currentObject = engine->globalObject();

	QStringList path = QString(metaObject->className()).split("::");

	for (int i = 0, count = path.count(); i < count; i++)
	{
		auto &name = path.at(i);
		auto next = currentObject.property(name);
		if (!next.isValid() || next.isUndefined())
		{
			if (i == count - 1)
			{
				next = engine->newFunction(
					constructor, engine->newQMetaObject(metaObject));
			} else
			{
				next = engine->newObject();
			}
		}
		currentObject.setProperty(name, next);
		currentObject = next;
	}
}

QScriptValue ThrowUnknownError(QScriptContext *context)
{
	return context->throwError(
		QCoreApplication::translate(CSTRKEY(Script), UNKNOWN_ERROR_MESSAGE));
}

QScriptValue NewQObjectPrototype(QScriptEngine *engine, QObject *object)
{
	Q_ASSERT(nullptr != engine);
	Q_ASSERT(nullptr != object);
	return engine->newQObject(object, QScriptEngine::QtOwnership,
		QScriptEngine::ExcludeChildObjects | QScriptEngine::ExcludeDeleteLater |
			QScriptEngine::SkipMethodsInEnumeration);
}
}
}
