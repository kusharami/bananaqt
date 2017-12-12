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

#include <QObject>
#include <QFont>
#include <QScriptEngine>

Q_DECLARE_METATYPE(QFont::Style)

namespace Banana
{
class ScriptQFont
	: public QObject
	, public QFont
{
	Q_OBJECT

	Q_PROPERTY(QString family READ family WRITE setFamily)
	Q_PROPERTY(QString styleName READ styleName WRITE setStyleName)
	Q_PROPERTY(qreal pointSize READ pointSizeF WRITE setPointSizeF)
	Q_PROPERTY(int pixelSize READ pixelSize WRITE setPixelSize)
	Q_PROPERTY(int weight READ weight WRITE setWeight)
	Q_PROPERTY(bool bold READ bold WRITE setBold)
	Q_PROPERTY(bool italic READ italic WRITE setItalic)
	Q_PROPERTY(bool underline READ underline WRITE setUnderline)
	Q_PROPERTY(bool overline READ overline WRITE setOverline)
	Q_PROPERTY(bool strikeOut READ strikeOut WRITE setStrikeOut)
	Q_PROPERTY(bool fixedPitch READ fixedPitch WRITE setFixedPitch)
	Q_PROPERTY(QFont::Style style READ style WRITE setStyle)
	Q_PROPERTY(bool kerning READ kerning WRITE setKerning)

public:
	static void Register(QScriptEngine *engine);

	ScriptQFont(const QFont &font);

	Q_INVOKABLE inline QString toString() const;

	Q_INVOKABLE bool fromString(const QString &str);

private:
	static QScriptValue Constructor(
		QScriptContext *context, QScriptEngine *engine);
	static QScriptValue ToScriptValue(QScriptEngine *engine, const QFont &in);
	static void FromScriptValue(const QScriptValue &object, QFont &out);
};

QString ScriptQFont::toString() const
{
	return QFont::toString();
}
}
