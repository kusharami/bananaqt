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
#include <QSizeF>
#include <QScriptEngine>
#include <QScriptable>

Q_DECLARE_METATYPE(QSizeF *)
Q_DECLARE_METATYPE(Qt::AspectRatioMode)

namespace Banana
{
class ScriptQSize final
	: public QObject
	, public QScriptable
{
	Q_OBJECT

	Q_PROPERTY(qreal width READ width WRITE setWidth)
	Q_PROPERTY(qreal height READ height WRITE setHeight)

	Q_PROPERTY(bool isNull READ isNull STORED false)
	Q_PROPERTY(bool empty READ isEmpty STORED false)
	Q_PROPERTY(bool valid READ isValid STORED false)

public:
	explicit ScriptQSize(QObject *parent);
	static void Register(QScriptEngine *engine);

	qreal width() const;
	void setWidth(qreal value);

	qreal height() const;
	void setHeight(qreal value);

	bool isNull() const;
	bool isEmpty() const;
	bool isValid() const;

public slots:
	bool equals(const QSizeF &other) const;
	bool equals(qreal width, qreal height) const;
	QSizeF sub(const QSizeF &value) const;
	QSizeF sub(qreal width, qreal height) const;
	QSizeF add(const QSizeF &value) const;
	QSizeF add(qreal width, qreal height) const;
	QSizeF mul(qreal value) const;
	QSizeF div(qreal value) const;
	QSizeF transposed() const;
	void transpose();

	void scale(qreal w, qreal h, Qt::AspectRatioMode mode);
	void scale(const QSizeF &s, Qt::AspectRatioMode mode);
	QSizeF scaled(qreal w, qreal h, Qt::AspectRatioMode mode) const;
	QSizeF scaled(const QSizeF &s, Qt::AspectRatioMode mode) const;

	QSizeF expandedTo(const QSizeF &other) const;
	QSizeF expandedTo(qreal w, qreal h) const;
	QSizeF boundedTo(const QSizeF &other) const;
	QSizeF boundedTo(qreal w, qreal h) const;

	QString toString() const;
	QSize toSize() const;

	void assign(const QSizeF &other);
	QSizeF clone() const;

private:
	static QString className();
	QSizeF *thisSize() const;

	static QScriptValue ToScriptValue(QScriptEngine *engine, const QSize &in);
	static QScriptValue ToScriptValue(QScriptEngine *engine, const QSizeF &in);
	static void FromScriptValue(const QScriptValue &object, QSize &out);
	static void FromScriptValue(const QScriptValue &object, QSizeF &out);

	static QScriptValue Constructor(
		QScriptContext *context, QScriptEngine *engine);
	static QScriptValue ConstructorF(
		QScriptContext *context, QScriptEngine *engine);
};
}
