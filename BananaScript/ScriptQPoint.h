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
#include <QPointF>
#include <QScriptEngine>
#include <QScriptable>

Q_DECLARE_METATYPE(QPointF *)

namespace Banana
{
class ScriptQPoint final
	: public QObject
	, public QScriptable
{
	Q_OBJECT

	Q_PROPERTY(qreal x READ x WRITE setX)
	Q_PROPERTY(qreal y READ y WRITE setY)
	Q_PROPERTY(qreal manhattanLength READ manhattanLength STORED false)
	Q_PROPERTY(bool isNull READ isNull STORED false)

public:
	explicit ScriptQPoint(QObject *parent);
	static void Register(QScriptEngine *engine);
	template <typename T>
	static inline void RegisterType(QScriptEngine *engine)
	{
		qScriptRegisterMetaType<T>(
			engine, ToScriptValueCustom<T>, FromScriptValueCustom<T>);
	}

	qreal x() const;
	void setX(qreal pos);

	qreal y() const;
	void setY(qreal pos);

	qreal manhattanLength() const;

	bool isNull() const;

public slots:
	bool equals(const QPointF &other) const;
	bool equals(qreal x, qreal y) const;
	QPointF sub(const QPointF &value) const;
	QPointF sub(qreal x, qreal y) const;
	QPointF add(const QPointF &value) const;
	QPointF add(qreal x, qreal y) const;
	QPointF mul(qreal value) const;
	QPointF div(qreal value) const;
	qreal dotProduct(const QPointF &to) const;
	qreal dotProduct(qreal x, qreal y) const;

	QString toString() const;
	QPoint toPoint() const;

	void assign(const QPointF &other);
	QPointF clone() const;

private:
	static QString className();
	QPointF *thisPoint() const;

	template <typename T>
	static QScriptValue ToScriptValueCustom(QScriptEngine *engine, const T &in)
	{
		return ToScriptValue(engine, in);
	}

	template <typename T>
	static void FromScriptValueCustom(const QScriptValue &object, T &out)
	{
		FromScriptValue(object, out);
	}

	static QScriptValue ToScriptValue(QScriptEngine *engine, const QPoint &in);
	static QScriptValue ToScriptValue(QScriptEngine *engine, const QPointF &in);
	static void FromScriptValue(const QScriptValue &object, QPoint &out);
	static void FromScriptValue(const QScriptValue &object, QPointF &out);

	static QScriptValue Constructor(
		QScriptContext *context, QScriptEngine *engine);
	static QScriptValue ConstructorF(
		QScriptContext *context, QScriptEngine *engine);
};
}
