/*
 * MIT License
 *
 * Copyright (c) 2016 Alexandra Cherdantseva
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <QObject>

namespace Banana
{

	class UniqueNameScope : public QObject
	{
		Q_OBJECT

	public:
		explicit UniqueNameScope(const QMetaObject *meta_object,
								 Qt::CaseSensitivity sensitivity = Qt::CaseInsensitive,
								 QObject *parent = nullptr);

		const QMetaObject *getObjectType() const;

		Qt::CaseSensitivity getCaseSensitivity() const;
		void setCaseSensitivity(Qt::CaseSensitivity value);

		void connectObject(QObject *object);
		void disconnectObject(QObject *object);

	protected:
		virtual QObject *findSibling(QObject *object, const QString &find_name) const;
		virtual bool checkSiblingNameForObject(QObject *sibling, const QString &name, QObject *object) const;

		const QMetaObject *meta_object;
		Qt::CaseSensitivity sensitivity;

	private slots:
		void onObjectNameChanged(const QString &name);

	private:
		static QString getFixedName(const QString &name, QObject *object);
		static QString getNumberSeparator(QObject *object);
		static QString getNumberedFormat(QObject *object);

		void setObjectName(QObject *object, const QString &name);
		void internalConnectObject(QObject *object);
		void internalDisconnectObject(QObject *object);
	};

}
