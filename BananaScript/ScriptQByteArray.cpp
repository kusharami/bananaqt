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

#include "ScriptQByteArray.h"

#include "ScriptUtils.h"

#include <QScriptClassPropertyIterator>
#include <QTextCodec>

Q_DECLARE_METATYPE(Banana::ScriptQByteArray *)

namespace Banana
{
using namespace Script;

class ScriptQByteArray::PropertyIterator : public QScriptClassPropertyIterator
{
public:
	PropertyIterator(const QScriptValue &object);

	virtual bool hasNext() const override;
	virtual void next() override;

	virtual bool hasPrevious() const override;
	virtual void previous() override;

	virtual void toFront() override;
	virtual void toBack() override;

	virtual QScriptString name() const override;
	virtual uint id() const override;

private:
	int m_index;
	int m_last;
};

void ScriptQByteArray::Register(QScriptEngine *engine)
{
	qScriptRegisterMetaType<QByteArray>(engine, ScriptQByteArray::toScriptValue,
		ScriptQByteArray::fromScriptValue);

	auto cls = new ScriptQByteArray(engine);
	auto proto = NewQObjectPrototype(engine, cls);
	cls->proto = proto;

	QScriptValue global = engine->globalObject();
	proto.setPrototype(
		global.property(QSTRKEY(Object)).property(QSTRKEY(prototype)));

	proto.setProperty(QSTRKEY(concat), engine->newFunction(concat));
	proto.setProperty(QSTRKEY(push), engine->newFunction(push));
	proto.setProperty(QSTRKEY(splice), engine->newFunction(splice));
	proto.setProperty(QSTRKEY(unshift), engine->newFunction(unshift));

	auto ctor = engine->newFunction(ScriptQByteArray::construct, proto);
	ctor.setData(engine->toScriptValue(cls));
	ctor.setProperty(QSTRKEY(fromBase64), engine->newFunction(fromBase64));
	ctor.setProperty(QSTRKEY(fromHex), engine->newFunction(fromHex));

	global.setProperty(className(), ctor);
}

ScriptQByteArray::ScriptQByteArray(QScriptEngine *engine)
	: QObject(engine)
	, QScriptClass(engine)
{
	length = engine->toStringHandle(QSTRKEY(length));
}

QScriptValue ScriptQByteArray::newInstance(int size, char init)
{
	engine()->reportAdditionalMemoryCost(size);
	return newInstance(QByteArray(size, init));
}

QScriptValue ScriptQByteArray::newInstance(const QByteArray &ba)
{
	QScriptEngine *engine = this->engine();
	QScriptValue data = engine->newVariant(QVariant::fromValue(ba));
	return engine->newObject(this, data);
}

QScriptClass::QueryFlags ScriptQByteArray::queryProperty(
	const QScriptValue &object, const QScriptString &name, QueryFlags flags,
	uint *id)
{
	QByteArray *ba = thisByteArray(object);
	if (!ba)
		return 0;

	if (name == length)
	{
		return flags;
	}

	bool isArrayIndex;
	int pos = int(name.toArrayIndex(&isArrayIndex));
	if (not isArrayIndex)
		return 0;

	*id = uint(pos);
	if (0 != (flags & HandlesReadAccess) && pos >= ba->size())
		flags &= ~HandlesReadAccess;
	return flags;
}

QScriptValue ScriptQByteArray::property(
	const QScriptValue &object, const QScriptString &name, uint id)
{
	QByteArray *ba = thisByteArray(object);
	if (!ba)
		return QScriptValue();

	if (name == length)
	{
		return ba->length();
	}

	int pos = int(id);
	if (pos < 0 || pos >= ba->size())
		return QScriptValue();

	return QScriptValue(engine(), int(ba->at(pos)));
}

void ScriptQByteArray::setProperty(QScriptValue &object,
	const QScriptString &name, uint id, const QScriptValue &value)
{
	QByteArray *ba = thisByteArray(object);
	if (!ba)
		return;

	if (name == length)
	{
		resize(*ba, value.toInt32());
	} else
	{
		int pos = int(id);
		if (pos < 0)
			return;

		if (ba->size() <= pos)
			resize(*ba, pos + 1);

		(*ba)[pos] = scriptValueToChar(value);
	}
}

QScriptValue::PropertyFlags ScriptQByteArray::propertyFlags(
	const QScriptValue &, const QScriptString &name, uint)
{
	if (name == length)
	{
		return QScriptValue::Undeletable | QScriptValue::SkipInEnumeration;
	}

	return QScriptValue::Undeletable;
}

QScriptClassPropertyIterator *ScriptQByteArray::newIterator(
	const QScriptValue &object)
{
	return new PropertyIterator(object);
}

QString ScriptQByteArray::name() const
{
	return className();
}

QString ScriptQByteArray::className()
{
	return QSTRKEY(QByteArray);
}

QScriptValue ScriptQByteArray::prototype() const
{
	return proto;
}

QByteArray *ScriptQByteArray::thisByteArray(QScriptContext *context)
{
	Q_ASSERT(nullptr != context);
	return thisByteArray(context->thisObject());
}

QByteArray *ScriptQByteArray::thisByteArray(const QScriptValue &value)
{
	return qscriptvalue_cast<QByteArray *>(value.data());
}

QTextCodec *ScriptQByteArray::scriptValueToTextCodec(const QScriptValue &value)
{
	QByteArray codecBA;
	fromScriptValue(value, codecBA);

	auto codec = QTextCodec::codecForName(codecBA);
	if (nullptr == codec)
		codec = QTextCodec::codecForLocale();

	return codec;
}

QScriptValue ScriptQByteArray::fromBase64(
	QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 1)
		return ThrowBadNumberOfArguments(context);

	QByteArray ba;
	fromScriptValue(context->argument(0), ba);

	ba = QByteArray::fromBase64(ba);
	return toScriptValue(engine, ba);
}

QScriptValue ScriptQByteArray::fromHex(
	QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 1)
		return ThrowBadNumberOfArguments(context);

	QByteArray ba;
	fromScriptValue(context->argument(0), ba);

	ba = QByteArray::fromHex(ba);
	return toScriptValue(engine, ba);
}

QScriptValue ScriptQByteArray::concat(
	QScriptContext *context, QScriptEngine *engine)
{
	auto ba = thisByteArray(context);

	if (nullptr == ba)
		return QScriptValue();

	QByteArray result = *ba;

	for (int i = 0, count = context->argumentCount(); i < count; i++)
	{
		fromScriptValue(context->argument(i), result);
	}

	if (!result.isEmpty())
		engine->reportAdditionalMemoryCost(result.size());

	return toScriptValue(engine, result);
}

QScriptValue ScriptQByteArray::push(
	QScriptContext *context, QScriptEngine *engine)
{
	auto ba = thisByteArray(context);
	if (nullptr == ba)
		return QScriptValue();

	int count = context->argumentCount();
	int prevSize = ba->size();
	ba->resize(prevSize + count);
	char *ptr = &ba->data()[prevSize];
	for (int i = 0; i < count; i++, ptr++)
	{
		*ptr = scriptValueToChar(context->argument(i));
	}

	if (count > 0)
		engine->reportAdditionalMemoryCost(count);

	return QScriptValue(engine, ba->length());
}

QScriptValue ScriptQByteArray::unshift(
	QScriptContext *context, QScriptEngine *engine)
{
	auto ba = thisByteArray(context);
	if (nullptr == ba)
		return QScriptValue();

	int count = context->argumentCount();
	ba->prepend(count, 0);
	char *ptr = ba->data();
	for (int i = count - 1; i >= 0; i--, ptr++)
	{
		*ptr = scriptValueToChar(context->argument(i));
	}

	if (count > 0)
		engine->reportAdditionalMemoryCost(count);

	return QScriptValue(engine, ba->length());
}

QScriptValue ScriptQByteArray::splice(
	QScriptContext *context, QScriptEngine *engine)
{
	auto ba = thisByteArray(context);
	if (nullptr == ba)
		return QScriptValue();

	int start =
		ScriptQByteArray::convertIndex(*ba, context->argument(0).toInt32());

	int argCount = context->argumentCount();

	int deleteCount = 0;
	if (argCount >= 2)
		deleteCount = context->argument(1).toInt32();

	ba->remove(start, deleteCount);

	int memoryCost = -deleteCount;

	if (argCount > 2)
	{
		QByteArray toInsert(argCount - 2, Qt::Uninitialized);

		for (int i = 2; i < argCount; i++)
		{
			toInsert[i - 2] = scriptValueToChar(context->argument(i));
		}

		ba->insert(start, toInsert);

		memoryCost += toInsert.size();
	}

	if (memoryCost > 0)
		engine->reportAdditionalMemoryCost(memoryCost);

	return context->thisObject();
}

char ScriptQByteArray::scriptValueToChar(const QScriptValue &value, bool *okPtr)
{
	bool ok = false;
	int ch = 0;

	if (value.isNumber() || value.isBool() || value.isNull() ||
		value.isUndefined())
	{
		ch = value.toInt32();
		ok = true;
	} else
	{
		auto str = value.toString();
		int i = str.toInt(&ok);
		if (ok)
		{
			ch = i;
		} else
		{
			auto latin1 = str.toLatin1();
			if (latin1.size() == 1)
			{
				ch = latin1.at(0);
				ok = true;
			}
		}
	}

	if (nullptr != okPtr)
		*okPtr = ok;

	return char(ch);
}

QScriptValue ScriptQByteArray::construct(QScriptContext *ctx, QScriptEngine *)
{
	ScriptQByteArray *cls =
		qscriptvalue_cast<ScriptQByteArray *>(ctx->callee().data());
	if (!cls)
		return QScriptValue();

	QScriptValue arg = ctx->argument(0);
	if (arg.instanceOf(ctx->callee()))
		return cls->newInstance(qscriptvalue_cast<QByteArray>(arg));

	if (arg.isArray())
	{
		QByteArray ba;
		fromScriptValue(arg, ba);
		return cls->newInstance(ba);
	}

	auto arg2 = ctx->argument(1);

	if (!arg.isNumber() && !arg.isUndefined())
	{
		auto codec = scriptValueToTextCodec(arg2);
		return cls->newInstance(codec->fromUnicode(arg.toString()));
	}

	int size = arg.toInt32();
	char init = scriptValueToChar(arg2);
	return cls->newInstance(size, init);
}

QScriptValue ScriptQByteArray::toScriptValue(
	QScriptEngine *eng, const QByteArray &ba)
{
	QScriptValue ctor = eng->globalObject().property(className());
	ScriptQByteArray *cls = qscriptvalue_cast<ScriptQByteArray *>(ctor.data());
	Q_ASSERT(nullptr != cls);

	return cls->newInstance(ba);
}

void ScriptQByteArray::fromScriptValue(
	const QScriptValue &value, QByteArray &out)
{
	if (value.isArray())
	{
		int length = value.property(QSTRKEY(length)).toInt32();
		int prevSize = out.size();
		out.resize(prevSize + length);
		char *ptr = &out.data()[prevSize];
		for (int j = 0; j < length; j++, ptr++)
		{
			*ptr = scriptValueToChar(value.property(j));
		}
	} else
	{
		QByteArray tempBa;
		auto ba = thisByteArray(value);
		if (nullptr != ba)
			tempBa = *ba;

		if (ba == nullptr)
		{
			tempBa = value.toString().toLocal8Bit();
		}

		if (not out.isEmpty())
			out.append(tempBa);
		else
			out = tempBa;
	}
}

void ScriptQByteArray::resize(QByteArray &ba, int newSize)
{
	int oldSize = ba.size();
	ba.resize(newSize);
	if (newSize > oldSize)
		engine()->reportAdditionalMemoryCost(newSize - oldSize);
}

ScriptQByteArray::PropertyIterator::PropertyIterator(const QScriptValue &object)
	: QScriptClassPropertyIterator(object)
{
	toFront();
}

bool ScriptQByteArray::PropertyIterator::hasNext() const
{
	QByteArray *ba = thisByteArray(object());
	return m_index < ba->size();
}

void ScriptQByteArray::PropertyIterator::next()
{
	m_last = m_index;
	++m_index;
}

bool ScriptQByteArray::PropertyIterator::hasPrevious() const
{
	return (m_index > 0);
}

void ScriptQByteArray::PropertyIterator::previous()
{
	--m_index;
	m_last = m_index;
}

void ScriptQByteArray::PropertyIterator::toFront()
{
	m_index = 0;
	m_last = -1;
}

void ScriptQByteArray::PropertyIterator::toBack()
{
	QByteArray *ba = thisByteArray(object());
	m_index = ba->size();
	m_last = -1;
}

QScriptString ScriptQByteArray::PropertyIterator::name() const
{
	return object().engine()->toStringHandle(QString::number(m_last));
}

uint ScriptQByteArray::PropertyIterator::id() const
{
	return uint(m_last);
}

void ScriptQByteArray::clear()
{
	auto ba = thisByteArray();
	if (ba)
		ba->clear();
}

void ScriptQByteArray::chop(int n)
{
	auto ba = thisByteArray();
	if (ba)
		ba->chop(n);
}

QByteArray ScriptQByteArray::repeated(int times) const
{
	auto ba = thisByteArray();
	if (ba)
		return ba->repeated(times);

	return QByteArray();
}

QScriptValue ScriptQByteArray::split(const QScriptValue &separator)
{
	auto ba = thisByteArray();
	if (!ba)
		return QScriptValue();

	bool ok;
	char sep = scriptValueToChar(separator, &ok);
	if (not ok)
		return ThrowIncompatibleArgumentType(context(), 1);

	auto list = ba->split(sep);

	auto count = quint32(list.size());
	QScriptValue result = engine()->newArray(count);

	for (quint32 i = 0; i < count; i++)
	{
		result.setProperty(i, newInstance(list.at(int(i))));
	}

	return result;
}

QByteArray ScriptQByteArray::replace(
	const QScriptValue &what, const QScriptValue &to) const
{
	auto ba = thisByteArray();
	if (ba)
	{
		return ba->replace(
			scriptValueToByteArray(what), scriptValueToByteArray(to));
	}

	return QByteArray();
}

QByteArray ScriptQByteArray::replace(
	int index, int len, const QScriptValue &to) const
{
	auto ba = thisByteArray();
	if (nullptr == ba)
		return QByteArray();

	if (len < 0)
	{
		index -= len;
		len = -len;
	}

	if (index > ba->size())
	{
		index = ba->size();
		len = 0;
	}

	if (index < 0)
	{
		len += index;
		index = 0;
	}

	if (len < 0)
		len = 0;

	return ba->replace(index, len, scriptValueToByteArray(to));
}

bool ScriptQByteArray::equals(const QByteArray &other)
{
	auto ba = thisByteArray();
	if (ba)
		return *ba == other;

	return false;
}

QByteArray ScriptQByteArray::left(int len) const
{
	auto ba = thisByteArray();
	if (ba)
		return ba->left(len);

	return QByteArray();
}

QByteArray ScriptQByteArray::mid(int pos, int len) const
{
	auto ba = thisByteArray();
	if (ba)
		return ba->mid(pos, len);

	return QByteArray();
}

QScriptValue ScriptQByteArray::remove(int pos, int len)
{
	auto ba = thisByteArray();
	if (ba)
		ba->remove(pos, len);

	return thisObject();
}

QByteArray ScriptQByteArray::right(int len) const
{
	auto ba = thisByteArray();
	if (ba)
		return ba->right(len);

	return QByteArray();
}

QByteArray ScriptQByteArray::simplified() const
{
	auto ba = thisByteArray();
	if (ba)
		return ba->simplified();

	return QByteArray();
}

QByteArray ScriptQByteArray::toBase64() const
{
	auto ba = thisByteArray();
	if (ba)
		return ba->toBase64();

	return QByteArray();
}

QByteArray ScriptQByteArray::toHex() const
{
	auto ba = thisByteArray();
	if (ba)
		return ba->toHex();

	return QByteArray();
}

QByteArray ScriptQByteArray::toLower() const
{
	auto ba = thisByteArray();
	if (ba)
		return ba->toLower();

	return QByteArray();
}

QByteArray ScriptQByteArray::toUpper() const
{
	auto ba = thisByteArray();
	if (ba)
		return ba->toUpper();

	return QByteArray();
}

QByteArray ScriptQByteArray::trimmed() const
{
	auto ba = thisByteArray();
	if (ba)
		return ba->trimmed();

	return QByteArray();
}

void ScriptQByteArray::truncate(int pos)
{
	auto ba = thisByteArray();
	if (ba)
		return ba->truncate(pos);
}

QString ScriptQByteArray::toString() const
{
	auto ba = thisByteArray();
	if (nullptr == ba)
		return className();

	return QStringLiteral("%1(length=%2)").arg(className()).arg(ba->size());
}

QString ScriptQByteArray::toStringLatin() const
{
	auto ba = thisByteArray();
	if (ba)
		return QString::fromLatin1(*ba);

	return QString();
}

QString ScriptQByteArray::toStringLocal() const
{
	return toString(QByteArray());
}

QString ScriptQByteArray::toString(const QByteArray &codecName) const
{
	auto ba = thisByteArray();
	if (nullptr == ba)
		return QString();

	auto codec = QTextCodec::codecForName(codecName);
	if (nullptr == codec)
		codec = QTextCodec::codecForLocale();

	return codec->toUnicode(*ba);
}

QString ScriptQByteArray::join(const QString &separator) const
{
	QString result;

	auto ba = thisByteArray();

	if (nullptr != ba)
	{
		for (char c : *ba)
		{
			if (not result.isEmpty())
				result += separator;
			result += QString::number(int(c));
		}
	}

	return result;
}

QScriptValue ScriptQByteArray::pop()
{
	QScriptValue result;

	auto ba = thisByteArray();
	if (ba)
	{
		int len = ba->length();
		if (len > 0)
		{
			result = QScriptValue(engine(), int(ba->at(len - 1)));

			ba->chop(1);
		}
	}

	return result;
}

QScriptValue ScriptQByteArray::reverse()
{
	auto ba = thisByteArray();

	if (ba)
		std::reverse(ba->begin(), ba->end());

	return thisObject();
}

QScriptValue ScriptQByteArray::shift()
{
	QScriptValue result;

	auto ba = thisByteArray();
	if (ba)
	{
		int len = ba->length();
		if (len > 0)
		{
			result = QScriptValue(engine(), int(ba->at(0)));

			ba->remove(0, 1);
		}
	}

	return result;
}

QByteArray ScriptQByteArray::slice(int start, int end) const
{
	QByteArray result;

	auto ba = thisByteArray();
	if (ba)
	{
		start = convertIndex(*ba, start);
		end = convertIndex(*ba, end);

		int len = end - start;

		if (len > 0)
		{
			result = ba->mid(start, len);

			engine()->reportAdditionalMemoryCost(len);
		}
	}

	return result;
}

QScriptValue ScriptQByteArray::sort(QScriptValue compareFn)
{
	auto ba = thisByteArray();
	if (ba)
	{
		if (compareFn.isValid())
		{
			if (not compareFn.isFunction())
				return ThrowIncompatibleArgumentType(context(), 1);

			auto engine = this->engine();
			try
			{
				std::sort(ba->begin(), ba->end(),
					[engine, &compareFn](char a, char b) mutable -> bool {
						QScriptValueList args;
						args.reserve(2);
						args.append(int(a));
						args.append(int(b));
						auto result = compareFn.call(QScriptValue(), args);

						if (engine->hasUncaughtException())
						{
							throw std::exception();
						}

						return result.toBool();
					});
			} catch (...)
			{
				if (engine->hasUncaughtException())
					return engine->uncaughtException();

				return ThrowUnknownError(engine->currentContext());
			}
		} else
		{
			std::sort(ba->begin(), ba->end());
		}
	}

	return thisObject();
}

int ScriptQByteArray::indexOf(const QScriptValue &search, int fromIndex) const
{
	auto ba = thisByteArray();
	if (nullptr == ba || search.isUndefined() || !search.isValid())
		return -1;

	fromIndex = convertIndex(*ba, fromIndex);
	auto searchBa = thisByteArray(search);
	if (searchBa)
		return ba->indexOf(*searchBa, fromIndex);

	bool ok;

	char ch = ScriptQByteArray::scriptValueToChar(search, &ok);
	if (ok)
		return ba->indexOf(ch, fromIndex);

	return -1;
}

int ScriptQByteArray::lastIndexOf(
	const QScriptValue &search, int fromIndex) const
{
	auto ba = thisByteArray();
	if (nullptr == ba || search.isUndefined() || !search.isValid())
		return -1;

	fromIndex = convertIndex(*ba, fromIndex);
	auto searchBa = thisByteArray(search);
	if (searchBa)
		return ba->lastIndexOf(*searchBa, fromIndex);

	bool ok;

	char ch = ScriptQByteArray::scriptValueToChar(search, &ok);
	if (ok)
		return ba->lastIndexOf(ch, fromIndex);

	return -1;
}

bool ScriptQByteArray::every(
	QScriptValue callback, const QScriptValue &thisObject) const
{
	auto ba = thisByteArray();
	if (nullptr == ba)
		return false;

	auto engine = this->engine();
	if (not callback.isFunction())
	{
		ThrowIncompatibleArgumentType(engine->currentContext(), 1);
		return false;
	}

	int count = ba->length();

	if (count > 0)
	{
		int checkCount = 0;

		for (int i = 0; i < count; i++)
		{
			QScriptValueList args;
			args.reserve(3);
			args.append(int(ba->at(i)));
			args.append(i);
			args.append(this->thisObject());
			auto result = callback.call(thisObject, args);
			if (engine->hasUncaughtException())
				return false;

			if (result.toBool())
				checkCount++;
		}

		return checkCount == count;
	}

	return false;
}

bool ScriptQByteArray::some(
	QScriptValue callback, const QScriptValue &thisObject) const
{
	auto ba = thisByteArray();
	if (nullptr == ba)
		return false;

	auto engine = this->engine();
	if (not callback.isFunction())
	{
		ThrowIncompatibleArgumentType(engine->currentContext(), 1);
		return false;
	}

	int count = ba->length();

	for (int i = 0; i < count; i++)
	{
		QScriptValueList args;
		args.reserve(3);
		args.append(int(ba->at(i)));
		args.append(i);
		args.append(this->thisObject());
		auto result = callback.call(thisObject, args);
		if (engine->hasUncaughtException())
			break;

		if (result.toBool())
			return true;
	}

	return false;
}

void ScriptQByteArray::forEach(
	QScriptValue callback, const QScriptValue &thisObject) const
{
	auto ba = thisByteArray();
	if (nullptr == ba)
		return;

	auto engine = this->engine();
	if (not callback.isFunction())
	{
		ThrowIncompatibleArgumentType(engine->currentContext(), 1);
		return;
	}

	int count = ba->length();

	for (int i = 0; i < count; i++)
	{
		QScriptValueList args;
		args.reserve(3);
		args.append(int(ba->at(i)));
		args.append(i);
		args.append(this->thisObject());
		callback.call(thisObject, args);
		if (engine->hasUncaughtException())
			break;
	}
}

QScriptValue ScriptQByteArray::map(
	QScriptValue callback, const QScriptValue &thisObject) const
{
	auto ba = thisByteArray();
	if (nullptr == ba)
		return QScriptValue();

	auto engine = this->engine();
	if (not callback.isFunction())
	{
		return ThrowIncompatibleArgumentType(engine->currentContext(), 1);
	}

	int count = ba->length();

	QScriptValue result = engine->newArray(count);

	for (int i = 0; i < count; i++)
	{
		QScriptValueList args;
		args.reserve(3);
		args.append(int(ba->at(i)));
		args.append(i);
		args.append(this->thisObject());
		result.setProperty(i, callback.call(thisObject, args));
		if (engine->hasUncaughtException())
			return engine->uncaughtException();
	}

	return result;
}

QByteArray ScriptQByteArray::mapBytes(
	QScriptValue callback, const QScriptValue &thisObject) const
{
	auto ba = thisByteArray();
	if (nullptr == ba)
		return QByteArray();

	auto engine = this->engine();
	if (not callback.isFunction())
	{
		ThrowIncompatibleArgumentType(engine->currentContext(), 1);
		return QByteArray();
	}

	int count = ba->length();
	QByteArray result(count, 0);

	for (int i = 0; i < count; i++)
	{
		QScriptValueList args;
		args.reserve(3);
		args.append(int(ba->at(i)));
		args.append(i);
		args.append(this->thisObject());
		result[i] = ScriptQByteArray::scriptValueToChar(
			callback.call(thisObject, args));
		if (engine->hasUncaughtException())
			break;
	}

	if (!result.isEmpty())
		engine->reportAdditionalMemoryCost(result.size());

	return result;
}

QByteArray ScriptQByteArray::filter(
	QScriptValue callback, const QScriptValue &thisObject) const
{
	QByteArray result;
	auto ba = thisByteArray();
	if (nullptr == ba)
		return result;

	auto engine = this->engine();
	if (not callback.isFunction())
	{
		ThrowIncompatibleArgumentType(engine->currentContext(), 1);
		return result;
	}

	int count = ba->length();
	result.reserve(count);
	for (int i = 0; i < count; i++)
	{
		QScriptValueList args;
		args.reserve(3);
		args.append(int(ba->at(i)));
		args.append(i);
		args.append(this->thisObject());
		auto ok = callback.call(thisObject, args);
		if (engine->hasUncaughtException())
			break;
		if (ok.toBool())
			result.push_back(ba->at(i));
	}
	result.squeeze();

	if (!result.isEmpty())
		engine->reportAdditionalMemoryCost(result.size());

	return result;
}

QScriptValue ScriptQByteArray::reduce(
	QScriptValue callback, const QScriptValue &initialValue) const
{
	auto ba = thisByteArray();
	if (nullptr == ba)
		return QScriptValue();

	auto engine = this->engine();

	if (not callback.isFunction())
		return ThrowIncompatibleArgumentType(engine->currentContext(), 1);

	QScriptValue result(engine, initialValue.toNumber());

	int count = ba->length();

	for (int i = 0; i < count; i++)
	{
		QScriptValueList args;
		args.reserve(4);
		args.append(result);
		args.append(int(ba->at(i)));
		args.append(i);
		args.append(this->thisObject());
		result = callback.call(QScriptValue(), args);
		if (engine->hasUncaughtException())
			return engine->uncaughtException();
	}

	return result;
}

QScriptValue ScriptQByteArray::reduceRight(
	QScriptValue callback, const QScriptValue &initialValue) const
{
	auto ba = thisByteArray();
	if (nullptr == ba)
		return QScriptValue();

	auto engine = this->engine();
	if (not callback.isFunction())
		return ThrowIncompatibleArgumentType(engine->currentContext(), 1);

	QScriptValue result(engine, initialValue.toNumber());

	int count = ba->length();

	for (int i = count - 1; i >= 0; i--)
	{
		QScriptValueList args;
		args.reserve(4);
		args.append(result);
		args.append(int(ba->at(i)));
		args.append(i);
		args.append(this->thisObject());
		result = callback.call(QScriptValue(), args);
		if (engine->hasUncaughtException())
			return engine->uncaughtException();
	}

	return result;
}

QByteArray ScriptQByteArray::scriptValueToByteArray(const QScriptValue &value)
{
	auto pb = thisByteArray(value);

	return (pb ? QByteArray::fromRawData(pb->data(), pb->size())
			   : value.isNumber() || value.isNull() || value.isUndefined() ||
					value.isBool()
				? QByteArray(1, char(value.toInt32()))
				: value.toString().toUtf8());
}

int ScriptQByteArray::convertIndex(const QByteArray &ba, int index)
{
	if (index >= 0)
	{
		return qMin(index, ba.length());
	}

	return qMax(0, ba.length() + index);
}

QByteArray *ScriptQByteArray::thisByteArray() const
{
	return thisByteArray(thisObject());
}
}
