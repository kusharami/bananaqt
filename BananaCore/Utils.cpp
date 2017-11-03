/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2016 Alexandra Cherdantseva

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

#include "Utils.h"

#include "Core.h"
#include "NamingPolicy.h"
#include "ScriptUtils.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QRegExp>
#include <QDir>
#include <QStringList>
#include <QUrl>
#include <QFont>
#include <QFontDatabase>
#include <QRect>
#include <QRectF>
#include <QPoint>
#include <QPointF>
#include <QSize>
#include <QSizeF>
#include <QTextStream>
#include <QScriptEngine>
#include <QMetaEnum>
#include <QProcess>
#include <QMimeData>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace Banana
{
namespace Utils
{
static const QString sTypeKey("__type");
static const QString sValueKey("value");
static const QString sStyleKey("style");
static const QString sStyleStrategyKey("styleStrategy");
static const QString sXKey("x");
static const QString sYKey("y");
static const QString sWKey("width");
static const QString sHKey("height");

const QString *pTypeKey = &sTypeKey;

bool LoadVariantMapFromIODevice(QVariantMap &vmap, QIODevice *device)
{
	QJsonDocument doc;
	if (LoadJsonFromIODevice(doc, device))
	{
		vmap = ConvertJsonValueToVariant(doc.object()).toMap();
		return true;
	}

	return false;
}

bool SaveVariantMapToIODevice(const QVariantMap &vmap, QIODevice *device)
{
	QJsonDocument doc;
	doc.setObject(ConvertVariantToJsonValue(vmap).toObject());

	return SaveJsonToIODevice(doc, device);
}

bool LoadVariantMapFromFile(QVariantMap &vmap, const QString &filepath)
{
	bool ok = false;
	QFile file(filepath);

	if (file.open(QIODevice::ReadOnly))
	{
		ok = LoadVariantMapFromIODevice(vmap, &file);

		file.close();
	}

	return ok;
}

bool SaveVariantMapToFile(const QVariantMap &vmap, const QString &filepath)
{
	bool ok = false;
	if (QDir().mkpath(QFileInfo(filepath).path()))
	{
		QFile::remove(filepath);

		QFile file(filepath);
		if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
		{
			ok = SaveVariantMapToIODevice(vmap, &file);

			file.close();
		}
	}

	return ok;
}

QString ConvertToIdentifierName(const QString &source)
{
	QString result(source);
	auto new_name_bytes = result.toLatin1();
	auto name_data = new_name_bytes.data();
	int name_size = new_name_bytes.size();
	for (int i = 0; i < name_size; i++)
	{
		char c = name_data[i];

		if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') ||
			(c >= 'a' && c <= 'z') || c == '_')
			continue;

		name_data[i] = '_';
	}
	if (name_size > 0)
	{
		if (name_data[0] >= '0' && name_data[0] <= '9')
		{
			new_name_bytes.insert(0, '_');
		}
		result = QString(new_name_bytes);
	}

	return result;
}

QString ConvertToFileName(const QString &source, int max_size)
{
	QString result(source);

	result.replace(QRegExp("[\\x00-\\x1F\\\\\\/\\|\\?\\:\\*\\\"\\<\\>]"), "_");

	result.resize(std::min(max_size, result.length()));

	return result;
}

QStringList SplitPath(const QString &path)
{
	QString path_str(path);
	path_str.replace(QRegExp("[\\\\\\/]"), "/");

	QStringList result = path_str.split('/', QString::SkipEmptyParts);

	if (QDir::isAbsolutePath(path) && path_str.at(0) == '/')
	{
		if (result.isEmpty())
			result.append("/");
		else
		{
			QString first_step = result.at(0);
			first_step.prepend("/");
			result[0] = first_step;
		}
	}

	return result;
}

QVariant ValueFrom(
	const QVariantMap &data, const QString &key, const QVariant &default_value)
{
	auto it = data.find(key);

	if (it != data.end())
		return it.value();

	return default_value;
}

QJsonValue ConvertVariantToJsonValue(const QVariant &variant)
{
	switch (variant.type())
	{
		case QMetaType::QVariantMap:
		{
			auto map = variant.toMap();

			QJsonObject object;

			for (auto it = map.begin(); it != map.end(); ++it)
			{
				object.insert(it.key(), ConvertVariantToJsonValue(it.value()));
			}

			return QJsonValue(object);
		}

		case QMetaType::QVariantList:
		{
			auto list = variant.toList();

			QJsonArray array;

			for (auto it = list.begin(); it != list.end(); ++it)
			{
				array.append(ConvertVariantToJsonValue(*it));
			}

			return QJsonValue(array);
		}

		case QMetaType::QUrl:
		{
			QJsonObject object;

			object.insert(sTypeKey, "QUrl");
			object.insert(
				sValueKey, variant.toUrl().toString(QUrl::FullyEncoded));

			return QJsonValue(object);
		}

		case QMetaType::QFont:
		{
			return QJsonValue(QJsonObject::fromVariantMap(
				ConvertQFontToVariantMap(variant.value<QFont>())));
		}

		case QMetaType::QRect:
		{
			auto rect = variant.toRect();

			QJsonObject object;

			object.insert(sTypeKey, "QRect");
			object.insert(sXKey, rect.x());
			object.insert(sYKey, rect.y());
			object.insert(sWKey, rect.width());
			object.insert(sHKey, rect.height());

			return QJsonValue(object);
		}

		case QMetaType::QRectF:
		{
			auto rect = variant.toRectF();

			QJsonObject object;

			object.insert(sXKey, rect.x());
			object.insert(sYKey, rect.y());
			object.insert(sWKey, rect.width());
			object.insert(sHKey, rect.height());

			return QJsonValue(object);
		}

		case QMetaType::QSize:
		{
			auto size = variant.toSize();

			QJsonObject object;

			object.insert(sTypeKey, "QSize");
			object.insert(sWKey, size.width());
			object.insert(sHKey, size.height());

			return QJsonValue(object);
		}

		case QMetaType::QSizeF:
		{
			auto size = variant.toSizeF();

			QJsonObject object;

			object.insert(sWKey, size.width());
			object.insert(sHKey, size.height());

			return QJsonValue(object);
		}

		case QMetaType::QPoint:
		{
			auto point = variant.toPoint();

			QJsonObject object;

			object.insert(sTypeKey, "QPoint");
			object.insert(sXKey, point.x());
			object.insert(sYKey, point.y());

			return QJsonValue(object);
		}

		case QMetaType::QPointF:
		{
			auto point = variant.toPointF();

			QJsonObject object;

			object.insert(sXKey, point.x());
			object.insert(sYKey, point.y());

			return QJsonValue(object);
		}

		default:
			break;
	}

	return QJsonValue::fromVariant(variant);
}

QVariant ConvertJsonValueToVariant(const QJsonValue &value)
{
	if (value.isObject())
	{
		auto object = value.toObject();

		auto type_it = object.find(sTypeKey);
		if (object.end() != type_it)
		{
			auto value = type_it.value();
			if (value.isString())
			{
				auto type = value.toString();

				if (type == "QPoint")
				{
					QPoint point(object.value(sXKey).toInt(),
						object.value(sYKey).toInt());

					return QVariant(point);
				}

				if (type == "QPointF")
				{
					QPointF point(object.value(sXKey).toDouble(),
						object.value(sYKey).toDouble());

					return QVariant(point);
				}

				if (type == "QSize")
				{
					QSize size(object.value(sWKey).toInt(),
						object.value(sHKey).toInt());

					return QVariant(size);
				}

				if (type == "QSizeF")
				{
					QSizeF size(object.value(sWKey).toDouble(),
						object.value(sHKey).toDouble());

					return QVariant(size);
				}

				if (type == "QRect")
				{
					QRect rect(object.value(sXKey).toInt(),
						object.value(sYKey).toInt(),
						object.value(sWKey).toInt(),
						object.value(sHKey).toInt());

					return QVariant(rect);
				}

				if (type == "QRectF")
				{
					QRectF rect(object.value(sXKey).toDouble(),
						object.value(sYKey).toDouble(),
						object.value(sWKey).toDouble(),
						object.value(sHKey).toDouble());

					return QVariant(rect);
				}

				if (type == "QUrl")
				{
					QUrl url(
						object.value(sValueKey).toString(), QUrl::StrictMode);

					return QVariant(url);
				}

				if (type == "QFont")
				{
					return QVariant(
						ConvertVariantMapToQFont(object.toVariantMap()));
				}
			}
		} else if (object.size() == 4)
		{
			auto x_it = object.find(sXKey);
			auto y_it = object.find(sYKey);
			auto width_it = object.find(sWKey);
			auto height_it = object.find(sHKey);

			if (object.end() != x_it && object.end() != y_it &&
				object.end() != width_it && object.end() != height_it)
			{
				return QRectF(x_it.value().toDouble(), y_it.value().toDouble(),
					width_it.value().toDouble(), height_it.value().toDouble());
			}
		} else if (object.size() == 2)
		{
			auto x_it = object.find(sXKey);
			auto y_it = object.find(sYKey);

			if (object.end() != x_it && object.end() != y_it)
			{
				return QPointF(
					x_it.value().toDouble(), y_it.value().toDouble());
			}

			x_it = object.find(sWKey);
			y_it = object.find(sHKey);

			if (object.end() != x_it && object.end() != y_it)
			{
				return QSizeF(x_it.value().toDouble(), y_it.value().toDouble());
			}
		}

		QVariantMap map;
		for (auto it = object.begin(); it != object.end(); ++it)
		{
			map.insert(it.key(), ConvertJsonValueToVariant(it.value()));
		}
		return QVariant(map);
	}

	if (value.isArray())
	{
		QVariantList list;

		auto array = value.toArray();

		for (auto it = array.begin(); it != array.end(); ++it)
		{
			list.append(ConvertJsonValueToVariant(*it));
		}

		return QVariant(list);
	}

	return value.toVariant();
}

bool LoadTextFromFile(QString &text, const QString &filePath)
{
	bool ok = false;
	QFile file(filePath);

	if (file.open(QIODevice::ReadOnly))
	{
		QTextStream stream(&file);
		text = stream.readAll();

		ok = (QTextStream::Ok == stream.status());

		file.close();
	}

	return ok;
}

bool SaveTextToFile(const QString &text, const QString &filePath)
{
	bool ok = false;

	QFile file(filePath);

	if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
	{
		auto utf8 = text.toUtf8();
		ok = (utf8.size() == file.write(utf8));

		file.close();
	}

	return ok;
}

QStringList ListDirectoryContents(
	const QString &path, const QStringList &filters)
{
	QStringList result;

	QDir dir(path);

	if (dir.exists())
	{
		if (filters.isEmpty())
			result = dir.entryList(filters);
		else
		{
			for (auto it = filters.begin(); it != filters.end(); ++it)
			{
				QStringList filter;
				filter.push_back(*it);
				result << dir.entryList(filter);
			}
		}
	}

	return result;
}

bool CheckFilePath(const QString &path)
{
	bool absolute = QDir::isAbsolutePath(path);
	auto split = SplitPath(path);

	bool first = true;

	foreach (const QString &name, split)
	{
		if (first && absolute)
		{
			first = false;

			if (!name.isEmpty() && name.at(0) == '/')
			{
				QString check(&name.data()[1]);

				if (check != ConvertToFileName(check))
					return false;
			}
		} else if (name != ConvertToFileName(name))
			return false;
	}

	return true;
}

QScriptValue VariantToScriptValue(
	const QVariant &variant, QScriptEngine *engine)
{
	QScriptValue result;

	switch (variant.type())
	{
		case QVariant::Map:
		{
			auto vmap = variant.toMap();

			result = engine->newObject();

			for (auto it = vmap.begin(); it != vmap.end(); ++it)
			{
				auto &key = it.key();
				auto &value = it.value();

				result.setProperty(key, VariantToScriptValue(value, engine));
			}
			break;
		}

		case QVariant::List:
		case QVariant::StringList:
		{
			auto vlist = variant.toList();

			int len = vlist.length();
			result = engine->newArray(len);

			for (int i = 0; i < len; i++)
			{
				auto &value = vlist.at(i);

				result.setProperty(i, VariantToScriptValue(value, engine));
			}
			break;
		}

		default:
		{
			if (variant.isNull() || !variant.isValid())
				result = QScriptValue(engine, QScriptValue::NullValue);
			else
				switch (variant.type())
				{
					case QVariant::Bool:
						result = QScriptValue(engine, variant.toBool());
						break;

					case QVariant::Int:
						result = QScriptValue(engine, variant.toInt());
						break;

					case QVariant::UInt:
						result = QScriptValue(engine, variant.toUInt());
						break;

					case QVariant::LongLong:
					case QVariant::ULongLong:
					case QVariant::Double:
						result = QScriptValue(engine, variant.toDouble());
						break;

					case QVariant::Char:
					case QVariant::String:
						result = QScriptValue(engine, variant.toString());
						break;

					default:
						result = engine->newVariant(variant);
						break;
				}

			break;
		}
	}

	return result;
}

bool LoadBinaryFromIODevice(QByteArray &output, QIODevice *device)
{
	Q_ASSERT(nullptr != device);

	if (device->isOpen())
	{
		auto size64 = device->size();
		if (size64 >= 0 && size64 <= 256 * 1024 * 1024)
		{
			int size = static_cast<int>(size64);
			output.resize(size);
			return (device->read(output.data(), size) == size);
		}
	}

	return false;
}

bool LoadBinaryFromFile(QByteArray &output, const QString &filepath)
{
	bool ok = false;
	QFile file(filepath);

	if (file.open(QIODevice::ReadOnly))
	{
		ok = LoadBinaryFromIODevice(output, &file);

		file.close();
	}

	return ok;
}

void SortStringList(QStringList &stringList)
{
	std::stable_sort(stringList.begin(), stringList.end(),
		[](const QString &a, const QString &b) -> bool {
			return QString::compare(a, b, Qt::CaseInsensitive) < 0;
		});
}

bool CreateSymLink(const QString &target, const QString &linkpath)
{
#ifdef Q_OS_WIN
	DWORD dwFlags =
		QFileInfo(target).isDir() ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0;
#ifdef UNICODE
	static_assert(sizeof(WCHAR) == sizeof(decltype(*target.utf16())),
		"WCHAR size mismatch");

	return CreateSymbolicLinkW(
		reinterpret_cast<const WCHAR *>(linkpath.utf16()),
		reinterpret_cast<const WCHAR *>(target.utf16()), dwFlags);
#else
	return CreateSymbolicLinkA(
		linkpath.toLocal8Bit().data(), target.toLocal8Bit().data(), dwFlags);
#endif
#else
	return QFile::link(target, linkpath);
#endif
}

static QString fixedFontString(QString s)
{
	if (s.isEmpty())
		return s;

	if (s.at(s.length() - 1).isDigit())
		return s;

	int lastCommaIndex = s.lastIndexOf(',');
	if (lastCommaIndex < 0)
		return s;

	s.resize(lastCommaIndex);
	return s;
}

QFont ConvertVariantMapToQFont(const QVariantMap &object)
{
	QFont font;
	font.fromString(fixedFontString(object.value(sValueKey).toString()));
	font.setStyleStrategy(QFont::PreferAntialias);
	if (object.contains(sStyleKey))
		font.setStyleName(object.value(sStyleKey).toString());
	if (object.contains(sStyleStrategyKey))
	{
		auto meta = QMetaEnum::fromType<QFont::StyleStrategy>();
		auto value = object.value(sStyleStrategyKey).toString().toLatin1();
		bool ok = false;
		auto strategy =
			(QFont::StyleStrategy) meta.keyToValue(value.data(), &ok);
		if (ok)
			font.setStyleStrategy(strategy);
	}

#ifdef Q_OS_MAC
	auto style = font.styleName();
	if (!style.isEmpty())
	{
		auto family = font.family();
		QFontDatabase db;
		for (auto &s : db.styles(family))
		{
			if (s == style)
			{
				font.setBold(db.bold(family, style));
				font.setItalic(db.italic(family, style));
				break;
			}
		}
	}
#endif

	return font;
}

QVariantMap ConvertQFontToVariantMap(const QFont &font)
{
	QVariantMap object;
	object.insert(sTypeKey, "QFont");
	object.insert(sValueKey, fixedFontString(font.toString()));
	object.insert(sStyleKey, font.styleName());
	auto meta = QMetaEnum::fromType<QFont::StyleStrategy>();
	object.insert(sStyleStrategyKey, meta.valueToKey(font.styleStrategy()));

	return object;
}

void ShowInGraphicalShell(const QString &pathIn)
{
#ifdef Q_OS_WIN
	QStringList param;
	if (!QFileInfo(pathIn).isDir())
		param += QLatin1String("/select,");
	param += QDir::toNativeSeparators(pathIn);
	QProcess::startDetached(QLatin1String("explorer.exe"), param);
#elif defined(Q_OS_MAC)
	QStringList scriptArgs;
	scriptArgs << QLatin1String("-e")
			   << QString::fromLatin1("tell application \"Finder\" "
									  "to reveal POSIX "
									  "file \"%1\"")
					  .arg(pathIn);
	QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
	scriptArgs.clear();
	scriptArgs << QLatin1String("-e")
			   << QLatin1String("tell application \"Finder\" to activate");
	QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
#else
#error Unsupported platform
#endif
}

bool VariantsEqual(const QVariant &a, const QVariant &b)
{
	bool emptyA = VariantIsEmpty(a);
	bool emptyB = VariantIsEmpty(b);

	if (emptyA)
		return emptyB;

	if (emptyB)
		return emptyA;

	return (a.type() == b.type() && a.userType() == b.userType() && a == b);
}

bool VariantIsEmpty(const QVariant &value)
{
	return (value.isNull() || !value.isValid() ||
		(value.type() == QVariant::String && value.toString().isEmpty()));
}

QString QuoteString(const QString &value)
{
	QString result(value);
	return '"' + result.replace('\\', "\\\\").replace('"', "\\\"") + '"';
}

QMetaProperty GetMetaPropertyByName(
	const QObject *object, const char *propertyName)
{
	Q_ASSERT(nullptr != object);
	return GetMetaPropertyByName(object->metaObject(), propertyName);
}

QMetaProperty GetMetaPropertyByName(
	const QMetaObject *metaObject, const char *propertyName)
{
	Q_ASSERT(nullptr != metaObject);

	int index = metaObject->indexOfProperty(propertyName);
	Q_ASSERT(index >= 0);

	return metaObject->property(index);
}

const QMetaObject *GetMetaObjectForProperty(const QMetaProperty &property)
{
	auto result = property.enclosingMetaObject();

	int index = result->indexOfProperty(property.name());
	Q_ASSERT(index >= 0);

	while (nullptr != result && result->propertyOffset() > index)
	{
		result = result->superClass();
	}

	Q_ASSERT(nullptr != result);

	return result;
}

bool IsDigitString(const QString &str)
{
	for (auto &c : str)
	{
		if (!c.isDigit())
			return false;
	}

	return true;
}

bool FileNameIsValid(const QString &fileName)
{
	return (ConvertToFileName(fileName) == fileName);
}

bool IsAncestorOf(const QObject *descendant, const QObject *object)
{
	if (nullptr != object && nullptr != descendant)
	{
		forever
		{
			object = object->parent();

			if (nullptr == object)
				break;

			if (object == descendant)
				return true;
		}
	}

	return false;
}

bool DeleteFileOrLink(const QString &filePath)
{
	return DeleteFileOrLink(QFileInfo(filePath));
}

bool DeleteFileOrLink(const QFileInfo &fileInfo)
{
	bool isLink = fileInfo.isSymLink();
	if (!isLink && !fileInfo.exists())
		return true;

#ifdef Q_OS_WIN
	if (isLink && fileInfo.isDir())
	{
		if (QDir().rmdir(fileInfo.filePath()))
			return true;
	}
#endif
	return QFile::remove(fileInfo.filePath());
}

const QMetaObject *GetMetaObjectForClass(const QByteArray &className)
{
	int typeId =
		QMetaType::type(className.endsWith('*') ? className : className + " *");
	return QMetaType::metaObjectForType(typeId);
}

const QObject *LoadQObjectPointer(
	const QMetaObject *metaObject, const QMimeData *data)
{
	Q_ASSERT(nullptr != metaObject);
	Q_ASSERT(nullptr != data);

	QString className(metaObject->className());
	if (data->hasFormat(className))
	{
		auto bytes = data->data(className);

		if (bytes.size() == sizeof(uintptr_t))
		{
			auto ptr = reinterpret_cast<const uintptr_t *>(bytes.constData());
			return reinterpret_cast<const QObject *>(*ptr);
		}
	}

	return nullptr;
}

void StoreQObjectPointer(const QObject *object, QMimeData *data)
{
	Q_ASSERT(nullptr != object);
	Q_ASSERT(nullptr != data);

	QByteArray bytes;
	bytes.resize(sizeof(uintptr_t));
	auto ptr = reinterpret_cast<uintptr_t *>(bytes.data());
	*ptr = (uintptr_t) object;
	data->setData(object->metaObject()->className(), bytes);
}

QStringList GetNamesChain(
	const QObject *topAncestor, const QObject *bottomDescendant)
{
	QStringList result;

	const QObject *current = bottomDescendant;

	while (nullptr != current && current != topAncestor)
	{
		result.push_front(current->objectName());

		current = current->parent();
	}

	return result;
}

QObject *GetTopAncestor(QObject *object)
{
	while (nullptr != object)
	{
		auto parent = object->parent();
		if (nullptr == parent)
			return object;

		object = parent;
	}

	return nullptr;
}

QObject *GetDescendant(const QObject *topAncestor, const QStringList &path)
{
	auto result = const_cast<QObject *>(topAncestor);
	if (nullptr == result)
		return nullptr;

	int count = path.count();
	for (int i = 0; i < count; i++)
	{
		result = result->findChild<QObject *>(
			path.at(i), Qt::FindDirectChildrenOnly);
		if (nullptr == result)
			break;
	}

	return result;
}

bool LoadJsonFromFile(QJsonDocument &doc, const QString &filepath)
{
	bool ok = false;
	QFile file(filepath);

	if (file.open(QIODevice::ReadOnly))
	{
		ok = LoadJsonFromIODevice(doc, &file);

		file.close();
	}

	return ok;
}

bool SaveJsonToFile(const QJsonDocument &doc, const QString &filepath)
{
	bool ok = false;
	if (QDir().mkpath(QFileInfo(filepath).path()))
	{
		QFile::remove(filepath);

		QFile file(filepath);
		if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
		{
			ok = SaveJsonToIODevice(doc, &file);

			file.close();
		}
	}

	return ok;
}

bool LoadJsonFromIODevice(QJsonDocument &doc, QIODevice *device)
{
	Q_ASSERT(nullptr != device);

	if (device->isOpen())
	{
		QByteArray binary;
		auto size64 = device->size();
		if (size64 >= 0 && size64 <= 128 * 1024 * 1024)
		{
			int size = int(size64);
			binary.resize(size);
			if (device->read(binary.data(), size) == size)
			{
				QJsonParseError parseResult;
				doc = QJsonDocument::fromJson(binary, &parseResult);

				if (QJsonParseError::NoError == parseResult.error &&
					doc.isObject())
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool SaveJsonToIODevice(const QJsonDocument &doc, QIODevice *device)
{
	Q_ASSERT(nullptr != device);

	if (device->isOpen())
	{
		auto binary = doc.toJson();

		auto size = binary.size();

		return (device->write(binary.data(), size) == size);
	}

	return false;
}

const QMetaObject *GetMetaObjectForClass(const QString &className)
{
	return GetMetaObjectForClass(className.toLatin1());
}
}
}
