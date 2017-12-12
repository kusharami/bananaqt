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

#include "ScriptQColor.h"

#include "ScriptUtils.h"

namespace Banana
{
using namespace Script;

void ScriptQColor::Register(QScriptEngine *engine)
{
	qScriptRegisterMetaType(engine, ToScriptValue, FromScriptValue);

	auto proto = Script::NewQObjectPrototype<ScriptQColor>(engine);

	auto qColor = engine->newFunction(ScriptQColor::Constructor, proto);
	qColor.setData(proto);
	qColor.setProperty(QSTRKEY(isValidColor),
		engine->newFunction(ScriptQColor::isValidColor), STATIC_SCRIPT_VALUE);

	engine->globalObject().setProperty(
		className(), qColor, STATIC_SCRIPT_VALUE);
}

ScriptQColor::ScriptQColor(QObject *parent)
	: QObject(parent)
{
}

int ScriptQColor::alpha() const
{
	auto c = thisColor();
	if (c)
		return c->alpha();

	return -1;
}

void ScriptQColor::setAlpha(int alpha)
{
	auto c = thisColor();
	if (c)
		c->setAlpha(alpha);
}

qreal ScriptQColor::alphaF() const
{
	auto c = thisColor();
	if (c)
		return c->alphaF();

	return qQNaN();
}

void ScriptQColor::setAlphaF(qreal alpha)
{
	auto c = thisColor();
	if (c)
		c->setAlphaF(alpha);
}

int ScriptQColor::red() const
{
	auto c = thisColor();
	if (c)
		return c->red();

	return -1;
}

int ScriptQColor::green() const
{
	auto c = thisColor();
	if (c)
		return c->green();

	return -1;
}

int ScriptQColor::blue() const
{
	auto c = thisColor();
	if (c)
		return c->blue();

	return -1;
}

void ScriptQColor::setRed(int red)
{
	auto c = thisColor();
	if (c)
		c->setRed(red);
}

void ScriptQColor::setGreen(int green)
{
	auto c = thisColor();
	if (c)
		c->setGreen(green);
}

void ScriptQColor::setBlue(int blue)
{
	auto c = thisColor();
	if (c)
		c->setBlue(blue);
}

qreal ScriptQColor::redF() const
{
	auto c = thisColor();
	if (c)
		return c->redF();

	return qQNaN();
}

qreal ScriptQColor::greenF() const
{
	auto c = thisColor();
	if (c)
		return c->greenF();

	return qQNaN();
}

qreal ScriptQColor::blueF() const
{
	auto c = thisColor();
	if (c)
		return c->blueF();

	return qQNaN();
}

void ScriptQColor::setRedF(qreal red)
{
	auto c = thisColor();
	if (c)
		c->setRedF(red);
}

void ScriptQColor::setGreenF(qreal green)
{
	auto c = thisColor();
	if (c)
		c->setGreenF(green);
}

void ScriptQColor::setBlueF(qreal blue)
{
	auto c = thisColor();
	if (c)
		c->setBlueF(blue);
}

void ScriptQColor::setRgb(int r, int g, int b, int a)
{
	auto c = thisColor();
	if (c)
		c->setRgb(r, g, b, a);
}

void ScriptQColor::setRgbF(qreal r, qreal g, qreal b, qreal a)
{
	auto c = thisColor();
	if (c)
		c->setRgbF(r, g, b, a);
}

quint32 ScriptQColor::rgba() const
{
	auto c = thisColor();
	if (c)
		return c->rgba();

	return 0;
}

void ScriptQColor::setRgba(quint32 rgba)
{
	auto c = thisColor();
	if (c)
		c->setRgba(rgba);
}

quint32 ScriptQColor::rgb() const
{
	auto c = thisColor();
	if (c)
		return c->rgb();

	return 0;
}

void ScriptQColor::setRgb(quint32 rgb)
{
	auto c = thisColor();
	if (c)
		c->setRgb(rgb);
}

QString ScriptQColor::toString() const
{
	auto c = thisColor();
	if (c)
		return c->name();

	return className();
}

QScriptValue ScriptQColor::Constructor(
	QScriptContext *context, QScriptEngine *engine)
{
	QColor color;
	int argc = context->argumentCount();
	switch (argc)
	{
		case 0:
			break;

		case 1:
			FromScriptValue(context->argument(0), color);
			break;

		case 3:
		case 4:
		{
			ConstructWith(color, context->argument(0), context->argument(1),
				context->argument(2),
				argc == 4 ? context->argument(3) : QScriptValue());
			break;
		}

		default:
			return ThrowBadNumberOfArguments(context);
	}

	return ToScriptValue(engine, color);
}

QScriptValue ScriptQColor::ToScriptValue(
	QScriptEngine *engine, const QColor &in)
{
	auto ctor = engine->globalObject().property(className());

	auto result = engine->newObject();
	result.setData(
		engine->newVariant(in.isValid() ? in.toRgb() : QColor(Qt::black)));
	result.setPrototype(ctor.data());
	return result;
}

void ScriptQColor::FromScriptValue(const QScriptValue &value, QColor &out)
{
	auto data = value.data();
	if (not data.isVariant())
		data = value;
	if (data.isVariant())
	{
		auto v = data.toVariant();
		if (v.canConvert<QColor>())
		{
			out = v.value<QColor>();
			if (not out.isValid())
				out.setRgb(0, 0, 0);
			else
				out = out.toRgb();
			return;
		}

		switch (v.type())
		{
			case QVariant::Int:
			case QVariant::UInt:
				out.setRgba(v.toUInt());
				return;

			case QVariant::LongLong:
			case QVariant::ULongLong:
				out.setRgba64(QRgba64::fromRgba64(v.toULongLong()));
				return;

			default:
				break;
		}
	}

	if (value.isObject())
	{
		if (not ConstructWith(out, value.property(QSTRKEY(red)),
				value.property(QSTRKEY(green)), value.property(QSTRKEY(blue)),
				value.property(QSTRKEY(alpha))))
		{
			ConstructWith(out, value.property(QSTRKEY(r)),
				value.property(QSTRKEY(g)), value.property(QSTRKEY(b)),
				value.property(QSTRKEY(a)));
		}
	} else
	{
		QString str;
		if (value.isString())
		{
			str = value.toString();
		} else if (value.isVariant())
		{
			auto var = value.toVariant();
			if (var.type() == QVariant::String)
				str = var.toString();
		}

		if (not str.isEmpty() && QColor::isValidColor(str))
		{
			out.setNamedColor(str);
			return;
		}

		bool ok = false;
		quint32 qRgba = 0;

		if (value.isNumber())
		{
			qRgba = value.toUInt32();
			ok = true;
		} else if (value.isString())
		{
			qRgba = value.toString().toUInt(&ok);
		} else if (value.isVariant())
		{
			qRgba = value.toVariant().toUInt(&ok);
		}

		if (ok)
		{
			out.setRgba(static_cast<QRgb>(qRgba));
		}
	}
}

bool ScriptQColor::ConstructWith(QColor &out, const QScriptValue &red,
	const QScriptValue &green, const QScriptValue &blue,
	const QScriptValue &alpha)
{
	if (red.isValid() && green.isValid() && blue.isValid())
	{
		out.setRed(red.toInt32());
		out.setGreen(green.toInt32());
		out.setBlue(blue.toInt32());
		out.setAlpha(alpha.isValid() ? alpha.toInt32() : 255);

		return true;
	}

	return false;
}

QString ScriptQColor::toStringWithAlpha() const
{
	auto c = thisColor();
	if (c)
		return c->name(QColor::HexArgb);

	return QString();
}

void ScriptQColor::setNamedColor(const QString &str)
{
	auto c = thisColor();
	if (c)
		c->setNamedColor(str);
}

void ScriptQColor::assign(const QColor &other)
{
	auto c = thisColor();
	if (c)
		*c = other;
}

QColor ScriptQColor::clone() const
{
	auto c = thisColor();
	if (c)
		return *c;

	return QColor();
}

bool ScriptQColor::equals(const QColor &other) const
{
	auto c = thisColor();
	if (c)
		return *c == other;

	return false;
}

bool ScriptQColor::equals(int r, int g, int b, int a) const
{
	auto c = thisColor();
	if (c)
		return *c == QColor(r, g, b, a);

	return false;
}

QString ScriptQColor::className()
{
	return QSTRKEY(QColor);
}

QColor *ScriptQColor::thisColor() const
{
	return qscriptvalue_cast<QColor *>(thisObject().data());
}

QScriptValue ScriptQColor::isValidColor(
	QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 1)
		return ThrowBadNumberOfArguments(context);

	return QScriptValue(
		engine, QColor::isValidColor(context->argument(0).toString()));
}
}
