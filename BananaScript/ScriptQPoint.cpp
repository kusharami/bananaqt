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

#include "ScriptQPoint.h"

#include "ScriptUtils.h"

#include "BananaCore/Const.h"

#include <QSizeF>

namespace Banana
{
using namespace Script;

void ScriptQPoint::Register(QScriptEngine *engine)
{
	qScriptRegisterMetaType<QPoint>(engine, ToScriptValue, FromScriptValue);
	qScriptRegisterMetaType<QPointF>(engine, ToScriptValue, FromScriptValue);

	auto proto = NewQObjectPrototype<ScriptQPoint>(engine);

	auto ctor = engine->newFunction(Constructor, proto);
	ctor.setData(proto);
	auto ctorF = engine->newFunction(ConstructorF, proto);
	ctorF.setData(proto);

	auto global = engine->globalObject();
	global.setProperty(className(), ctor, STATIC_SCRIPT_VALUE);
	global.setProperty(QSTRKEY(QPointF), ctorF, STATIC_SCRIPT_VALUE);
}

ScriptQPoint::ScriptQPoint(QObject *parent)
	: QObject(parent)
{
}

qreal ScriptQPoint::x() const
{
	auto point = thisPoint();
	if (point)
		return point->x();

	return qQNaN();
}

void ScriptQPoint::setX(qreal pos)
{
	auto point = thisPoint();
	if (point)
		point->setX(pos);
}

qreal ScriptQPoint::y() const
{
	auto point = thisPoint();
	if (point)
		return point->y();

	return qQNaN();
}

void ScriptQPoint::setY(qreal pos)
{
	auto point = thisPoint();
	if (point)
		point->setY(pos);
}

qreal ScriptQPoint::manhattanLength() const
{
	auto point = thisPoint();
	if (point)
		return point->manhattanLength();

	return qQNaN();
}

bool ScriptQPoint::isNull() const
{
	auto point = thisPoint();
	if (point)
		return point->isNull();

	return true;
}

bool ScriptQPoint::equals(const QPointF &other) const
{
	auto point = thisPoint();
	if (point)
		return *point == other;

	return false;
}

bool ScriptQPoint::equals(qreal x, qreal y) const
{
	auto point = thisPoint();
	if (point)
		return *point == QPointF(x, y);

	return false;
}

QPointF ScriptQPoint::sub(const QPointF &value) const
{
	auto point = thisPoint();
	if (point)
		return *point - value;

	return QPointF();
}

QPointF ScriptQPoint::sub(qreal x, qreal y) const
{
	auto point = thisPoint();
	if (point)
		return *point - QPointF(x, y);

	return QPointF();
}

QPointF ScriptQPoint::add(const QPointF &value) const
{
	auto point = thisPoint();
	if (point)
		return *point + value;

	return QPointF();
}

QPointF ScriptQPoint::add(qreal x, qreal y) const
{
	auto point = thisPoint();
	if (point)
		return *point + QPointF(x, y);

	return QPointF();
}

QPointF ScriptQPoint::mul(qreal value) const
{
	auto point = thisPoint();
	if (point)
		return *point * value;

	return QPointF();
}

QPointF ScriptQPoint::div(qreal value) const
{
	auto point = thisPoint();
	if (point)
		return *point / value;

	return QPointF();
}

qreal ScriptQPoint::dotProduct(const QPointF &to) const
{
	auto point = thisPoint();
	if (point)
		return QPointF::dotProduct(*point, to);

	return qQNaN();
}

qreal ScriptQPoint::dotProduct(qreal x, qreal y) const
{
	auto point = thisPoint();
	if (point)
		return QPointF::dotProduct(*point, QPointF(x, y));

	return qQNaN();
}

QString ScriptQPoint::toString() const
{
	auto point = thisPoint();
	if (point)
	{
		return QStringLiteral("%1(%2, %3)").arg(className()).arg(x()).arg(y());
	}

	return className();
}

QPoint ScriptQPoint::toPoint() const
{
	auto point = thisPoint();
	if (point)
		return point->toPoint();

	return QPoint();
}

void ScriptQPoint::assign(const QPointF &other)
{
	auto point = thisPoint();
	if (point)
		*point = other;
}

QPointF ScriptQPoint::clone() const
{
	auto point = thisPoint();
	if (point)
		return *point;

	return QPointF();
}

QString ScriptQPoint::className()
{
	return QSTRKEY(QPoint);
}

QPointF *ScriptQPoint::thisPoint() const
{
	return qscriptvalue_cast<QPointF *>(thisObject().data());
}

QScriptValue ScriptQPoint::ToScriptValue(
	QScriptEngine *engine, const QPoint &in)
{
	auto ctor = engine->globalObject().property(className());

	auto result = engine->newObject();
	result.setData(engine->newVariant(QPointF(in)));
	result.setPrototype(ctor.data());
	return result;
}

QScriptValue ScriptQPoint::ToScriptValue(
	QScriptEngine *engine, const QPointF &in)
{
	auto ctor = engine->globalObject().property(QSTRKEY(QPointF));

	auto result = engine->newObject();
	result.setData(engine->newVariant(in));
	result.setPrototype(ctor.data());
	return result;
}

void ScriptQPoint::FromScriptValue(const QScriptValue &object, QPoint &out)
{
	auto data = object.data();
	if (not data.isVariant())
		data = object;
	if (data.isVariant())
	{
		auto v = data.toVariant();
		switch (v.type())
		{
			case QVariant::Point:
				out = v.toPoint();
				return;

			case QVariant::PointF:
				out = v.toPointF().toPoint();
				return;

			case QVariant::Size:
			{
				auto size = v.toSize();
				out.setX(size.width());
				out.setY(size.height());
				return;
			}

			case QVariant::SizeF:
			{
				auto size = v.toSizeF().toSize();
				out.setX(size.width());
				out.setY(size.height());
				return;
			}

			default:
				break;
		}
	}

	out.setX(object.property(pX).toInt32());
	out.setY(object.property(pY).toInt32());
}

void ScriptQPoint::FromScriptValue(const QScriptValue &object, QPointF &out)
{
	auto data = object.data();
	if (not data.isVariant())
		data = object;
	if (data.isVariant())
	{
		auto v = data.toVariant();
		switch (v.type())
		{
			case QVariant::Point:
				out = v.toPoint();
				return;

			case QVariant::PointF:
				out = v.toPointF();
				return;

			case QVariant::Size:
			{
				auto size = v.toSize();
				out.setX(qreal(size.width()));
				out.setY(qreal(size.height()));
				return;
			}

			case QVariant::SizeF:
			{
				auto size = v.toSizeF();
				out.setX(size.width());
				out.setY(size.height());
				return;
			}

			default:
				break;
		}
	}

	out.setX(object.property(pX).toNumber());
	out.setY(object.property(pY).toNumber());
}

QScriptValue ScriptQPoint::Constructor(
	QScriptContext *context, QScriptEngine *engine)
{
	QPoint point;
	if (context->argumentCount() == 1)
	{
		FromScriptValue(context->argument(0), point);
	} else if (context->argumentCount() == 2)
	{
		point.setX(context->argument(0).toInt32());
		point.setY(context->argument(1).toInt32());
	} else if (context->argumentCount() != 0)
		return ThrowBadNumberOfArguments(context);

	return ToScriptValue(engine, point);
}

QScriptValue ScriptQPoint::ConstructorF(
	QScriptContext *context, QScriptEngine *engine)
{
	QPointF point;
	if (context->argumentCount() == 1)
	{
		FromScriptValue(context->argument(0), point);
	} else if (context->argumentCount() == 2)
	{
		point.setX(context->argument(0).toNumber());
		point.setY(context->argument(1).toNumber());
	} else if (context->argumentCount() != 0)
		return ThrowBadNumberOfArguments(context);

	return ToScriptValue(engine, point);
}
}
