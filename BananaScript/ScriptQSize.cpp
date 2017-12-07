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

#include "ScriptQSize.h"

#include "ScriptUtils.h"

#include "BananaCore/Const.h"

#include <QPointF>

namespace Banana
{
using namespace Script;

void ScriptQSize::Register(QScriptEngine *engine)
{
	static bool convertersRegistered = false;
	if (not convertersRegistered)
	{
		convertersRegistered = true;
		QMetaType::registerConverter<Qt::AspectRatioMode, qint32>();
	}

	qScriptRegisterMetaType(engine, EnumToScriptValue<Qt::AspectRatioMode>,
		EnumFromScriptValue<Qt::AspectRatioMode>);

	qScriptRegisterMetaType<QSize>(engine, ToScriptValue, FromScriptValue);
	qScriptRegisterMetaType<QSizeF>(engine, ToScriptValue, FromScriptValue);

	auto proto = NewQObjectPrototype<ScriptQSize>(engine);

	auto qSize = engine->newFunction(Constructor, proto);
	qSize.setData(proto);
	auto qSizeF = engine->newFunction(ConstructorF, proto);
	qSizeF.setData(proto);

	enum
	{
		IgnoreAspectRatio = Qt::IgnoreAspectRatio,
		KeepAspectRatio = Qt::KeepAspectRatio,
		KeepAspectRatioByExpanding = Qt::KeepAspectRatioByExpanding
	};

	SCRIPT_REG_ENUM(qSize, IgnoreAspectRatio);
	SCRIPT_REG_ENUM(qSize, KeepAspectRatio);
	SCRIPT_REG_ENUM(qSize, KeepAspectRatioByExpanding);

	SCRIPT_REG_ENUM(qSizeF, IgnoreAspectRatio);
	SCRIPT_REG_ENUM(qSizeF, KeepAspectRatio);
	SCRIPT_REG_ENUM(qSizeF, KeepAspectRatioByExpanding);

	auto global = engine->globalObject();
	global.setProperty(className(), qSize, STATIC_SCRIPT_VALUE);
	global.setProperty(QSTRKEY(QSizeF), qSizeF, STATIC_SCRIPT_VALUE);
}

ScriptQSize::ScriptQSize(QObject *parent)
	: QObject(parent)
{
}

qreal ScriptQSize::width() const
{
	auto size = thisSize();
	if (size)
		return size->width();

	return qQNaN();
}

void ScriptQSize::setWidth(qreal value)
{
	auto size = thisSize();
	if (size)
		size->setWidth(value);
}

qreal ScriptQSize::height() const
{
	auto size = thisSize();
	if (size)
		return size->height();

	return qQNaN();
}

void ScriptQSize::setHeight(qreal value)
{
	auto size = thisSize();
	if (size)
		size->setHeight(value);
}

bool ScriptQSize::equals(const QSizeF &other) const
{
	auto size = thisSize();
	if (size)
		return *size == other;

	return false;
}

bool ScriptQSize::equals(qreal width, qreal height) const
{
	auto size = thisSize();
	if (size)
		return *size == QSizeF(width, height);

	return false;
}

QSizeF ScriptQSize::sub(const QSizeF &value) const
{
	auto size = thisSize();
	if (size)
		return *size - value;

	return QSizeF();
}

QSizeF ScriptQSize::sub(qreal width, qreal height) const
{
	auto size = thisSize();
	if (size)
		return *size - QSizeF(width, height);

	return QSizeF();
}

QSizeF ScriptQSize::add(const QSizeF &value) const
{
	auto size = thisSize();
	if (size)
		return *size + value;

	return QSizeF();
}

QSizeF ScriptQSize::add(qreal width, qreal height) const
{
	auto size = thisSize();
	if (size)
		return *size + QSizeF(width, height);

	return QSizeF();
}

QSizeF ScriptQSize::mul(qreal value) const
{
	auto size = thisSize();
	if (size)
		return *size * value;

	return QSizeF();
}

QSizeF ScriptQSize::div(qreal value) const
{
	auto size = thisSize();
	if (size)
		return *size / value;

	return QSizeF();
}

QSizeF ScriptQSize::transposed() const
{
	auto size = thisSize();
	if (size)
		return size->transposed();

	return QSizeF();
}

void ScriptQSize::transpose()
{
	auto size = thisSize();
	if (size)
		size->transpose();
}

void ScriptQSize::scale(qreal w, qreal h, Qt::AspectRatioMode mode)
{
	auto size = thisSize();
	if (size)
		size->scale(w, h, mode);
}

void ScriptQSize::scale(const QSizeF &s, Qt::AspectRatioMode mode)
{
	auto size = thisSize();
	if (size)
		size->scale(s, mode);
}

QSizeF ScriptQSize::scaled(qreal w, qreal h, Qt::AspectRatioMode mode) const
{
	auto size = thisSize();
	if (size)
		return size->scaled(w, h, mode);

	return QSizeF();
}

QSizeF ScriptQSize::scaled(const QSizeF &s, Qt::AspectRatioMode mode) const
{
	auto size = thisSize();
	if (size)
		return size->scaled(s, mode);

	return QSizeF();
}

QSizeF ScriptQSize::expandedTo(const QSizeF &other) const
{
	auto size = thisSize();
	if (size)
		return size->expandedTo(other);

	return QSizeF();
}

QSizeF ScriptQSize::expandedTo(qreal w, qreal h) const
{
	auto size = thisSize();
	if (size)
		return size->expandedTo(QSizeF(w, h));

	return QSizeF();
}

QSizeF ScriptQSize::boundedTo(const QSizeF &other) const
{
	auto size = thisSize();
	if (size)
		return size->boundedTo(other);

	return QSizeF();
}

QSizeF ScriptQSize::boundedTo(qreal w, qreal h) const
{
	auto size = thisSize();
	if (size)
		return size->boundedTo(QSizeF(w, h));

	return QSizeF();
}

bool ScriptQSize::isNull() const
{
	auto size = thisSize();
	if (size)
		return size->isNull();

	return true;
}

bool ScriptQSize::isEmpty() const
{
	auto size = thisSize();
	if (size)
		return size->isEmpty();

	return true;
}

bool ScriptQSize::isValid() const
{
	auto size = thisSize();
	if (size)
		return size->isValid();

	return false;
}

QString ScriptQSize::toString() const
{
	auto size = thisSize();
	if (size)
	{
		return QStringLiteral("%1(%2 x %3)")
			.arg(className())
			.arg(size->width())
			.arg(size->height());
	}

	return className();
}

QSize ScriptQSize::toSize() const
{
	auto size = thisSize();
	if (size)
		return size->toSize();

	return QSize();
}

void ScriptQSize::assign(const QSizeF &other)
{
	auto size = thisSize();
	if (size)
		*size = other;
}

QSizeF ScriptQSize::clone() const
{
	auto size = thisSize();
	if (size)
		return *size;

	return QSizeF();
}

QString ScriptQSize::className()
{
	return QSTRKEY(QSize);
}

QSizeF *ScriptQSize::thisSize() const
{
	return qscriptvalue_cast<QSizeF *>(thisObject().data());
}

QScriptValue ScriptQSize::ToScriptValue(QScriptEngine *engine, const QSize &in)
{
	auto ctor = engine->globalObject().property(className());

	auto result = engine->newObject();
	result.setData(engine->newVariant(QSizeF(in)));
	result.setPrototype(ctor.data());
	return result;
}

QScriptValue ScriptQSize::ToScriptValue(QScriptEngine *engine, const QSizeF &in)
{
	auto ctor = engine->globalObject().property(QSTRKEY(QSizeF));

	auto result = engine->newObject();
	result.setData(engine->newVariant(in));
	result.setPrototype(ctor.data());
	return result;
}

void ScriptQSize::FromScriptValue(const QScriptValue &object, QSize &out)
{
	auto data = object.data();
	if (not data.isVariant())
		data = object;
	if (data.isVariant())
	{
		auto v = data.toVariant();
		switch (v.type())
		{
			case QVariant::Size:
				out = v.toSize();
				return;

			case QVariant::SizeF:
				out = v.toSizeF().toSize();
				return;

			case QVariant::Point:
			{
				auto point = v.toPoint();
				out.setWidth(point.x());
				out.setHeight(point.y());
				return;
			}

			case QVariant::PointF:
			{
				auto point = v.toPointF().toPoint();
				out.setWidth(point.x());
				out.setHeight(point.y());
				return;
			}

			default:
				break;
		}
	}

	out.setWidth(object.property(pWidth).toInt32());
	out.setHeight(object.property(pHeight).toInt32());
}

void ScriptQSize::FromScriptValue(const QScriptValue &object, QSizeF &out)
{
	auto data = object.data();
	if (not data.isVariant())
		data = object;
	if (data.isVariant())
	{
		auto v = data.toVariant();
		switch (v.type())
		{
			case QVariant::Size:
				out = v.toSize();
				return;

			case QVariant::SizeF:
				out = v.toSizeF();
				return;

			case QVariant::Point:
			{
				auto point = v.toPoint();
				out.setWidth(qreal(point.x()));
				out.setHeight(qreal(point.y()));
				return;
			}

			case QVariant::PointF:
			{
				auto point = v.toPointF();
				out.setWidth(point.x());
				out.setHeight(point.y());
				return;
			}

			default:
				break;
		}
	}

	out.setWidth(object.property(pWidth).toNumber());
	out.setHeight(object.property(pHeight).toNumber());
}

QScriptValue ScriptQSize::Constructor(
	QScriptContext *context, QScriptEngine *engine)
{
	QSize size;
	if (context->argumentCount() == 1)
	{
		FromScriptValue(context->argument(0), size);
	} else if (context->argumentCount() == 2)
	{
		size.setWidth(context->argument(0).toInt32());
		size.setHeight(context->argument(1).toInt32());
	} else if (context->argumentCount() != 0)
		return ThrowBadNumberOfArguments(context);

	return ToScriptValue(engine, size);
}

QScriptValue ScriptQSize::ConstructorF(
	QScriptContext *context, QScriptEngine *engine)
{
	QSizeF size;
	if (context->argumentCount() == 1)
	{
		FromScriptValue(context->argument(0), size);
	} else if (context->argumentCount() == 2)
	{
		size.setWidth(context->argument(0).toNumber());
		size.setHeight(context->argument(1).toNumber());
	} else if (context->argumentCount() != 0)
		return ThrowBadNumberOfArguments(context);

	return ToScriptValue(engine, size);
}
}
