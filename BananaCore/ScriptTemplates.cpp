#include "ScriptTemplates.h"

#include "ScriptRunner.h"

using namespace Banana;

namespace Scripting
{

	template <typename OUT_T, typename ARG_T,
			  void (OUT_T::*setX)(ARG_T),
			  void (OUT_T::*setY)(ARG_T)>
	static inline void VecFromScriptValue(const QScriptValue &object, OUT_T &out)
	{
		if (object.isQObject())
		{
			auto point = dynamic_cast<Point *>(object.toQObject());
			if (nullptr != point)
			{
				(out.*setX)(point->x());
				(out.*setY)(point->y());
				return;
			}

			auto size = dynamic_cast<Size *>(object.toQObject());
			if (nullptr != size)
			{
				(out.*setX)(size->width());
				(out.*setY)(size->height());
				return;
			}
		} else
		{
			if (std::is_same<int, ARG_T>::value)
			{
				(out.*setX)(object.property(pX).toInt32());
				(out.*setY)(object.property(pY).toInt32());
			} else
			{
				(out.*setX)(object.property(pX).toNumber());
				(out.*setY)(object.property(pY).toNumber());
			}
		}
	}

	template <typename CLASS, typename VEC_T, typename ARG_T,
			  void (VEC_T::*setX)(ARG_T),
			  void (VEC_T::*setY)(ARG_T)>
	static inline QScriptValue VecConstructor(QScriptContext *context, QScriptEngine *engine)
	{
		VEC_T vec;
		if (context->argumentCount() == 1)
		{
			VecFromScriptValue<VEC_T, ARG_T, setX, setY>(context->argument(0), vec);
		} else
		if (context->argumentCount() == 2)
		{
			if (std::is_same<int, ARG_T>::value)
			{
				(vec.*setX)(context->argument(0).toInt32());
				(vec.*setY)(context->argument(1).toInt32());
			} else
			{
				(vec.*setX)(context->argument(0).toNumber());
				(vec.*setY)(context->argument(1).toNumber());
			}
		} else
		if (context->argumentCount() != 0)
			return ThrowBadNumberOfArguments(context);

		return ScriptObjectConstructor<CLASS, VEC_T>(engine, context, vec);
	}

	QScriptValue ThrowBadNumberOfArguments(QScriptContext *context)
	{
		return context->throwError(QScriptContext::UnknownError,
								   ScriptRunner::tr("Bad number of arguments."));
	}

	Font::Font(const QFont &font)
		: QFont(font)
	{
	}

	bool Font::fromString(const QString &str)
	{
		bool result = QFont::fromString(str);

		if (result)
			setStyleStrategy(PreferAntialias);

		return result;
	}

	QScriptValue Font::Constructor(QScriptContext *context, QScriptEngine *engine)
	{
		return ScriptObjectConstructorEx<Font, QFont>(engine, context);
	}

	QScriptValue Font::ToScriptValue(QScriptEngine *engine, const QFont &in)
	{
		return DescendantToScriptValue<Font, QFont>(engine, in);
	}

	void Font::FromScriptValue(const QScriptValue &object, QFont &out)
	{
		auto font = dynamic_cast<Font *>(object.toQObject());

		if (nullptr != font)
			out = *font;
		else
			out = Banana::getDefaultFont();
	}

	QScriptValue IncompatibleArgumentType(QScriptContext *context, int idx)
	{
		return context->throwError(QScriptContext::UnknownError,
								   ScriptRunner::tr("Type of argument #%1 is incompatible.").arg(idx));
	}

	Point::Point(const QPointF &point)
		: QPointF(point.x(), point.y())
	{

	}

	QScriptValue Point::ToScriptValue(QScriptEngine *engine, const QPoint &in)
	{
		return DescendantToScriptValue<Point, QPoint>(engine, in);
	}

	QScriptValue Point::ToScriptValue(QScriptEngine *engine, const QPointF &in)
	{
		return DescendantToScriptValue<Point, QPointF>(engine, in);
	}

	void Point::FromScriptValue(const QScriptValue &object, QPoint &out)
	{
		VecFromScriptValue<QPoint, int, &QPoint::setX, &QPoint::setY>(object, out);
	}

	void Point::FromScriptValue(const QScriptValue &object, QPointF &out)
	{
		VecFromScriptValue<QPointF, qreal, &QPointF::setX, &QPointF::setY>(object, out);
	}

	QScriptValue Point::Constructor(QScriptContext *context, QScriptEngine *engine)
	{
		return VecConstructor<Point, QPoint, int, &QPoint::setX, &QPoint::setY>(context, engine);
	}

	QScriptValue Point::ConstructorF(QScriptContext *context, QScriptEngine *engine)
	{
		return VecConstructor<Point, QPointF, qreal, &QPointF::setX, &QPointF::setY>(context, engine);
	}

	Rect::Rect(const QRectF &rect)
		: QRectF(rect.x(), rect.y(), rect.width(), rect.height())
	{

	}

	bool Rect::containsPoint(qreal x, qreal y) const
	{
		return QRectF::contains(x, y);
	}

	bool Rect::containsRect(qreal x, qreal y, qreal width, qreal height) const
	{
		return QRectF::contains(QRectF(x, y, width, height));
	}

	bool Rect::intersects(qreal x, qreal y, qreal width, qreal height) const
	{
		return QRectF::intersects(QRectF(x, y, width, height));
	}

	QScriptValue Rect::ToScriptValue(QScriptEngine *engine, const QRect &in)
	{
		return DescendantToScriptValue<Rect, QRect>(engine, in);
	}

	QScriptValue Rect::ToScriptValue(QScriptEngine *engine, const QRectF &in)
	{
		return DescendantToScriptValue<Rect, QRectF>(engine, in);
	}

	void Rect::FromScriptValue(const QScriptValue &object, QRect &out)
	{
		out.setX(object.property(pX).toInt32());
		out.setY(object.property(pY).toInt32());
		out.setWidth(object.property(pWidth).toInt32());
		out.setHeight(object.property(pHeight).toInt32());
	}

	void Rect::FromScriptValue(const QScriptValue &object, QRectF &out)
	{
		out.setX(object.property(pX).toNumber());
		out.setY(object.property(pY).toNumber());
		out.setWidth(object.property(pWidth).toNumber());
		out.setHeight(object.property(pHeight).toNumber());
	}

	QScriptValue Rect::Constructor(QScriptContext *context, QScriptEngine *engine)
	{
		QRect rect;
		if (context->argumentCount() == 1)
		{
			FromScriptValue(context->argument(0), rect);
		} else
		if (context->argumentCount() == 4)
		{
			rect.setX(context->argument(0).toInt32());
			rect.setY(context->argument(1).toInt32());
			rect.setWidth(context->argument(2).toInt32());
			rect.setHeight(context->argument(3).toInt32());
		} else
		if (context->argumentCount() != 0)
			return ThrowBadNumberOfArguments(context);

		return ScriptObjectConstructor<Rect, QRect>(engine, context, rect);
	}

	QScriptValue Rect::ConstructorF(QScriptContext *context, QScriptEngine *engine)
	{
		QRectF rect;
		if (context->argumentCount() == 1)
		{
			FromScriptValue(context->argument(0), rect);
		} else
		if (context->argumentCount() == 4)
		{
			rect.setX(context->argument(0).toNumber());
			rect.setY(context->argument(1).toNumber());
			rect.setWidth(context->argument(2).toNumber());
			rect.setHeight(context->argument(3).toNumber());
		} else
		if (context->argumentCount() != 0)
			return ThrowBadNumberOfArguments(context);

		return ScriptObjectConstructor<Rect, QRectF>(engine, context, rect);
	}

	Size::Size(const QSizeF &size)
		: QSizeF(size.width(), size.height())
	{

	}

	QScriptValue Size::ToScriptValue(QScriptEngine *engine, const QSize &in)
	{
		return DescendantToScriptValue<Size, QSize>(engine, in);
	}

	QScriptValue Size::ToScriptValue(QScriptEngine *engine, const QSizeF &in)
	{
		return DescendantToScriptValue<Size, QSizeF>(engine, in);
	}

	void Size::FromScriptValue(const QScriptValue &object, QSize &out)
	{
		VecFromScriptValue<QSize, int, &QSize::setWidth, &QSize::setHeight>(object, out);
	}

	void Size::FromScriptValue(const QScriptValue &object, QSizeF &out)
	{
		VecFromScriptValue<QSizeF, qreal, &QSizeF::setWidth, &QSizeF::setHeight>(object, out);
	}

	QScriptValue Size::Constructor(QScriptContext *context, QScriptEngine *engine)
	{
		return VecConstructor<Size, QSize, int, &QSize::setWidth, &QSize::setHeight>(context, engine);
	}

	QScriptValue Size::ConstructorF(QScriptContext *context, QScriptEngine *engine)
	{
		return VecConstructor<Size, QSizeF, qreal, &QSizeF::setWidth, &QSizeF::setHeight>(context, engine);
	}

	FileInfo::FileInfo(const QFileInfo &info)
		: QFileInfo(info)
	{

	}

	QScriptValue FileInfo::Constructor(QScriptContext *context, QScriptEngine *engine)
	{
		return ScriptObjectConstructorEx<FileInfo, QFileInfo>(engine, context);
	}

	QScriptValue FileInfo::ToScriptValue(QScriptEngine *engine, const QFileInfo &in)
	{
		return DescendantToScriptValue<FileInfo, QFileInfo>(engine, in);
	}

	void FileInfo::FromScriptValue(const QScriptValue &object, QFileInfo &out)
	{
		auto info = dynamic_cast<FileInfo *>(object.toQObject());

		if (nullptr != info)
			out = *info;
		else
			out = QFileInfo();
	}

	Color::Color(const QColor &color)
		: QColor(color)
	{

	}

	QScriptValue Color::Constructor(QScriptContext *context, QScriptEngine *engine)
	{
		int arg_cnt = context->argumentCount();
		switch (arg_cnt)
		{
			case 0:
			case 1:
			{
				if (arg_cnt == 1)
				{
					auto argument = context->argument(0);
					if (argument.isObject())
					{
						auto result = ConstructWith(context, engine,
													argument.property("r"),
													argument.property("g"),
													argument.property("b"),
													argument.property("a"));

						if (result.isQObject())
							return result;

						result = ConstructWith(context, engine,
													argument.property("red"),
													argument.property("green"),
													argument.property("blue"),
													argument.property("alpha"));

						if (result.isQObject())
							return result;
					} else
					if (argument.isNumber())
					{
						QColor color(static_cast<QRgb>(argument.toInt32()));
						return ScriptObjectConstructor<Color, QColor>(engine, context, color);
					}
				}

				return ScriptObjectConstructorEx<Color, QColor>(engine, context);
			}

			case 3:
			case 4:
			{
				auto result = ConstructWith(context, engine,
											context->argument(0),
											context->argument(1),
											context->argument(2),
											context->argument(3));

				if (result.isQObject())
					return result;

				return IncompatibleArgumentType(context,
					!context->argument(0).isNumber() ? 0 :
					!context->argument(1).isNumber() ? 1 :
					!context->argument(2).isNumber() ? 2 : 3);
			}

			default:
				break;
		}

		return ThrowBadNumberOfArguments(context);
	}

	QScriptValue Color::ToScriptValue(QScriptEngine *engine, const QColor &in)
	{
		return DescendantToScriptValue<Color, QColor>(engine, in);
	}

	void Color::FromScriptValue(const QScriptValue &object, QColor &out)
	{
		auto color = dynamic_cast<Color *>(object.toQObject());

		if (nullptr != color)
			out = *color;
		else
			out = QColor();
	}

	QScriptValue Color::ConstructWith(QScriptContext *context,
									  QScriptEngine *engine,
									  const QScriptValue &red,
									  const QScriptValue &green,
									  const QScriptValue &blue,
									  const QScriptValue &alpha)
	{
		if (red.isNumber()
		&&	green.isNumber()
		&&	blue.isNumber())
		{
			QColor color(red.toInt32(), green.toInt32(), blue.toInt32());

			if (alpha.isNumber())
			{
				color.setAlpha(alpha.toInt32());
			}

			return ScriptObjectConstructor<Color, QColor>(engine, context, color);
		}

		return QScriptValue();
	}

	Dir::Dir(const QDir &info)
		: QDir(info)
	{

	}

	QScriptValue Dir::Constructor(QScriptContext *context, QScriptEngine *engine)
	{
		return ScriptObjectConstructorEx<Dir, QDir>(engine, context);
	}

	QScriptValue Dir::ToScriptValue(QScriptEngine *engine, const QDir &in)
	{
		return DescendantToScriptValue<Dir, QDir>(engine, in);
	}

	void Dir::FromScriptValue(const QScriptValue &object, QDir &out)
	{
		auto dir = dynamic_cast<Dir *>(object.toQObject());

		if (nullptr != dir)
			out = *dir;
		else
			out = QDir();
	}

	void RegisterQMetaObject(QScriptEngine *engine,
							 const QMetaObject *metaObject,
							 QScriptValue (*constructor)(QScriptContext *, QScriptEngine *))
	{
		auto currentObject = engine->globalObject();

		QStringList path = QString(metaObject->className()).split("::");

		for (int i = 0, count = path.count(); i < count; i++)
		{
			auto &name = path.at(i);
			auto next = currentObject.property(name);
			if (!next.isValid() || next.isUndefined())
			{
				if (i == count - 1)
				{
					next = engine->newFunction(constructor, engine->newQMetaObject(metaObject));
				} else
				{
					next = engine->newObject();
				}
			}
			currentObject.setProperty(name, next);
			currentObject = next;
		}
	}

	QScriptValue Scripting::Dir::entryList(const QScriptValue &nameFilters,
										   QDir::Filters filters,
										   QDir::SortFlags sort) const
	{
		QScriptValue result;
		if (!nameFilters.isArray())
			return result;

		QStringList list;

		for (int i = 0, count = nameFilters.property("length").toInt32(); i < count; i++)
		{
			list.push_back(nameFilters.property(i).toString());
		}


		list = QDir::entryList(list, filters, sort);

		result = nameFilters.engine()->newArray(list.count());
		for (int i = 0, count = list.count(); i < count; i++)
		{
			result.setProperty(i, list.at(i));
		}

		return result;
	}

}
