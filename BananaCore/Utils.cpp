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

#ifdef _WINDOWS
#define NOMINMAX
#include <windows.h>
#endif

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
		Q_ASSERT(nullptr != device);

		if (device->isOpen())
		{
			QByteArray binary;
			auto size64 = device->size();
			if (size64 >= 0 && size64 <= 128 * 1024 * 1024)
			{
				int size = static_cast<int>(size64);
				binary.resize(size);
				if (device->read(binary.data(), size) == size)
				{
					QJsonParseError parse_result;
					auto doc = QJsonDocument::fromJson(binary, &parse_result);

					if (QJsonParseError::NoError == parse_result.error
					&&	doc.isObject())
					{
						binary.clear();
						vmap = ConvertJsonValueToVariant(doc.object()).toMap();
						return true;
					}
				}
			}
		}

		return false;
	}

	bool SaveVariantMapToIODevice(const QVariantMap &vmap, QIODevice *device)
	{
		Q_ASSERT(nullptr != device);

		if (device->isOpen())
		{
			QJsonDocument doc;

			doc.setObject(ConvertVariantToJsonValue(vmap).toObject());

			auto binary = doc.toJson();

			auto size = binary.size();

			return (device->write(binary.data(), size) == size);
		}

		return false;
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

			if ((c >= '0' && c <= '9')
			||	(c >= 'A' && c <= 'Z')
			||	(c >= 'a' && c <= 'z')
			||	c == '_')
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

	QVariant ValueFrom(const QVariantMap &data, const QString &key, const QVariant &default_value)
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

				for (auto it = list.begin(); it != list.end(); ++ it)
				{
					array.append(ConvertVariantToJsonValue(*it));
				}

				return QJsonValue(array);
			}

			case QMetaType::QUrl:
			{
				QJsonObject object;

				object.insert(sTypeKey, "QUrl");
				object.insert(sValueKey, variant.toUrl().toString(QUrl::FullyEncoded));

				return QJsonValue(object);
			}

			case QMetaType::QFont:
			{
				return QJsonValue(QJsonObject::fromVariantMap(ConvertQFontToVariantMap(variant.value<QFont>())));
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
						QUrl url(object.value(sValueKey).toString(), QUrl::StrictMode);

						return QVariant(url);
					}

					if (type == "QFont")
					{
						return QVariant(ConvertVariantMapToQFont(object.toVariantMap()));
					}
				}
			} else
			if (object.size() == 4)
			{
				auto x_it = object.find(sXKey);
				auto y_it = object.find(sYKey);
				auto width_it = object.find(sWKey);
				auto height_it = object.find(sHKey);

				if (object.end() != x_it && object.end() != y_it
				&&	object.end() != width_it && object.end() != height_it)
				{
					return QRectF(x_it.value().toDouble(), y_it.value().toDouble(),
								  width_it.value().toDouble(), height_it.value().toDouble());
				}
			} else
			if (object.size() == 2)
			{
				auto x_it = object.find(sXKey);
				auto y_it = object.find(sYKey);

				if (object.end() != x_it && object.end() != y_it)
				{
					return QPointF(x_it.value().toDouble(), y_it.value().toDouble());
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

	bool LoadTextFromFile(QString &text, const QString &filepath)
	{
		bool ok = false;
		QFile file(filepath);

		if (file.open(QIODevice::ReadOnly))
		{
			QTextStream stream(&file);
			text = stream.readAll();

			ok = (QTextStream::Ok == stream.status());

			file.close();
		}

		return ok;
	}

	QStringList ListDirectoryContents(const QString &path, const QStringList &filters)
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
			} else
			if (name != ConvertToFileName(name))
				return false;

		}

		return true;
	}

	QScriptValue VariantToScriptValue(const QVariant &variant, QScriptEngine *engine)
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
			} break;

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
			} break;

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
			} break;
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
		std::sort(stringList.begin(), stringList.end(),
		[](const QString &a, const QString &b) -> bool
		{
			return QString::compare(a, b, Qt::CaseInsensitive) < 0;
		});
	}

	bool CreateSymLink(const QString &target, const QString &linkpath)
	{
#ifdef _WINDOWS
		DWORD dwFlags = QFileInfo(target).isDir() ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0;
	#ifdef UNICODE
		wchar_t wcTarget[1024];
		wcTarget[target.toWCharArray(wcTarget)] = 0;
		wchar_t wcLink[1024];
		wcLink[linkpath.toWCharArray(wcLink)] = 0;

		return CreateSymbolicLinkW(wcLink, wcTarget, dwFlags);
	#else
		return CreateSymbolicLinkA(linkpath.toLocal8Bit().data(),
								   target.toLocal8Bit().data(), dwFlags);
	#endif
#else
		return QFile::link(target, linkpath);
#endif
	}

	QFont ConvertVariantMapToQFont(const QVariantMap &object)
	{
		QFont font;
		font.fromString(object.value(sValueKey).toString());
		font.setStyleStrategy(QFont::PreferAntialias);
		if (object.contains(sStyleKey))
			font.setStyleName(object.value(sStyleKey).toString());
		if (object.contains(sStyleStrategyKey))
		{
			auto meta = QMetaEnum::fromType<QFont::StyleStrategy>();
			auto value = object.value(sStyleStrategyKey).toString().toLatin1();
			bool ok = false;
			auto strategy = (QFont::StyleStrategy) meta.keyToValue(value.data(), &ok);
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
		object.insert(sValueKey, font.toString());
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
				   << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"")
					  .arg(pathIn);
		QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
		scriptArgs.clear();
		scriptArgs << QLatin1String("-e")
				   << QLatin1String("tell application \"Finder\" to activate");
		QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
#else
#	error Unsupported platform
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

		return (a.type() == b.type()
			&&	a.userType() == b.userType()
			&&	a == b);
	}

	bool VariantIsEmpty(const QVariant &value)
	{
		return (value.isNull()
			||	!value.isValid()
			||	(value.type() == QVariant::String && value.toString().isEmpty()));
	}

	QString QuoteString(const QString &value)
	{
		QString result(value);
		return '"' + result.replace('\\', "\\\\").replace('"', "\\\"") + '"';
	}

	QMetaProperty GetMetaPropertyByName(const QObject *object, const char *propertyName)
	{
		Q_ASSERT(nullptr != object);
		return GetMetaPropertyByName(object->metaObject(), propertyName);
	}

	QMetaProperty GetMetaPropertyByName(const QMetaObject *metaObject, const char *propertyName)
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
			result = result->superClass();

		Q_ASSERT(nullptr != result);

		return result;
	}

}
