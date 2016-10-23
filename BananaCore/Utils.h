/*
 * Banana Qt Libraries
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

#include <QString>
#include <QVariant>
#include <QJsonValue>
#include <QFont>

#ifdef QT_DEBUG
#include <QDebug>
#	define LOG_WARNING(text) \
		qDebug() << __FILE__ << ":" << __LINE__; \
		qWarning() << text
#else
#	define LOG_WARNING(text)
#endif

class QIODevice;
class QDir;

namespace Banana
{

namespace Utils
{
	extern const QString *pTypeKey;

	QVariantMap ConvertQFontToVariantMap(const QFont &font);
	QFont ConvertVariantMapToQFont(const QVariantMap &object);
	bool CreateSymLink(const QString &target, const QString& linkpath);
	void SortStringList(QStringList &stringList);

	bool LoadTextFromFile(QString &text, const QString &filepath);
	bool LoadVariantMapFromFile(QVariantMap &vmap, const QString &filepath);
	bool SaveVariantMapToFile(const QVariantMap &vmap, const QString &filepath);

	bool LoadVariantMapFromIODevice(QVariantMap &vmap, QIODevice *device);
	bool SaveVariantMapToIODevice(const QVariantMap &vmap, QIODevice *device);

	bool LoadBinaryFromIODevice(QByteArray &output, QIODevice *device);
	bool LoadBinaryFromFile(QByteArray &output, const QString &filepath);

	QStringList ListDirectoryContents(const QString &path, const QStringList &filters);

	bool VariantsEqual(const QVariant &a, const QVariant &b);
	bool VariantIsEmpty(const QVariant &value);

	QJsonValue ConvertVariantToJsonValue(const QVariant &variant);
	QVariant ConvertJsonValueToVariant(const QJsonValue &value);

	QString ConvertToIdentifierName(const QString &source);

	QString ConvertToFileName(const QString &source, int max_size = 255 - 11);

	bool FileNameIsValid(const QString &fileName);

	QStringList SplitPath(const QString &path);

	QString QuoteString(const QString &value);

	bool IsDigitString(const QString &str);

	bool CheckFilePath(const QString &path);

	QVariant ToStandardVariant(const QVariant &variant);

	QVariant ValueFrom(const QVariantMap &data, const QString &key, const QVariant &default_value = QVariant());

	void ShowInGraphicalShell(const QString &pathIn);

	QMetaProperty GetMetaPropertyByName(const QMetaObject *metaObject, const char *propertyName);
	QMetaProperty GetMetaPropertyByName(const QObject *object, const char *propertyName);

	const QMetaObject *GetMetaObjectForProperty(const QMetaProperty &property);
}

}
