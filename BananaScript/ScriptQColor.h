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
#include <QColor>
#include <QScriptValue>
#include <QScriptable>

Q_DECLARE_METATYPE(QColor *)

namespace Banana
{
class ScriptQColor final
	: public QObject
	, public QScriptable
{
	Q_OBJECT

	Q_PROPERTY(int alpha READ alpha WRITE setAlpha)
	Q_PROPERTY(qreal alphaF READ alphaF WRITE setAlphaF STORED false)

	Q_PROPERTY(int red READ red WRITE setRed)
	Q_PROPERTY(qreal redF READ redF WRITE setRedF STORED false)

	Q_PROPERTY(int green READ green WRITE setGreen)
	Q_PROPERTY(qreal greenF READ greenF WRITE setGreenF STORED false)

	Q_PROPERTY(int blue READ blue WRITE setBlue)
	Q_PROPERTY(qreal blueF READ blueF WRITE setBlueF STORED false)

	Q_PROPERTY(quint32 rgba READ rgba WRITE setRgba STORED false)
	Q_PROPERTY(quint32 rgb READ rgb WRITE setRgb STORED false)

public:
	explicit ScriptQColor(QObject *parent);
	static void Register(QScriptEngine *engine);

	int alpha() const;
	void setAlpha(int alpha);

	qreal alphaF() const;
	void setAlphaF(qreal alpha);

	int red() const;
	int green() const;
	int blue() const;
	void setRed(int red);
	void setGreen(int green);
	void setBlue(int blue);

	qreal redF() const;
	qreal greenF() const;
	qreal blueF() const;
	void setRedF(qreal red);
	void setGreenF(qreal green);
	void setBlueF(qreal blue);

	void setRgb(int r, int g, int b, int a = 255);
	void setRgbF(qreal r, qreal g, qreal b, qreal a = 1.0);

	quint32 rgba() const;
	void setRgba(quint32 rgba);

	quint32 rgb() const;
	void setRgb(quint32 rgb);

public slots:
	inline quint32 valueOf() const;
	QString toString() const;
	QString toStringWithAlpha() const;
	void setNamedColor(const QString &str);

	void assign(const QColor &other);
	QColor clone() const;
	bool equals(const QColor &other) const;
	bool equals(int r, int g, int b, int a = 255) const;

private:
	static QString className();
	QColor *thisColor() const;

	static QScriptValue isValidColor(
		QScriptContext *context, QScriptEngine *engine);

	static QScriptValue Constructor(
		QScriptContext *context, QScriptEngine *engine);
	static QScriptValue ToScriptValue(QScriptEngine *engine, const QColor &in);
	static void FromScriptValue(const QScriptValue &value, QColor &out);

	static bool ConstructWith(QColor &out, const QScriptValue &red,
		const QScriptValue &green, const QScriptValue &blue,
		const QScriptValue &alpha);
};

quint32 ScriptQColor::valueOf() const
{
	return rgba();
}
}
