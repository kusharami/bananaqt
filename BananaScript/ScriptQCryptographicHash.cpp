/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2018 Alexandra Cherdantseva

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

#include "ScriptQCryptographicHash.h"

#include "ScriptUtils.h"

#include "ScriptQByteArray.h"

#include <QIODevice>

#include <memory>

namespace Banana
{
using namespace Script;

namespace Script
{
struct Hash : public QCryptographicHash
{
	static void RegisterEnums(QScriptEngine *engine, QScriptValue &to)
	{
		static bool convertersRegistered = false;
		if (not convertersRegistered)
		{
			convertersRegistered = true;
			QMetaType::registerConverter<Algorithm, qint32>();
		}

		qScriptRegisterMetaType(engine, EnumToScriptValue<Algorithm>,
			EnumFromScriptValue<Algorithm>);

		SCRIPT_REG_ENUM(to, Md4);
		SCRIPT_REG_ENUM(to, Md5);
		SCRIPT_REG_ENUM(to, Sha1);
		SCRIPT_REG_ENUM(to, Sha224);
		SCRIPT_REG_ENUM(to, Sha256);
		SCRIPT_REG_ENUM(to, Sha384);
		SCRIPT_REG_ENUM(to, Sha512);
		SCRIPT_REG_ENUM(to, Keccak_224);
		SCRIPT_REG_ENUM(to, Keccak_256);
		SCRIPT_REG_ENUM(to, Keccak_384);
		SCRIPT_REG_ENUM(to, Keccak_512);
		SCRIPT_REG_ENUM(to, RealSha3_224);
		SCRIPT_REG_ENUM(to, RealSha3_256);
		SCRIPT_REG_ENUM(to, RealSha3_384);
		SCRIPT_REG_ENUM(to, RealSha3_512);
	}
};
}

struct ScriptQCryptographicHash::SharedData : public QSharedData
{
	QCryptographicHash::Algorithm algorithm;
	std::unique_ptr<QCryptographicHash> hash;

	SharedData();
	SharedData(const SharedData &other);

	void setAlgorithm(QCryptographicHash::Algorithm value);
};

ScriptQCryptographicHash::ScriptQCryptographicHash(QObject *parent)
	: QObject(parent)
{
}

void ScriptQCryptographicHash::Register(QScriptEngine *engine)
{
	qScriptRegisterMetaType(engine, ToScriptValue, FromScriptValue);

	auto proto = NewQObjectPrototype<ScriptQCryptographicHash>(engine);

	auto qHash = engine->newFunction(Constructor, proto);
	qHash.setData(proto);

	Hash::RegisterEnums(engine, qHash);

	qHash.setProperty(
		QSTRKEY(hash), engine->newFunction(hash), STATIC_SCRIPT_VALUE);

	engine->globalObject().setProperty(className(), qHash, STATIC_SCRIPT_VALUE);
}

QCryptographicHash::Algorithm ScriptQCryptographicHash::algorithm() const
{
	auto d = thisData();
	if (d)
	{
		return d->algorithm();
	}

	return QCryptographicHash::Algorithm(-1);
}

void ScriptQCryptographicHash::setAlgorithm(QCryptographicHash::Algorithm value)
{
	auto d = thisData();
	if (d)
	{
		d->setAlgorithm(value);
	}
}

void ScriptQCryptographicHash::reset()
{
	auto data = thisData();
	if (data)
	{
		data->hash().reset();
	}
}

void ScriptQCryptographicHash::addData(const QScriptValue &data)
{
	auto d = thisData();
	if (!d)
	{
		return;
	}

	auto &hash = d->hash();

	if (data.isQObject())
	{
		auto device = qobject_cast<QIODevice *>(data.toQObject());

		if (device)
		{
			hash.addData(device);
			return;
		}
	}

	QByteArray ba;
	ScriptQByteArray::fromScriptValue(data, ba);
	hash.addData(ba);
}

QByteArray ScriptQCryptographicHash::result() const
{
	auto d = thisData();
	if (!d)
	{
		return QByteArray();
	}

	return d->hash().result();
}

QString ScriptQCryptographicHash::toString() const
{
	if (thisData())
	{
		return QString::fromLatin1(result().toHex());
	}

	return className();
}

QByteArray ScriptQCryptographicHash::valueOf() const
{
	return result();
}

QString ScriptQCryptographicHash::className()
{
	return QSTRKEY(QCryptographicHash);
}

ScriptQCryptographicHash::Data *ScriptQCryptographicHash::thisData() const
{
	return qscriptvalue_cast<Data *>(thisObject().data());
}

QScriptValue ScriptQCryptographicHash::hash(
	QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 2)
	{
		return ThrowBadNumberOfArguments(context);
	}

	auto algorithm =
		QCryptographicHash::Algorithm(context->argument(1).toInt32());

	auto arg = context->argument(0);
	if (arg.isQObject())
	{
		auto device = qobject_cast<QIODevice *>(arg.toQObject());

		if (device)
		{
			QCryptographicHash hash(algorithm);
			hash.addData(device);
			return ScriptQByteArray::toScriptValue(engine, hash.result());
		}
	}

	QByteArray ba;
	ScriptQByteArray::fromScriptValue(arg, ba);
	return ScriptQByteArray::toScriptValue(
		engine, QCryptographicHash::hash(ba, algorithm));
}

QScriptValue ScriptQCryptographicHash::Constructor(
	QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 1)
	{
		return ThrowBadNumberOfArguments(context);
	}

	Data data(QCryptographicHash::Algorithm(context->argument(0).toInt32()));
	return ToScriptValue(engine, data);
}

QScriptValue ScriptQCryptographicHash::ToScriptValue(
	QScriptEngine *engine, const Data &in)
{
	auto ctor = engine->globalObject().property(className());

	auto result = engine->newObject();
	result.setData(engine->newVariant(QVariant::fromValue(in)));
	result.setPrototype(ctor.data());
	return result;
}

void ScriptQCryptographicHash::FromScriptValue(
	const QScriptValue &object, Data &out)
{
	auto data = qscriptvalue_cast<Data *>(object.data());
	out = data ? *data : Data(QCryptographicHash::Algorithm(object.toInt32()));
}

ScriptQCryptographicHash::Data::Data()
	: Data(QCryptographicHash::Md5)
{
}

ScriptQCryptographicHash::Data::Data(QCryptographicHash::Algorithm algorithm)
	: d(new SharedData)
{
	d->setAlgorithm(algorithm);
}

ScriptQCryptographicHash::Data::Data(
	const Banana::ScriptQCryptographicHash::Data &other)
	: d(other.d)
{
}

ScriptQCryptographicHash::Data::~Data()
{
	// destruct here
}

ScriptQCryptographicHash::Data &ScriptQCryptographicHash::Data::operator=(
	const Data &other)
{
	d = other.d;
	return *this;
}

QCryptographicHash::Algorithm ScriptQCryptographicHash::Data::algorithm() const
{
	return d->algorithm;
}

void ScriptQCryptographicHash::Data::setAlgorithm(
	QCryptographicHash::Algorithm value)
{
	if (d->algorithm == value)
	{
		return;
	}

	d->setAlgorithm(value);
}

QCryptographicHash &ScriptQCryptographicHash::Data::hash()
{
	return *d->hash.get();
}

ScriptQCryptographicHash::SharedData::SharedData()
{
	setAlgorithm(QCryptographicHash::Md5);
}

ScriptQCryptographicHash::SharedData::SharedData(const SharedData &other)
	: QSharedData(other)
{
	setAlgorithm(other.algorithm);
}

void ScriptQCryptographicHash::SharedData::setAlgorithm(
	QCryptographicHash::Algorithm value)
{
	algorithm = value;
	hash.reset(new QCryptographicHash(value));
}
}
