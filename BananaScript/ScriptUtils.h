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

#pragma once

#include <QScriptEngine>

#include "BananaCore/Define.h"

#define STATIC_SCRIPT_VALUE (QScriptValue::ReadOnly | QScriptValue::Undeletable)

#define SCRIPT_REG_ENUM(object, name) \
	object.setProperty(QSTRKEY(name), (int) name, STATIC_SCRIPT_VALUE)

#define DEFINE_SCRIPT_ENUM(Type, TOTAL, enumObject, parent) \
	enumObject = engine->newObject(); \
	parent.setProperty(QSTRKEY(Type), enumObject, STATIC_SCRIPT_VALUE); \
	for (int i = 0; i < TOTAL; i++) \
	enumObject.setProperty(Type##ToStr((Type) i), i, STATIC_SCRIPT_VALUE)

#define DEFINE_SCRIPT_FLAGS(Name, Type, TOTAL, flagsObject, parent) \
	flagsObject = engine->newObject(); \
	parent.setProperty(Name, flagsObject, STATIC_SCRIPT_VALUE); \
	for (int i = 0; i < TOTAL; i++) \
	{ \
		flagsObject.setProperty( \
			Type##ToStr((Type) i), 1 << i, STATIC_SCRIPT_VALUE); \
	} \
	flagsObject.setProperty(QSTRKEY(All), (1 << TOTAL) - 1, STATIC_SCRIPT_VALUE)

class QScriptContext;

namespace Banana
{
namespace Script
{
QScriptValue ThrowBadNumberOfArguments(QScriptContext *context);
QScriptValue ThrowIncompatibleArgumentType(QScriptContext *context, int idx);
QScriptValue ThrowUnknownError(QScriptContext *context);

void RegisterQMetaObject(QScriptEngine *engine, const QMetaObject *metaObject,
	QScriptValue (*constructor)(QScriptContext *, QScriptEngine *));
QScriptValue ConstructQObject(QObject *object, QScriptContext *context,
	QScriptEngine *engine,
	QScriptEngine::ValueOwnership ownership = QScriptEngine::AutoOwnership,
	QScriptEngine::QObjectWrapOptions options =
		QScriptEngine::ExcludeChildObjects | QScriptEngine::ExcludeDeleteLater |
		QScriptEngine::SkipMethodsInEnumeration);

QVariant ScriptValueToVariant(const QScriptValue &value, bool links = false);
void CopyScriptProperties(const QScriptValue &from, QScriptValue &to);
QScriptValue VariantToScriptValue(
	const QVariant &variant, QScriptEngine *engine);
QScriptValue NewQObjectPrototype(QScriptEngine *engine, QObject *object);

template <typename T>
static QScriptValue NewQObjectPrototype(QScriptEngine *engine)
{
	Q_ASSERT(nullptr != engine);
	return NewQObjectPrototype(engine, new T(engine));
}

template <typename ENUM_T>
static QScriptValue EnumToScriptValue(QScriptEngine *engine, const ENUM_T &in)
{
	return QScriptValue(engine, qint32(in));
}

template <typename ENUM_T>
static void EnumFromScriptValue(const QScriptValue &object, ENUM_T &out)
{
	out = ENUM_T(object.toInt32());
}

template <typename From, typename To>
To ConvertImplicit(const From &from)
{
	return To(from);
}

template <typename CLASS, typename DESC_T>
static QScriptValue DescendantToScriptValue(
	QScriptEngine *engine, const DESC_T &in)
{
	return engine->newQObject(new CLASS(in), QScriptEngine::ScriptOwnership,
		QScriptEngine::ExcludeChildObjects |
			QScriptEngine::ExcludeSuperClassContents |
			QScriptEngine::ExcludeDeleteLater |
			QScriptEngine::SkipMethodsInEnumeration);
}

template <typename CLASS, typename DESC_T>
static inline void DescendantFromScriptValue(
	const QScriptValue &object, DESC_T &out)
{
	typename CLASS::Inherited temp;
	CLASS::FromScriptValue(object, temp);
	CLASS::convert(temp, out);
}

template <typename CLASS, typename DESC_T>
static QScriptValue ScriptObjectConstructor(
	QScriptEngine *engine, QScriptContext *context, const DESC_T &in)
{
	if (!context->isCalledAsConstructor())
	{
		return engine->newQObject(context->thisObject(), new CLASS(in),
			QScriptEngine::ScriptOwnership,
			QScriptEngine::ExcludeChildObjects |
				QScriptEngine::ExcludeSuperClassContents |
				QScriptEngine::ExcludeDeleteLater |
				QScriptEngine::SkipMethodsInEnumeration);
	}

	return DescendantToScriptValue<CLASS, DESC_T>(engine, in);
}

template <typename CLASS, typename DESC_T>
static QScriptValue ScriptObjectConstructorEx(
	QScriptEngine *engine, QScriptContext *context)
{
	DESC_T data;
	switch (context->argumentCount())
	{
		case 1:
		{
			auto argument = context->argument(0);
			if (argument.isQObject())
			{
				auto source = dynamic_cast<CLASS *>(argument.toQObject());
				if (nullptr == source)
					return ThrowIncompatibleArgumentType(context, 1);

				data = *source;
			} else if (argument.isObject())
			{
				auto result = ScriptObjectConstructor<CLASS, DESC_T>(
					engine, context, data);

				CopyScriptProperties(argument, result);
				return result;
			} else if (!argument.isNull() && !argument.isUndefined())
			{
				data = DESC_T(argument.toString());
			}

		} // fall through

		case 0:
			return ScriptObjectConstructor<CLASS, DESC_T>(
				engine, context, data);
	}

	return ThrowBadNumberOfArguments(context);
}

template <typename OUT_T, typename ARG_T, void (OUT_T::*setX)(ARG_T),
	void (OUT_T::*setY)(ARG_T)>
static void VecFromScriptValue(const QScriptValue &object, OUT_T &out,
	const QString &xKey, const QString &yKey)
{
	if (std::is_same<int, ARG_T>::value)
	{
		(out.*setX)(object.property(xKey).toInt32());
		(out.*setY)(object.property(yKey).toInt32());
	} else
	{
		(out.*setX)(object.property(xKey).toNumber());
		(out.*setY)(object.property(yKey).toNumber());
	}
}

template <typename CLASS, typename VEC_T, typename ARG_T,
	void (VEC_T::*setX)(ARG_T), void (VEC_T::*setY)(ARG_T)>
static QScriptValue VecConstructor(QScriptContext *context,
	QScriptEngine *engine, const QString &xKey, const QString &yKey)
{
	VEC_T vec;
	if (context->argumentCount() == 1)
	{
		VecFromScriptValue<VEC_T, ARG_T, setX, setY>(
			context->argument(0), vec, xKey, yKey);
	} else if (context->argumentCount() == 2)
	{
		if (std::is_same<int, ARG_T>::value)
		{
			(vec.*setX)(context->argument(0).toInt32());
			(vec.*setY)(context->argument(1).toInt32());
		} else
		{
			(vec.*setX)(context->argument(0).toNumber());
			(vec.*setY)(context->argument(1).toNumber());
		}
	} else if (context->argumentCount() != 0)
		return ThrowBadNumberOfArguments(context);

	return ScriptObjectConstructor<CLASS, VEC_T>(engine, context, vec);
}

template <typename T>
static QScriptValue QObjectToScriptValue(QScriptEngine *engine, T *const &in)
{
	return engine->newQObject(in);
}

template <typename T>
static void QObjectFromScriptValue(const QScriptValue &object, T *&out)
{
	out = qobject_cast<T *>(object.toQObject());
}
}
template <typename FLAGS_T>
static QScriptValue FlagsToScriptValue(QScriptEngine *engine, const FLAGS_T &in)
{
	return QScriptValue(engine, static_cast<qint32>(in.to_ulong()));
}

template <typename FLAGS_T>
static void FlagsFromScriptValue(const QScriptValue &object, FLAGS_T &out)
{
	out = FLAGS_T(object.toInt32());
}

template <typename STRING_T>
static QScriptValue StringToScriptValue(
	QScriptEngine *engine, const STRING_T &in)
{
	return QScriptValue(engine, QString(in));
}

template <typename STRING_T>
static void StringFromScriptValue(const QScriptValue &object, STRING_T &out)
{
	out = object.toString();
}
}
