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

#include "ScriptQFont.h"

#include "ScriptUtils.h"

#include "BananaCore/Const.h"

namespace Banana
{
using namespace Script;

void ScriptQFont::Register(QScriptEngine *engine)
{
	qScriptRegisterMetaType(engine, EnumToScriptValue<QFont::Style>,
		EnumFromScriptValue<QFont::Style>);

	static bool CONV_REG = false;
	if (!CONV_REG)
	{
		CONV_REG = true;
		QMetaType::registerConverter<QFont::Style, qint32>();
	}

	qScriptRegisterMetaType(
		engine, ScriptQFont::ToScriptValue, ScriptQFont::FromScriptValue);

	auto qFontObject = engine->newFunction(ScriptQFont::Constructor);

	SCRIPT_REG_ENUM(qFontObject, StyleNormal);
	SCRIPT_REG_ENUM(qFontObject, StyleItalic);
	SCRIPT_REG_ENUM(qFontObject, StyleOblique);

	engine->globalObject().setProperty(
		QSTRKEY(QFont), qFontObject, STATIC_SCRIPT_VALUE);
}

ScriptQFont::ScriptQFont(const QFont &font)
	: QFont(font)
{
}

bool ScriptQFont::fromString(const QString &str)
{
	bool result = QFont::fromString(str);

	if (result)
		setStyleStrategy(PreferAntialias);

	return result;
}

QScriptValue ScriptQFont::Constructor(
	QScriptContext *context, QScriptEngine *engine)
{
	return ScriptObjectConstructorEx<ScriptQFont, QFont>(engine, context);
}

QScriptValue ScriptQFont::ToScriptValue(QScriptEngine *engine, const QFont &in)
{
	return DescendantToScriptValue<ScriptQFont, QFont>(engine, in);
}

void ScriptQFont::FromScriptValue(const QScriptValue &object, QFont &out)
{
	auto font = dynamic_cast<ScriptQFont *>(object.toQObject());

	if (nullptr != font)
		out = *font;
	else
	{
		out = getDefaultFont();
	}
}
}
