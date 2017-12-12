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
#include <QRectF>
#include <QScriptValue>
#include <QScriptable>

Q_DECLARE_METATYPE(QRectF *)

namespace Banana
{
class ScriptQRect final
	: public QObject
	, public QScriptable
{
	Q_OBJECT

	Q_PROPERTY(qreal x READ left WRITE setLeft)
	Q_PROPERTY(qreal y READ top WRITE setTop)
	Q_PROPERTY(qreal width READ width WRITE setWidth)
	Q_PROPERTY(qreal height READ height WRITE setHeight)

	Q_PROPERTY(qreal left READ left WRITE setLeft STORED false)
	Q_PROPERTY(qreal top READ top WRITE setTop STORED false)
	Q_PROPERTY(qreal right READ right WRITE setRight STORED false)
	Q_PROPERTY(qreal bottom READ bottom WRITE setBottom STORED false)

	Q_PROPERTY(QPointF topLeft READ topLeft WRITE setTopLeft STORED false)
	Q_PROPERTY(
		QPointF bottomRight READ bottomRight WRITE setBottomRight STORED false)
	Q_PROPERTY(QPointF topRight READ topRight WRITE setTopRight STORED false)
	Q_PROPERTY(
		QPointF bottomLeft READ bottomLeft WRITE setBottomLeft STORED false)
	Q_PROPERTY(QPointF center READ center WRITE moveCenter STORED false)

	Q_PROPERTY(bool isNull READ isNull STORED false)
	Q_PROPERTY(bool empty READ isEmpty STORED false)
	Q_PROPERTY(bool valid READ isValid STORED false)

	Q_PROPERTY(QSizeF size READ size WRITE setSize STORED false)

public:
	explicit ScriptQRect(QObject *parent);
	static void Register(QScriptEngine *engine);

	bool isNull() const;
	bool isEmpty() const;
	bool isValid() const;

	qreal left() const;
	void setLeft(qreal pos);

	qreal top() const;
	void setTop(qreal pos);

	qreal right() const;
	void setRight(qreal pos);

	qreal bottom() const;
	void setBottom(qreal pos);

	QPointF topLeft() const;
	void setTopLeft(const QPointF &p);

	QPointF bottomRight() const;
	void setBottomRight(const QPointF &p);

	QPointF topRight() const;
	void setTopRight(const QPointF &p);

	QPointF bottomLeft() const;
	void setBottomLeft(const QPointF &p);

	QPointF center() const;
	void moveCenter(const QPointF &p);

	QSizeF size() const;
	void setSize(const QSizeF &s);

	qreal width() const;
	void setWidth(qreal w);

	qreal height() const;
	void setHeight(qreal h);

public slots:
	QRectF normalized() const;

	void moveLeft(qreal pos);
	void moveTop(qreal pos);
	void moveRight(qreal pos);
	void moveBottom(qreal pos);
	void moveTopLeft(const QPointF &p);
	void moveBottomRight(const QPointF &p);
	void moveTopRight(const QPointF &p);
	void moveBottomLeft(const QPointF &p);

	void translate(qreal dx, qreal dy);
	inline void translate(const QPointF &p);

	QRectF translated(qreal dx, qreal dy) const;
	inline QRectF translated(const QPointF &p) const;

	QRectF transposed() const;

	void moveTo(qreal x, qreal y);
	inline void moveTo(const QPointF &p);

	void assign(const QRectF &other);
	QRectF clone() const;
	void setRect(qreal x, qreal y, qreal w, qreal h);
	void setCoords(qreal x1, qreal y1, qreal x2, qreal y2);

	void adjust(qreal x1, qreal y1, qreal x2, qreal y2);
	QRectF adjusted(qreal x1, qreal y1, qreal x2, qreal y2) const;

	bool containsRect(const QRectF &r) const;
	bool containsRect(qreal x, qreal y, qreal width, qreal height) const;
	bool containsPoint(const QPointF &p) const;
	bool containsPoint(qreal x, qreal y) const;
	QRectF united(const QRectF &other) const;
	QRectF united(qreal x, qreal y, qreal width, qreal height) const;
	QRectF intersected(const QRectF &other) const;
	QRectF intersected(qreal x, qreal y, qreal width, qreal height) const;
	bool intersects(const QRectF &r) const;
	bool intersects(qreal x, qreal y, qreal width, qreal height) const;

	QRect toRect() const;
	QRect toAlignedRect() const;

	bool equals(const QRectF &other) const;
	bool equals(qreal x, qreal y, qreal width, qreal height) const;

	QString toString() const;

private:
	static QString className();
	QRectF *thisRect() const;

	static QScriptValue toScriptValue(QScriptEngine *engine, const QRect &rect);
	static QScriptValue toScriptValue(
		QScriptEngine *engine, const QRectF &rect);
	static void fromScriptValue(const QScriptValue &object, QRect &out);
	static void fromScriptValue(const QScriptValue &object, QRectF &out);

	static QScriptValue Constructor(
		QScriptContext *context, QScriptEngine *engine);
	static QScriptValue ConstructorF(
		QScriptContext *context, QScriptEngine *engine);
};

void ScriptQRect::translate(const QPointF &p)
{
	translate(p.x(), p.y());
}

QRectF ScriptQRect::translated(const QPointF &p) const
{
	return translated(p.x(), p.y());
}

void ScriptQRect::moveTo(const QPointF &p)
{
	moveTo(p.x(), p.y());
}
}
