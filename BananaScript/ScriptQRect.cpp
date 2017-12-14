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

#include "ScriptQRect.h"

#include "ScriptUtils.h"

#include "BananaCore/Const.h"

namespace Banana
{
using namespace Script;

ScriptQRect::ScriptQRect(QObject *parent)
	: QObject(parent)
{
}

void ScriptQRect::Register(QScriptEngine *engine)
{
	qScriptRegisterMetaType<QRect>(engine, toScriptValue, fromScriptValue);
	qScriptRegisterMetaType<QRectF>(engine, toScriptValue, fromScriptValue);

	auto proto = NewQObjectPrototype<ScriptQRect>(engine);
	auto ctor = engine->newFunction(Constructor, proto);
	ctor.setData(proto);
	auto ctorF = engine->newFunction(ConstructorF, proto);
	ctorF.setData(proto);

	auto global = engine->globalObject();
	global.setProperty(className(), ctor, STATIC_SCRIPT_VALUE);
	global.setProperty(QSTRKEY(QRectF), ctorF, STATIC_SCRIPT_VALUE);
}

QRectF ScriptQRect::normalized() const
{
	auto rect = thisRect();
	if (rect)
		return rect->normalized();

	return QRectF();
}

void ScriptQRect::moveLeft(qreal pos)
{
	auto rect = thisRect();
	if (rect)
		rect->moveLeft(pos);
}

void ScriptQRect::moveTop(qreal pos)
{
	auto rect = thisRect();
	if (rect)
		rect->moveTop(pos);
}

void ScriptQRect::moveRight(qreal pos)
{
	auto rect = thisRect();
	if (rect)
		rect->moveRight(pos);
}

void ScriptQRect::moveBottom(qreal pos)
{
	auto rect = thisRect();
	if (rect)
		rect->moveBottom(pos);
}

void ScriptQRect::moveTopLeft(const QPointF &p)
{
	auto rect = thisRect();
	if (rect)
		rect->moveTopLeft(p);
}

void ScriptQRect::moveBottomRight(const QPointF &p)
{
	auto rect = thisRect();
	if (rect)
		rect->moveBottomRight(p);
}

void ScriptQRect::moveTopRight(const QPointF &p)
{
	auto rect = thisRect();
	if (rect)
		rect->moveTopRight(p);
}

void ScriptQRect::moveBottomLeft(const QPointF &p)
{
	auto rect = thisRect();
	if (rect)
		rect->moveBottomLeft(p);
}

void ScriptQRect::translate(qreal dx, qreal dy)
{
	auto rect = thisRect();
	if (rect)
		rect->translate(dx, dy);
}

QRectF ScriptQRect::translated(qreal dx, qreal dy) const
{
	auto rect = thisRect();
	if (rect)
		return rect->translated(dx, dy);

	return QRectF();
}

QRectF ScriptQRect::transposed() const
{
	auto rect = thisRect();
	if (rect)
		return rect->transposed();

	return QRectF();
}

void ScriptQRect::moveTo(qreal x, qreal y)
{
	auto rect = thisRect();
	if (rect)
		rect->moveTo(x, y);
}

void ScriptQRect::assign(const QRectF &other)
{
	auto rect = thisRect();
	if (rect)
		*rect = other;
}

QRectF ScriptQRect::clone() const
{
	auto rect = thisRect();
	if (rect)
		return *rect;

	return QRectF();
}

void ScriptQRect::setRect(qreal x, qreal y, qreal w, qreal h)
{
	auto rect = thisRect();
	if (rect)
		rect->setRect(x, y, w, h);
}

void ScriptQRect::setCoords(qreal x1, qreal y1, qreal x2, qreal y2)
{
	auto rect = thisRect();
	if (rect)
		rect->setCoords(x1, y1, x2, y2);
}

void ScriptQRect::adjust(qreal x1, qreal y1, qreal x2, qreal y2)
{
	auto rect = thisRect();
	if (rect)
		rect->adjust(x1, y1, x2, y2);
}

QRectF ScriptQRect::adjusted(qreal x1, qreal y1, qreal x2, qreal y2) const
{
	auto rect = thisRect();
	if (rect)
		return rect->adjusted(x1, y1, x2, y2);

	return QRectF();
}

bool ScriptQRect::containsRect(const QRectF &r) const
{
	auto rect = thisRect();
	if (rect)
		return rect->contains(r);

	return false;
}

bool ScriptQRect::containsPoint(const QPointF &p) const
{
	auto rect = thisRect();
	if (rect)
		return rect->contains(p);

	return false;
}

bool ScriptQRect::containsRect(
	qreal x, qreal y, qreal width, qreal height) const
{
	auto rect = thisRect();
	if (rect)
		return rect->contains(QRectF(x, y, width, height));

	return false;
}

bool ScriptQRect::containsPoint(qreal x, qreal y) const
{
	auto rect = thisRect();
	if (rect)
		return rect->contains(x, y);

	return false;
}

QRectF ScriptQRect::united(const QRectF &other) const
{
	auto rect = thisRect();
	if (rect)
		return rect->united(other);

	return QRectF();
}

QRectF ScriptQRect::united(qreal x, qreal y, qreal width, qreal height) const
{
	auto rect = thisRect();
	if (rect)
		return rect->united(QRectF(x, y, width, height));

	return QRectF();
}

QRectF ScriptQRect::intersected(const QRectF &other) const
{
	auto rect = thisRect();
	if (rect)
		return rect->intersected(other);

	return QRectF();
}

QRectF ScriptQRect::intersected(
	qreal x, qreal y, qreal width, qreal height) const
{
	auto rect = thisRect();
	if (rect)
		return rect->intersected(QRectF(x, y, width, height));

	return QRectF();
}

bool ScriptQRect::intersects(const QRectF &r) const
{
	auto rect = thisRect();
	if (rect)
		return rect->intersects(r);

	return false;
}

bool ScriptQRect::intersects(qreal x, qreal y, qreal width, qreal height) const
{
	auto rect = thisRect();
	if (rect)
		return rect->intersects(QRectF(x, y, width, height));

	return false;
}

QRect ScriptQRect::toRect() const
{
	auto rect = thisRect();
	if (rect)
		return rect->toRect();

	return QRect();
}

QRect ScriptQRect::toAlignedRect() const
{
	auto rect = thisRect();
	if (rect)
		return rect->toAlignedRect();

	return QRect();
}

bool ScriptQRect::equals(const QRectF &other) const
{
	auto rect = thisRect();
	if (rect)
		return *rect == other;

	return false;
}

bool ScriptQRect::equals(qreal x, qreal y, qreal width, qreal height) const
{
	auto rect = thisRect();
	if (rect)
		return *rect == QRectF(x, y, width, height);

	return false;
}

QRectF *ScriptQRect::thisRect() const
{
	return qscriptvalue_cast<QRectF *>(thisObject().data());
}

bool ScriptQRect::isNull() const
{
	auto rect = thisRect();
	if (rect)
		return rect->isNull();

	return true;
}

bool ScriptQRect::isEmpty() const
{
	auto rect = thisRect();
	if (rect)
		return rect->isEmpty();

	return true;
}

bool ScriptQRect::isValid() const
{
	auto rect = thisRect();
	if (rect)
		return rect->isValid();

	return false;
}

qreal ScriptQRect::left() const
{
	auto rect = thisRect();
	if (rect)
		return rect->left();

	return qQNaN();
}

void ScriptQRect::setLeft(qreal pos)
{
	auto rect = thisRect();
	if (rect)
		rect->setLeft(pos);
}

qreal ScriptQRect::top() const
{
	auto rect = thisRect();
	if (rect)
		return rect->top();

	return qQNaN();
}

void ScriptQRect::setTop(qreal pos)
{
	auto rect = thisRect();
	if (rect)
		rect->setTop(pos);
}

qreal ScriptQRect::right() const
{
	auto rect = thisRect();
	if (rect)
		return rect->right();

	return qQNaN();
}

void ScriptQRect::setRight(qreal pos)
{
	auto rect = thisRect();
	if (rect)
		rect->setRight(pos);
}

qreal ScriptQRect::bottom() const
{
	auto rect = thisRect();
	if (rect)
		return rect->bottom();

	return qQNaN();
}

void ScriptQRect::setBottom(qreal pos)
{
	auto rect = thisRect();
	if (rect)
		rect->setBottom(pos);
}

QPointF ScriptQRect::topLeft() const
{
	auto rect = thisRect();
	if (rect)
		return rect->topLeft();

	return QPointF();
}

void ScriptQRect::setTopLeft(const QPointF &p)
{
	auto rect = thisRect();
	if (rect)
		rect->setTopLeft(p);
}

QPointF ScriptQRect::bottomRight() const
{
	auto rect = thisRect();
	if (rect)
		return rect->bottomRight();

	return QPointF();
}

void ScriptQRect::setBottomRight(const QPointF &p)
{
	auto rect = thisRect();
	if (rect)
		rect->setBottomRight(p);
}

QPointF ScriptQRect::topRight() const
{
	auto rect = thisRect();
	if (rect)
		return rect->topRight();

	return QPointF();
}

void ScriptQRect::setTopRight(const QPointF &p)
{
	auto rect = thisRect();
	if (rect)
		rect->setTopRight(p);
}

QPointF ScriptQRect::bottomLeft() const
{
	auto rect = thisRect();
	if (rect)
		return rect->bottomLeft();

	return QPointF();
}

void ScriptQRect::setBottomLeft(const QPointF &p)
{
	auto rect = thisRect();
	if (rect)
		rect->setBottomLeft(p);
}

QPointF ScriptQRect::center() const
{
	auto rect = thisRect();
	if (rect)
		return rect->center();

	return QPointF();
}

void ScriptQRect::moveCenter(const QPointF &p)
{
	auto rect = thisRect();
	if (rect)
		rect->moveCenter(p);
}

QSizeF ScriptQRect::size() const
{
	auto rect = thisRect();
	if (rect)
		return rect->size();

	return QSizeF();
}

void ScriptQRect::setSize(const QSizeF &s)
{
	auto rect = thisRect();
	if (rect)
		rect->setSize(s);
}

qreal ScriptQRect::width() const
{
	auto rect = thisRect();
	if (rect)
		return rect->width();

	return qQNaN();
}

void ScriptQRect::setWidth(qreal w)
{
	auto rect = thisRect();
	if (rect)
		rect->setWidth(w);
}

qreal ScriptQRect::height() const
{
	auto rect = thisRect();
	if (rect)
		return rect->height();

	return qQNaN();
}

void ScriptQRect::setHeight(qreal h)
{
	auto rect = thisRect();
	if (rect)
		rect->setHeight(h);
}

QString ScriptQRect::toString() const
{
	auto rect = thisRect();
	if (rect)
	{
		return QStringLiteral("%1(%2, %3, %4 x %5)")
			.arg(className())
			.arg(rect->x())
			.arg(rect->y())
			.arg(rect->width())
			.arg(rect->height());
	}

	return className();
}

QString ScriptQRect::className()
{
	return QSTRKEY(QRect);
}

QScriptValue ScriptQRect::toScriptValue(
	QScriptEngine *engine, const QRect &rect)
{
	auto ctor = engine->globalObject().property(className());

	auto result = engine->newObject();
	result.setData(engine->newVariant(QRectF(rect)));
	result.setPrototype(ctor.data());
	return result;
}

QScriptValue ScriptQRect::toScriptValue(
	QScriptEngine *engine, const QRectF &rect)
{
	auto ctor = engine->globalObject().property(QSTRKEY(QRectF));

	auto result = engine->newObject();
	result.setData(engine->newVariant(rect));
	result.setPrototype(ctor.data());
	return result;
}

void ScriptQRect::fromScriptValue(const QScriptValue &object, QRect &out)
{
	auto data = object.data();
	if (not data.isVariant())
		data = object;
	if (data.isVariant())
	{
		auto v = data.toVariant();
		switch (v.type())
		{
			case QVariant::Rect:
				out = v.toRect();
				return;

			case QVariant::RectF:
				out = v.toRectF().toRect();
				return;

			default:
				break;
		}
	}

	out.setX(object.property(pX).toInt32());
	out.setY(object.property(pY).toInt32());
	out.setWidth(object.property(pWidth).toInt32());
	out.setHeight(object.property(pHeight).toInt32());
}

void ScriptQRect::fromScriptValue(const QScriptValue &object, QRectF &out)
{
	auto data = object.data();
	if (not data.isVariant())
		data = object;
	if (data.isVariant())
	{
		auto v = data.toVariant();
		switch (v.type())
		{
			case QVariant::Rect:
				out = v.toRect();
				return;

			case QVariant::RectF:
				out = v.toRectF();
				return;

			default:
				break;
		}
	}

	out.setX(object.property(pX).toNumber());
	out.setY(object.property(pY).toNumber());
	out.setWidth(object.property(pWidth).toNumber());
	out.setHeight(object.property(pHeight).toNumber());
}

QScriptValue ScriptQRect::Constructor(
	QScriptContext *context, QScriptEngine *engine)
{
	QRect rect;
	if (context->argumentCount() == 1)
	{
		fromScriptValue(context->argument(0), rect);
	} else if (context->argumentCount() == 4)
	{
		rect.setX(context->argument(0).toInt32());
		rect.setY(context->argument(1).toInt32());
		rect.setWidth(context->argument(2).toInt32());
		rect.setHeight(context->argument(3).toInt32());
	} else if (context->argumentCount() != 0)
		return ThrowBadNumberOfArguments(context);

	return toScriptValue(engine, rect);
}

QScriptValue ScriptQRect::ConstructorF(
	QScriptContext *context, QScriptEngine *engine)
{
	QRectF rect;
	if (context->argumentCount() == 1)
	{
		fromScriptValue(context->argument(0), rect);
	} else if (context->argumentCount() == 4)
	{
		rect.setX(context->argument(0).toNumber());
		rect.setY(context->argument(1).toNumber());
		rect.setWidth(context->argument(2).toNumber());
		rect.setHeight(context->argument(3).toNumber());
	} else if (context->argumentCount() != 0)
		return ThrowBadNumberOfArguments(context);

	return toScriptValue(engine, rect);
}
}
