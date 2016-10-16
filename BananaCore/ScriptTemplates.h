#pragma once

#include "Const.h"

#include <type_traits>
#include <QScriptEngine>
#include <QRectF>
#include <QFont>
#include <QFileInfo>
#include <QDir>
#include <QColor>

#define GET_SET (QScriptValue::PropertyGetter | QScriptValue::PropertySetter)

#define GETTER_SETTER(Type, CLASS, Name, prototype) \
	GETTER_SETTER_EX(Type, CLASS, Name, Set##Name, prototype)

#define GETTER_SETTER_EX(Type, CLASS, Get, Set, prototype) \
	engine->newFunction(Type##Templates::GetterSetter<CLASS, &CLASS::Get, &CLASS::Set>, prototype)

#define DEFINE_SCRIPT_ENUM(Type, TOTAL, enumObject, parent) \
	enumObject = engine->newObject(); \
	parent.setProperty(#Type, enumObject, STATIC_SCRIPT_VALUE); \
	for (int i = 0; i < TOTAL; i++) \
		enumObject.setProperty(Type##ToStr((Type) i), i, STATIC_SCRIPT_VALUE)

#define DEFINE_SCRIPT_FLAGS(Name, Type, TOTAL, flagsObject, parent) \
	flagsObject = engine->newObject(); \
	parent.setProperty(Name, flagsObject, STATIC_SCRIPT_VALUE); \
	for (int i = 0; i < TOTAL; i++) \
		flagsObject.setProperty(Type##ToStr((Type) i), 1 << i, STATIC_SCRIPT_VALUE); \
	flagsObject.setProperty("All", (1 << TOTAL) - 1, STATIC_SCRIPT_VALUE)

Q_DECLARE_METATYPE(QFont::Style)
Q_DECLARE_METATYPE(QDir)
Q_DECLARE_METATYPE(QDir::Filters)
Q_DECLARE_METATYPE(QDir::SortFlags)

#define STATIC_SCRIPT_VALUE (QScriptValue::ReadOnly | QScriptValue::Undeletable)

namespace Scripting
{
	class Color : public QObject, public QColor
	{
		Q_OBJECT

		Q_PROPERTY(int alpha READ alpha WRITE setAlpha STORED true)
		Q_PROPERTY(qreal alphaF READ alphaF WRITE setAlphaF STORED false)

		Q_PROPERTY(int red READ red WRITE setRed STORED true)
		Q_PROPERTY(qreal redF READ redF WRITE setRedF STORED false)

		Q_PROPERTY(int green READ green WRITE setGreen STORED true)
		Q_PROPERTY(qreal greenF READ greenF WRITE setGreenF STORED false)

		Q_PROPERTY(int blue READ blue WRITE setBlue STORED true)
		Q_PROPERTY(qreal blueF READ blueF WRITE setBlueF STORED false)

		Q_PROPERTY(quint32 rgba READ rgba WRITE setRgba STORED false)
		Q_PROPERTY(quint32 rgb READ rgb WRITE setRgb STORED false)

	public:
		Color(const QColor &color);

		Q_INVOKABLE inline QString toString() const;
		Q_INVOKABLE inline QString toStringWithAlpha(bool withAlpha) const;
		Q_INVOKABLE inline void setNamedColor(const QString &str);

		Q_INVOKABLE inline bool equals(const QColor &other) const;

		static QScriptValue Constructor(QScriptContext *context, QScriptEngine *engine);
		static QScriptValue ToScriptValue(QScriptEngine *engine, const QColor &in);
		static void FromScriptValue(const QScriptValue &object, QColor &out);

	private:
		static QScriptValue ConstructWith(QScriptContext *context,
										  QScriptEngine *engine,
										  const QScriptValue &red,
										  const QScriptValue &green,
										  const QScriptValue &blue,
										  const QScriptValue &alpha);
	};

	QString Color::toString() const
	{
		return toStringWithAlpha(false);
	}

	QString Color::toStringWithAlpha(bool withAlpha) const
	{
		return QColor::name(withAlpha ? QColor::HexArgb : QColor::HexRgb);
	}

	void Color::setNamedColor(const QString &str)
	{
		QColor::setNamedColor(str);
	}

	bool Color::equals(const QColor &other) const
	{
		return *this == other;
	}

	class Dir : public QObject, public QDir
	{
		Q_OBJECT

		Q_PROPERTY(QString path READ path WRITE setPath STORED true)
		Q_PROPERTY(QString dirName READ dirName STORED false)
		Q_PROPERTY(QString absolutePath READ absolutePath STORED false)
		Q_PROPERTY(QString canonicalPath READ canonicalPath STORED false)

		Q_PROPERTY(bool readable READ isReadable STORED false)
		Q_PROPERTY(bool relative READ isRelative STORED false)
		Q_PROPERTY(bool absolute READ isAbsolute STORED false)
		Q_PROPERTY(bool isRoot READ isRoot STORED false)

	public:
		Dir(const QDir &info);

		Q_INVOKABLE inline bool exists() const;
		Q_INVOKABLE inline bool exists(const QString &name) const;
		Q_INVOKABLE inline bool mkdir(const QString &dirName) const;
		Q_INVOKABLE inline bool rmdir(const QString &dirName) const;
		Q_INVOKABLE inline bool mkpath(const QString &dirPath) const;
		Q_INVOKABLE inline bool rmpath(const QString &dirPath) const;

		Q_INVOKABLE inline bool removeRecursively();
		Q_INVOKABLE inline bool makeAbsolute();

		Q_INVOKABLE inline bool cd(const QString &dirName);

		Q_INVOKABLE inline QString filePath(const QString &fileName) const;
		Q_INVOKABLE inline QString absoluteFilePath(const QString &fileName) const;
		Q_INVOKABLE inline QString relativeFilePath(const QString &fileName) const;

		Q_INVOKABLE inline bool remove(const QString &fileName);
		Q_INVOKABLE inline bool rename(const QString &oldName, const QString &newName);

		Q_INVOKABLE QScriptValue entryList(const QScriptValue &nameFilters,
												 Filters filters = NoFilter,
												 SortFlags sort = NoSort) const;

		Q_INVOKABLE inline void refresh();

		Q_INVOKABLE inline bool equals(const QDir &other) const;

		static QScriptValue Constructor(QScriptContext *context, QScriptEngine *engine);
		static QScriptValue ToScriptValue(QScriptEngine *engine, const QDir &in);
		static void FromScriptValue(const QScriptValue &object, QDir &out);
	};

	bool Dir::exists() const
	{
		return QDir::exists();
	}

	bool Dir::exists(const QString &name) const
	{
		return QDir::exists(name);
	}

	bool Dir::mkdir(const QString &dirName) const
	{
		return QDir::mkdir(dirName);
	}

	bool Dir::rmdir(const QString &dirName) const
	{
		return QDir::rmdir(dirName);
	}

	bool Dir::mkpath(const QString &dirPath) const
	{
		return QDir::mkpath(dirPath);
	}

	bool Dir::rmpath(const QString &dirPath) const
	{
		return QDir::rmpath(dirPath);
	}

	bool Dir::removeRecursively()
	{
		return QDir::removeRecursively();
	}

	bool Dir::makeAbsolute()
	{
		return QDir::makeAbsolute();
	}

	bool Dir::cd(const QString &dirName)
	{
		return QDir::cd(dirName);
	}

	QString Dir::filePath(const QString &fileName) const
	{
		return QDir::filePath(fileName);
	}

	QString Dir::absoluteFilePath(const QString &fileName) const
	{
		return QDir::absoluteFilePath(fileName);
	}

	QString Dir::relativeFilePath(const QString &fileName) const
	{
		return QDir::relativeFilePath(fileName);
	}

	void Dir::refresh()
	{
		return QDir::refresh();
	}

	bool Dir::equals(const QDir &other) const
	{
		return *this == other;
	}

	bool Dir::remove(const QString &fileName)
	{
		return QDir::remove(fileName);
	}

	bool Dir::rename(const QString &oldName, const QString &newName)
	{
		return QDir::rename(oldName, newName);
	}

	class FileInfo : public QObject, public QFileInfo
	{
		Q_OBJECT

		Q_PROPERTY(QString filePath READ filePath WRITE setFile STORED true)
		Q_PROPERTY(QString absoluteFilePath READ absoluteFilePath STORED false)
		Q_PROPERTY(QString canonicalFilePath READ canonicalFilePath STORED false)
		Q_PROPERTY(QString fileName READ fileName STORED false)
		Q_PROPERTY(QString baseName READ baseName STORED false)
		Q_PROPERTY(QString completeBaseName READ completeBaseName STORED false)
		Q_PROPERTY(QString suffix READ suffix STORED false)
		Q_PROPERTY(QString bundleName READ bundleName STORED false)
		Q_PROPERTY(QString completeSuffix READ completeSuffix STORED false)

		Q_PROPERTY(QString path READ path STORED false)
		Q_PROPERTY(QString absolutePath READ absolutePath STORED false)
		Q_PROPERTY(QString canonicalPath READ canonicalPath STORED false)

		Q_PROPERTY(bool readable READ isReadable STORED false)
		Q_PROPERTY(bool writable READ isWritable STORED false)
		Q_PROPERTY(bool executable READ isExecutable STORED false)
		Q_PROPERTY(bool hidden READ isHidden STORED false)

		Q_PROPERTY(bool relative READ isRelative STORED false)
		Q_PROPERTY(bool absolute READ isAbsolute STORED false)

		Q_PROPERTY(bool isNativePath READ isNativePath STORED false)
		Q_PROPERTY(bool isFile READ isFile STORED false)
		Q_PROPERTY(bool isDir READ isDir STORED false)
		Q_PROPERTY(bool isSymLink READ isSymLink STORED false)
		Q_PROPERTY(bool isRoot READ isRoot STORED false)
		Q_PROPERTY(bool isBundle READ isBundle STORED false)

		Q_PROPERTY(qint64 size READ size STORED false)

	public:
		FileInfo(const QFileInfo &info);

		Q_INVOKABLE inline bool exists() const;
		Q_INVOKABLE inline void refresh();
		Q_INVOKABLE inline bool equals(const QFileInfo &other) const;

		static QScriptValue Constructor(QScriptContext *context, QScriptEngine *engine);
		static QScriptValue ToScriptValue(QScriptEngine *engine, const QFileInfo &in);
		static void FromScriptValue(const QScriptValue &object, QFileInfo &out);
	};

	bool FileInfo::exists() const
	{
		return QFileInfo::exists();
	}

	void FileInfo::refresh()
	{
		QFileInfo::refresh();
	}

	bool FileInfo::equals(const QFileInfo &other) const
	{
		return *this == other;
	}

	class Font : public QObject, public QFont
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
		Font(const QFont &font);

		Q_INVOKABLE inline QString toString() const { return QFont::toString(); }
		Q_INVOKABLE bool fromString(const QString &str);

		static QScriptValue Constructor(QScriptContext *context, QScriptEngine *engine);
		static QScriptValue ToScriptValue(QScriptEngine *engine, const QFont &in);
		static void FromScriptValue(const QScriptValue &object, QFont &out);
	};

	class Rect : public QObject, public QRectF
	{
		Q_OBJECT

		Q_PROPERTY(qreal x READ x WRITE setX)
		Q_PROPERTY(qreal y READ y WRITE setY)
		Q_PROPERTY(qreal width READ width WRITE setWidth)
		Q_PROPERTY(qreal height READ height WRITE setHeight)

		Q_PROPERTY(qreal left READ left WRITE setLeft STORED false)
		Q_PROPERTY(qreal top READ top WRITE setTop STORED false)
		Q_PROPERTY(qreal right READ right WRITE setRight STORED false)
		Q_PROPERTY(qreal bottom READ bottom WRITE setBottom STORED false)

		Q_PROPERTY(QPointF topLeft READ topLeft WRITE setTopLeft STORED false)
		Q_PROPERTY(QPointF bottomRight READ bottomRight WRITE setBottomRight STORED false)
		Q_PROPERTY(QPointF topRight READ topRight WRITE setTopRight STORED false)
		Q_PROPERTY(QPointF bottomLeft READ bottomLeft WRITE setBottomLeft STORED false)
		Q_PROPERTY(QPointF center READ center STORED false)

		Q_PROPERTY(bool null READ isNull STORED false)
		Q_PROPERTY(bool empty READ isEmpty STORED false)
		Q_PROPERTY(bool valid READ isValid STORED false)

		Q_PROPERTY(QSizeF size READ size WRITE setSize STORED false)

	public:
		typedef QRectF Inherited;

		Rect(const QRectF &rect);

		Q_INVOKABLE inline void moveLeft(qreal left);
		Q_INVOKABLE inline void moveTop(qreal top);
		Q_INVOKABLE inline void moveRight(qreal right);
		Q_INVOKABLE inline void moveBottom(qreal bottom);
		Q_INVOKABLE inline bool containsPoint(const QPointF &point) const;
		Q_INVOKABLE bool containsPoint(qreal x, qreal y) const;
		Q_INVOKABLE inline bool containsRect(const QRectF &rect) const;
		Q_INVOKABLE bool containsRect(qreal x, qreal y, qreal width, qreal height) const;
		Q_INVOKABLE inline bool intersects(const QRectF &rect) const;
		Q_INVOKABLE bool intersects(qreal x, qreal y, qreal width, qreal height) const;
		Q_INVOKABLE inline QRectF united(const QRectF &other) const;
		Q_INVOKABLE inline QRectF intersected(const QRectF &other) const;
		Q_INVOKABLE inline bool equals(const QRectF &other) const;

		static QScriptValue ToScriptValue(QScriptEngine *engine, const QRect &in);
		static QScriptValue ToScriptValue(QScriptEngine *engine, const QRectF &in);

		static void FromScriptValue(const QScriptValue &object, QRect &out);
		static void FromScriptValue(const QScriptValue &object, QRectF &out);

		static QScriptValue Constructor(QScriptContext *context, QScriptEngine *engine);
		static QScriptValue ConstructorF(QScriptContext *context, QScriptEngine *engine);

		static inline void convert(const QRectF &from, QRect &to);
		static inline void convert(const QRectF &from, QRectF &to);
	};

	void Rect::moveLeft(qreal left)
	{
		QRectF::moveLeft(left);
	}

	void Rect::moveTop(qreal top)
	{
		QRectF::moveTop(top);
	}

	void Rect::moveRight(qreal right)
	{
		QRectF::moveRight(right);
	}

	void Rect::moveBottom(qreal bottom)
	{
		QRectF::moveBottom(bottom);
	}

	bool Rect::containsPoint(const QPointF &point) const
	{
		return QRectF::contains(point);
	}

	bool Rect::containsRect(const QRectF &rect) const
	{
		return QRectF::contains(rect);
	}

	bool Rect::intersects(const QRectF &rect) const
	{
		return QRectF::intersects(rect);
	}

	QRectF Rect::united(const QRectF &other) const
	{
		return QRectF::united(other);
	}

	QRectF Rect::intersected(const QRectF &other) const
	{
		return QRectF::intersected(other);
	}

	bool Rect::equals(const QRectF &other) const
	{
		return *this == other;
	}

	void Rect::convert(const QRectF &from, QRect &to)
	{
		to = from.toRect();
	}

	void Rect::convert(const QRectF &from, QRectF &to)
	{
		to = from;
	}

	class Point : public QObject, public QPointF
	{
		Q_OBJECT

		Q_PROPERTY(qreal x READ x WRITE setX)
		Q_PROPERTY(qreal y READ y WRITE setY)
		Q_PROPERTY(qreal manhattanLength READ manhattanLength STORED false)


	public:
		typedef QPointF Inherited;

		Point(const QPointF &point);

		Q_INVOKABLE inline bool equals(const QPointF &other) const;
		Q_INVOKABLE inline QPointF sub(const QPointF &value) const;
		Q_INVOKABLE inline QPointF add(const QPointF &value) const;
		Q_INVOKABLE inline QPointF mul(qreal value) const;
		Q_INVOKABLE inline QPointF div(qreal value) const;
		Q_INVOKABLE inline qreal dotProduct(const QPointF &to) const;

		static QScriptValue ToScriptValue(QScriptEngine *engine, const QPoint &in);
		static QScriptValue ToScriptValue(QScriptEngine *engine, const QPointF &in);

		static void FromScriptValue(const QScriptValue &object, QPoint &out);
		static void FromScriptValue(const QScriptValue &object, QPointF &out);

		static QScriptValue Constructor(QScriptContext *context, QScriptEngine *engine);
		static QScriptValue ConstructorF(QScriptContext *context, QScriptEngine *engine);

		static inline void convert(const QPointF &from, QPoint &to);
		static inline void convert(const QPointF &from, QPointF &to);
	};

	bool Point::equals(const QPointF &other) const
	{
		return *this == other;
	}

	QPointF Point::sub(const QPointF &value) const
	{
		return *this - value;
	}

	QPointF Point::add(const QPointF &value) const
	{
		return *this + value;
	}

	QPointF Point::mul(qreal value) const
	{
		return *this * value;
	}

	QPointF Point::div(qreal value) const
	{
		return *this / value;
	}

	qreal Point::dotProduct(const QPointF &to) const
	{
		return QPointF::dotProduct(*this, to);
	}

	void Point::convert(const QPointF &from, QPoint &to)
	{
		to = from.toPoint();
	}

	void Point::convert(const QPointF &from, QPointF &to)
	{
		to = from;
	}

	class Size : public QObject, public QSizeF
	{
		Q_OBJECT
		Q_PROPERTY(qreal width READ width WRITE setWidth)
		Q_PROPERTY(qreal height READ height WRITE setHeight)

		Q_PROPERTY(bool null READ isNull STORED false)
		Q_PROPERTY(bool empty READ isEmpty STORED false)
		Q_PROPERTY(bool valid READ isValid STORED false)

	public:
		typedef QSizeF Inherited;

		Size(const QSizeF &size);

		Q_INVOKABLE inline bool equals(const QSizeF &other) const;
		Q_INVOKABLE inline QSizeF sub(const QSizeF &value) const;
		Q_INVOKABLE inline QSizeF add(const QSizeF &value) const;
		Q_INVOKABLE inline QSizeF mul(qreal value) const;
		Q_INVOKABLE inline QSizeF div(qreal value) const;

		static QScriptValue ToScriptValue(QScriptEngine *engine, const QSize &in);
		static QScriptValue ToScriptValue(QScriptEngine *engine, const QSizeF &in);

		static void FromScriptValue(const QScriptValue &object, QSize &out);
		static void FromScriptValue(const QScriptValue &object, QSizeF &out);

		static QScriptValue Constructor(QScriptContext *context, QScriptEngine *engine);
		static QScriptValue ConstructorF(QScriptContext *context, QScriptEngine *engine);

		static inline void convert(const QSizeF &from, QSize &to);
		static inline void convert(const QSizeF &from, QSizeF &to);
	};

	bool Size::equals(const QSizeF &other) const
	{
		return *this == other;
	}

	QSizeF Size::sub(const QSizeF &value) const
	{
		return *this - value;
	}

	QSizeF Size::add(const QSizeF &value) const
	{
		return *this + value;
	}

	QSizeF Size::mul(qreal value) const
	{
		return *this * value;
	}

	QSizeF Size::div(qreal value) const
	{
		return *this / value;
	}

	void Size::convert(const QSizeF &from, QSize &to)
	{
		to = from.toSize();
	}

	void Size::convert(const QSizeF &from, QSizeF &to)
	{
		to = from;
	}

	template <typename CLASS, typename DESC_T>
	static QScriptValue DescendantToScriptValue(QScriptEngine *engine, const DESC_T &in)
	{
		return engine->newQObject(new CLASS(in), QScriptEngine::ScriptOwnership);
	}

	template <typename CLASS, typename DESC_T>
	static QScriptValue ScriptObjectConstructor(QScriptEngine *engine, QScriptContext *context, const DESC_T &in)
	{
		return engine->newQObject(context->thisObject(), new CLASS(in), QScriptEngine::ScriptOwnership);
	}

	template <typename CLASS, typename DESC_T>
	static void DescendantFromScriptValue(const QScriptValue &object, DESC_T &out)
	{
		typename CLASS::Inherited temp;
		CLASS::FromScriptValue(object, temp);
		CLASS::convert(temp, out);
	}

	QScriptValue ThrowBadNumberOfArguments(QScriptContext *context);
	QScriptValue IncompatibleArgumentType(QScriptContext *context, int idx);

	template < typename AT
			 , typename RT = AT
			 , typename CVT_TYPE = typename std::remove_const<typename std::remove_reference<AT>::type>::type >
	struct Templates
	{
		template <CVT_TYPE (QScriptValue::*Convert)() const>
		struct In
		{
			template <typename T, RT (T::*Get)() const,
					  void (T::*Set)(AT, bool)>
			static QScriptValue GetterSetter(QScriptContext *context, QScriptEngine *)
			{
				QScriptValue callee = context->callee();
				auto prototype = callee.property("prototype");
				auto data = static_cast<T *>(prototype.toQObject());
				if (context->argumentCount() == 1)
				{
					auto argument = context->argument(0);
					(data->*Set)(CVT_TYPE((argument.*Convert)()), false);
					return argument;
				} else
					return QScriptValue(ST((data->*Get)()));

				return QScriptValue();
			}
		};
	};

	typedef Templates<const QString &>::In<&QScriptValue::toString> StringTemplates;
	typedef Templates<const QString &, QString>::In<&QScriptValue::toString> String2Templates;
	typedef Templates<int>::In<&QScriptValue::toInt32> IntTemplates;
	typedef Templates<bool>::In<&QScriptValue::toBool> BoolTemplates;
	typedef Templates<qreal>::In<&QScriptValue::toNumber> DoubleTemplates;

	template <typename T>
	static QScriptValue QObjectToScriptValue(QScriptEngine *engine, T* const &in)
	{
		return engine->newQObject(in);
	}

	template <typename T>
	static void QObjectFromScriptValue(const QScriptValue &object, T* &out)
	{
		out = qobject_cast<T*>(object.toQObject());
	}

	template <typename FLAGS_T>
	static QScriptValue FlagsToScriptValue(QScriptEngine *engine, const FLAGS_T &in)
	{
		return engine->newVariant(static_cast<qint32>(in.to_ulong()));
	}

	template <typename FLAGS_T>
	static void FlagsFromScriptValue(const QScriptValue &object, FLAGS_T &out)
	{
		out = FLAGS_T(object.toInt32());
	}

	template <typename STRING_T>
	static QScriptValue StringToScriptValue(QScriptEngine *engine, const STRING_T &in)
	{
		return engine->newVariant(QString(in));
	}

	template <typename STRING_T>
	static void StringFromScriptValue(const QScriptValue &object, STRING_T &out)
	{
		out = object.toString();
	}

	template <typename ENUM_T>
	static QScriptValue EnumToScriptValue(QScriptEngine *engine, const ENUM_T &in)
	{
		return engine->newVariant(int(in));
	}

	template <typename ENUM_T>
	static void EnumFromScriptValue(const QScriptValue &object, ENUM_T &out)
	{
		out = ENUM_T(object.toInt32());
	}

	template <typename CLASS, typename DESC_T>
	static QScriptValue ScriptObjectConstructorEx(QScriptEngine *engine, QScriptContext *context)
	{
		DESC_T data;
		switch (context->argumentCount())
		{
			case 1:
			{
				auto argument = context->argument(0);
				if (argument.isQObject())
				{
					auto source = dynamic_cast<CLASS *>(argument.toQObject());
					if (nullptr == source)
						return IncompatibleArgumentType(context, 1);

					data = *source;
				} else
				if (!argument.isNull() && !argument.isUndefined())
					data = DESC_T(argument.toString());

			}	// fall through
			case 0:
				return ScriptObjectConstructor<CLASS, DESC_T>(engine, context, data);
		}

		return ThrowBadNumberOfArguments(context);
	}

	void RegisterQMetaObject(QScriptEngine *engine,
							 const QMetaObject *metaObject,
							 QScriptValue (*constructor)(QScriptContext *, QScriptEngine *));

}
