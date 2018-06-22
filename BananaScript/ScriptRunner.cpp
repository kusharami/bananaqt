/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2016-2017 Alexandra Cherdantseva

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

#include "ScriptRunner.h"

#include "BananaCore/ProjectGroup.h"
#include "BananaCore/AbstractProjectDirectory.h"
#include "BananaCore/AbstractProjectFile.h"
#include "BananaCore/Utils.h"
#include "ScriptUtils.h"
#include "ScriptManager.h"
#include "Config.h"
#include "ScriptQPoint.h"
#include "ScriptQSize.h"
#include "ScriptQRect.h"
#include "ScriptQFont.h"
#include "ScriptQColor.h"
#include "ScriptQByteArray.h"
#include "ScriptQFileInfo.h"
#include "ScriptQDir.h"
#include "ScriptQSaveFile.h"
#include "ScriptQTemporaryFile.h"
#include "ScriptQUrl.h"
#include "ScriptQNetworkRequest.h"
#include "ScriptQNetworkAccessManager.h"
#include "ScriptQNetworkReply.h"
#include "ScriptQCryptographicHash.h"

#include <QScriptEngine>
#include <QScriptContextInfo>
#include <QCoreApplication>
#include <QStandardPaths>

namespace Banana
{
namespace Script
{
struct Temp
{
	static inline void init()
	{
		(void) QT_TRANSLATE_NOOP("FileFormatNamePlural", ".js");
	}
};

static const char szScriptAssert[] =
	QT_TRANSLATE_NOOP("Script", "Assertion failed.");
const char szScriptExtension[] = QT_TRANSLATE_NOOP("FileFormatName", ".js");
}
using namespace Script;

#undef assert
static QScriptValue assert(QScriptContext *context, QScriptEngine *)
{
	if (context->argumentCount() != 1)
		return ThrowBadNumberOfArguments(context);

	if (not context->argument(0).toBool())
	{
		return context->throwError(
			QCoreApplication::translate("Script", szScriptAssert));
	}
	return QScriptValue();
}

static QScriptValue copyProperties(
	QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 2)
	{
		return ThrowBadNumberOfArguments(context);
	}

	auto to = context->argument(1);
	CopyScriptProperties(context->argument(0), to);

	return QScriptValue(engine, QScriptValue::UndefinedValue);
}

static QScriptValue evaluateFileContent(
	QScriptEngine *engine, const QString &fileContent, const QString &filePath)
{
	Q_ASSERT(nullptr != engine);

	return engine->evaluate(fileContent, QDir::toNativeSeparators(filePath));
}

inline void internalEvalScript(
	ScriptRunner *runner, const QString &filePath, QScriptValue &out)
{
	auto engine = runner->activeEngine;
	if (nullptr == engine)
		return;

	auto context = engine->currentContext();

	QString resolvedFilePath;
	{
		QScriptContextInfo info(context->parentContext());
		QFileInfo currentFileInfo(info.fileName());
		if (currentFileInfo.exists())
		{
			resolvedFilePath = currentFileInfo.dir().absoluteFilePath(filePath);
		}
	}

	if (not QFile::exists(resolvedFilePath))
	{
		auto activeProjectDir =
			runner->projectGroup->getActiveProjectDirectory();
		if (nullptr != activeProjectDir)
		{
			resolvedFilePath =
				activeProjectDir->getAbsoluteFilePathFor(filePath, false);
		}
	}

	runner->importScript(resolvedFilePath);
	if (engine->hasUncaughtException())
		out = engine->uncaughtException();
}

static QString stringFormat(QScriptContext *context)
{
	QString text;

	int arg_cnt = context->argumentCount();

	if (arg_cnt >= 1)
	{
		text = context->argument(0).toString();

		for (int i = 1; i < arg_cnt; i++)
		{
			text = text.arg(context->argument(i).toString());
		}
	}

	return text;
}

static QScriptValue stringFormat(QScriptContext *context, QScriptEngine *e)
{
	if (context->argumentCount() < 2)
		return ThrowBadNumberOfArguments(context);

	return QScriptValue(e, stringFormat(context));
}

static QScriptValue macro(QScriptContext *context, QScriptEngine *)
{
	int argc = context->argumentCount();
	if (argc < 2)
		return ThrowBadNumberOfArguments(context);

	int i = 0;
	auto arg = context->argument(0);

	QString macroName;

	if (arg.isString())
	{
		macroName = arg.toString();
		i++;
		arg = context->argument(i);
	}

	if (macroName.isEmpty())
		macroName = ScriptManager::scriptedActionCaption();

	i++;

	if (not arg.isQObject())
		return ThrowIncompatibleArgumentType(context, i);

	auto obj = dynamic_cast<Object *>(arg.toQObject());
	if (nullptr == obj)
		return ThrowIncompatibleArgumentType(context, i);

	if (i >= argc)
		return ThrowBadNumberOfArguments(context);

	auto callback = context->argument(i);

	i++;

	if (not callback.isFunction())
		return ThrowIncompatibleArgumentType(context, i);

	obj->beginMacro(macroName);

	arg = callback.call(arg);

	obj->endMacro();

	return arg;
}

static QScriptValue internalImportScript(
	QScriptContext *context, QScriptEngine *e)
{
	if (context->argumentCount() < 1)
		return ThrowBadNumberOfArguments(context);

	Q_ASSERT(e->currentContext() == context);

	auto callee = context->callee();

	auto prototype = callee.property(QSTRKEY(prototype));
	auto runner = qobject_cast<ScriptRunner *>(prototype.toQObject());
	if (nullptr == runner)
	{
		return ThrowIncompatibleArgumentType(context, 0);
	}

	auto arg = context->argument(0);
	if (not arg.isString())
	{
		return ThrowIncompatibleArgumentType(context, 1);
	}

	QScriptValue result;
	internalEvalScript(runner, arg.toString(), result);

	return result;
}

static QScriptValue print(QScriptContext *context, QScriptEngine *e)
{
	auto callee = context->callee();
	auto prototype = callee.property(QSTRKEY(prototype));
	auto runner = qobject_cast<ScriptRunner *>(prototype.toQObject());
	if (nullptr == runner)
	{
		return ThrowIncompatibleArgumentType(context, 0);
	}

	QString text(stringFormat(context));

	runner->log(text);

	return QScriptValue(e, text);
}

static QScriptValue loadFromJson(QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() == 1)
	{
		QVariantMap vmap;
		bool ok = Utils::LoadVariantMapFromFile(
			vmap, context->argument(0).toString());
		if (ok)
			return VariantToScriptValue(vmap, engine);
	}

	return QScriptValue();
}

static QScriptValue saveToJson(QScriptContext *context, QScriptEngine *)
{
	if (context->argumentCount() == 2)
	{
		auto v = context->argument(0).toVariant();
		if (v.type() == QVariant::Map)
		{
			bool ok = Utils::SaveVariantMapToFile(
				v.toMap(), context->argument(1).toString());

			return QScriptValue(ok);
		}
	}

	return QScriptValue();
}

static QScriptValue loadTextFromFile(
	QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() == 1)
	{
		QString text;
		bool ok =
			Utils::LoadTextFromFile(text, context->argument(0).toString());
		if (ok)
			return QScriptValue(engine, text);
	}

	return QScriptValue();
}

static QScriptValue saveTextToFile(
	QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() == 2)
	{
		bool ok = Utils::SaveTextToFile(
			context->argument(0).toString(), context->argument(1).toString());

		return QScriptValue(engine, ok);
	}

	return QScriptValue();
}

static QScriptValue loadFileTree(QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() == 2)
	{
		auto result = context->argument(1);

		if (result.isArray())
		{
			int length = result.property(QSTRKEY(length)).toInt32();

			QStringList lst;

			for (int i = 0; i < length; i++)
			{
				lst.push_back(result.property(i).toString());
			}

			lst = Utils::ListDirectoryContents(
				context->argument(0).toString(), lst);

			length = lst.size();

			result = engine->newArray(length);

			for (int i = 0; i < length; i++)
			{
				result.setProperty(i, lst.at(i));
			}

			return result;
		}
	}

	return QScriptValue();
}

ScriptRunner::ScriptRunner(ProjectGroup *projectGroup, QObject *parent)
	: QObject(parent)
	, projectGroup(projectGroup)
	, activeEngine(nullptr)
	, mProcessEventsInterval(40)
{
}

bool ScriptRunner::abort()
{
	if (activeEngine)
	{
		activeEngine->abortEvaluation(
			activeEngine->currentContext()->throwValue(
				QScriptValue(activeEngine, 1)));
		activeEngine = nullptr;
	}

	return true;
}

bool ScriptRunner::isRunning() const
{
	return nullptr != activeEngine;
}

bool ScriptRunner::loadScriptFile(const QString &filePath, QString &dest)
{
	if (not Utils::LoadTextFromFile(dest, filePath))
	{
		savedErrorMessage = tr("Unable to load script file '%1'.")
								.arg(QDir::toNativeSeparators(filePath));
		log(savedErrorMessage);
		return false;
	}

	return true;
}

bool ScriptRunner::executeScript(const QString &script, const QString &filePath)
{
	QString fp = filePath;
	if (filePath.isEmpty())
		fp = QStringLiteral("PrepareScript");
	else
		importedScripts.insert(QDir::cleanPath(filePath));

	return executeCustom([this, &script, fp]() -> QScriptValue {
		return evaluateFileContent(activeEngine, script, fp);
	});
}

void ScriptRunner::logError(const QScriptValue &error)
{
	auto fileName = error.property(QSTRKEY(fileName)).toString();
	auto lineNumber = error.property(QSTRKEY(lineNumber)).toInt32();
	savedErrorMessage = QStringLiteral("(%1:%2) %3")
							.arg(fileName)
							.arg(lineNumber)
							.arg(error.toString());
	log(savedErrorMessage);

	auto global = error.engine()->globalObject();
	if (global.property(QSTRKEY(__showBacktrace)).toBool())
	{
		for (auto &backTraceLine : error.engine()->uncaughtExceptionBacktrace())
		{
			log(backTraceLine);
		}
	}
}

bool ScriptRunner::executeCustom(const Evaluate &evaluate)
{
	Q_ASSERT(nullptr != activeEngine);
	auto engine = activeEngine;

	QScriptValue result;
	try
	{
		result = evaluate();
	} catch (...)
	{
		if (engine->hasUncaughtException())
		{
			result = engine->uncaughtException();
		} else
		{
			result = ThrowUnknownError(engine->currentContext());
		}
	}

	if (result.isError())
	{
		logError(result);
		return false;
	} else
	{
		if (engine->hasUncaughtException())
		{
			activeEngine = nullptr;
		}
	}

	return true;
}

bool ScriptRunner::execute(
	const QString &filePath, const QString &prepareScript)
{
	importedScripts.clear();
	beforeScriptExecution(filePath);
	QScriptEngine engine;

	initializeEngine(&engine);

	auto globalObject = engine.globalObject();

	globalObject.setProperty(
		QSTRKEY(projects), engine.newQObject(projectGroup));

	savedErrorMessage.clear();

	bool ok = true;
	do
	{
		if (activeEngine && not prepareScript.isEmpty())
		{
			if (not executeScript(prepareScript))
			{
				ok = false;
				break;
			}
		}

		if (activeEngine && not filePath.isEmpty())
		{
			ok = false;
			QString scriptText;
			if (loadScriptFile(filePath, scriptText))
			{
				if (executeScript(scriptText, filePath))
				{
					ok = true;
				}
			}
		}
	} while (false);

	if (activeEngine == nullptr)
		ok = true;
	else
		activeEngine = nullptr;
	afterScriptExecution(ok, savedErrorMessage);
	return ok;
}

bool ScriptRunner::executeForTargets(
	const QString &filePath, const QObjectList &targets)
{
	importedScripts.clear();
	beforeScriptExecution(filePath);
	savedErrorMessage.clear();

	bool ok = false;
	QString scriptText;
	if (loadScriptFile(filePath, scriptText))
	{
		ok = true;

		QScriptEngine engine;

		initializeEngine(&engine);

		auto globalObject = engine.globalObject();

		globalObject.setProperty(
			QSTRKEY(projects), engine.newQObject(projectGroup));

		do
		{
			if (activeEngine == nullptr)
				break;

			if (not executeScript(scriptText, filePath))
			{
				ok = false;
				break;
			}

			QScriptValue exec = globalObject.property(QSTRKEY(exec));
			if (not exec.isValid())
			{
				break;
			}

			if (not exec.isFunction())
			{
				logError(engine.currentContext()->throwError(
					tr("Global property 'exec' is not a function.")));
				ok = false;
				break;
			}

			auto execPtr = &exec;

			auto targetsCopy = targets;
			for (auto target : targetsCopy)
			{
				if (activeEngine == nullptr ||
					activeEngine->hasUncaughtException() ||
					not executeCustom(
						[this, target, execPtr]() -> QScriptValue //
						{
							if (activeEngine == nullptr)
							{
								return QScriptValue();
							}

							auto args = activeEngine->newArray(1);
							args.setProperty(
								0, activeEngine->newQObject(target));

							return execPtr->call(QScriptValue(), args);
						}))
				{
					ok = false;
					break;
				}
			}
		} while (false);

		if (activeEngine == nullptr)
			ok = true;
		else
			activeEngine = nullptr;
	}
	afterScriptExecution(ok, savedErrorMessage);
	return ok;
}

const QString &ScriptRunner::getErrorMessage() const
{
	return savedErrorMessage;
}

void ScriptRunner::setProcessEventsInterval(int value)
{
	if (mProcessEventsInterval != value)
	{
		mProcessEventsInterval = value;

		if (activeEngine)
			activeEngine->setProcessEventsInterval(value);
	}
}

void ScriptRunner::importScript(const QString &filePath)
{
	Q_ASSERT(nullptr != activeEngine);
	Q_ASSERT(not filePath.isEmpty());

	auto cleanPath = QDir::cleanPath(filePath);
	if (importedScripts.count(cleanPath))
		return;
	importedScripts.insert(cleanPath);

	QString scriptText;
	bool loadOk = loadScriptFile(cleanPath, scriptText);
	auto context = activeEngine->currentContext();
	if (not loadOk)
	{
		context->throwError(savedErrorMessage);
		return;
	}

	auto prevActivationObject = context->activationObject();
	context->setActivationObject(activeEngine->globalObject());

	evaluateFileContent(activeEngine, scriptText, cleanPath);

	context->setActivationObject(prevActivationObject);
}

void ScriptRunner::initializeEngine(QScriptEngine *engine)
{
	Q_ASSERT(nullptr != engine);
	activeEngine = engine;

	qScriptRegisterMetaType(engine, QObjectToScriptValue<Banana::AbstractFile>,
		QObjectFromScriptValue<Banana::AbstractFile>);

	qScriptRegisterMetaType(engine, QObjectToScriptValue<Banana::Directory>,
		QObjectFromScriptValue<Banana::Directory>);

	ScriptQPoint::Register(engine);
	ScriptQSize::Register(engine);
	ScriptQRect::Register(engine);
	ScriptQColor::Register(engine);
	ScriptQFont::Register(engine);
	ScriptQFileInfo::Register(engine);
	ScriptQDir::Register(engine);
	ScriptQByteArray::Register(engine);
	ScriptQIODevice::Register(engine);
	ScriptQFileDevice::Register(engine);
	ScriptQFile::Register(engine);
	ScriptQSaveFile::Register(engine);
	ScriptQTemporaryFile::Register(engine);

	ScriptQUrl::Register(engine);
	ScriptQNetworkRequest::Register(engine);
	ScriptQNetworkReply::Register(engine);
	ScriptQNetworkAccessManager::Register(engine);
	ScriptQCryptographicHash::Register(engine);

	auto globalObject = engine->globalObject();

	auto systemObject = engine->newQObject(this);
	globalObject.setProperty(
		QSTRKEY(system), systemObject, STATIC_SCRIPT_VALUE);
	globalObject.setProperty(
		QSTRKEY(print), engine->newFunction(print, systemObject));

	systemObject.setProperty(QSTRKEY(loadTextFromFile),
		engine->newFunction(loadTextFromFile, systemObject));
	systemObject.setProperty(QSTRKEY(saveTextToFile),
		engine->newFunction(saveTextToFile, systemObject));
	systemObject.setProperty(
		QSTRKEY(loadFromJson), engine->newFunction(loadFromJson, systemObject));
	systemObject.setProperty(
		QSTRKEY(saveToJson), engine->newFunction(saveToJson, systemObject));
	systemObject.setProperty(
		QSTRKEY(loadFileTree), engine->newFunction(loadFileTree, systemObject));
	auto stringFormatFunc = engine->newFunction(stringFormat, globalObject);
	systemObject.setProperty(
		QSTRKEY(copyProperties), engine->newFunction(copyProperties));
	systemObject.setProperty(QSTRKEY(stringFormat), stringFormatFunc);
	globalObject.setProperty(QSTRKEY(strf), stringFormatFunc);
	globalObject.setProperty(
		QSTRKEY(assert), engine->newFunction(assert, globalObject));

	globalObject.setProperty(
		QSTRKEY(macro), engine->newFunction(macro, globalObject));

	globalObject.setProperty(QSTRKEY(importScript),
		engine->newFunction(internalImportScript, systemObject));
	{
		auto qStandardPaths = engine->newObject();
		enum
		{
			DesktopLocation = QStandardPaths::DesktopLocation,
			DocumentsLocation = QStandardPaths::DocumentsLocation,
			FontsLocation = QStandardPaths::FontsLocation,
			ApplicationsLocation = QStandardPaths::ApplicationsLocation,
			MusicLocation = QStandardPaths::MusicLocation,
			MoviesLocation = QStandardPaths::MoviesLocation,
			PicturesLocation = QStandardPaths::PicturesLocation,
			TempLocation = QStandardPaths::TempLocation,
			HomeLocation = QStandardPaths::HomeLocation,
			DataLocation = QStandardPaths::DataLocation,
			CacheLocation = QStandardPaths::CacheLocation,
			GenericDataLocation = QStandardPaths::GenericDataLocation,
			RuntimeLocation = QStandardPaths::RuntimeLocation,
			ConfigLocation = QStandardPaths::ConfigLocation,
			DownloadLocation = QStandardPaths::DownloadLocation,
			GenericCacheLocation = QStandardPaths::GenericCacheLocation,
			GenericConfigLocation = QStandardPaths::GenericConfigLocation,
			AppDataLocation = QStandardPaths::AppDataLocation,
			AppConfigLocation = QStandardPaths::AppConfigLocation,
			AppLocalDataLocation = QStandardPaths::AppLocalDataLocation,
		};
		SCRIPT_REG_ENUM(qStandardPaths, DesktopLocation);
		SCRIPT_REG_ENUM(qStandardPaths, DocumentsLocation);
		SCRIPT_REG_ENUM(qStandardPaths, FontsLocation);
		SCRIPT_REG_ENUM(qStandardPaths, ApplicationsLocation);
		SCRIPT_REG_ENUM(qStandardPaths, MusicLocation);
		SCRIPT_REG_ENUM(qStandardPaths, MoviesLocation);
		SCRIPT_REG_ENUM(qStandardPaths, PicturesLocation);
		SCRIPT_REG_ENUM(qStandardPaths, TempLocation);
		SCRIPT_REG_ENUM(qStandardPaths, HomeLocation);
		SCRIPT_REG_ENUM(qStandardPaths, DataLocation);
		SCRIPT_REG_ENUM(qStandardPaths, CacheLocation);
		SCRIPT_REG_ENUM(qStandardPaths, GenericDataLocation);
		SCRIPT_REG_ENUM(qStandardPaths, RuntimeLocation);
		SCRIPT_REG_ENUM(qStandardPaths, ConfigLocation);
		SCRIPT_REG_ENUM(qStandardPaths, DownloadLocation);
		SCRIPT_REG_ENUM(qStandardPaths, GenericCacheLocation);
		SCRIPT_REG_ENUM(qStandardPaths, GenericConfigLocation);
		SCRIPT_REG_ENUM(qStandardPaths, AppDataLocation);
		SCRIPT_REG_ENUM(qStandardPaths, AppConfigLocation);
		SCRIPT_REG_ENUM(qStandardPaths, AppLocalDataLocation);

		qStandardPaths.setProperty(
			QSTRKEY(writableLocation), engine->newFunction(writableLocation));
		qStandardPaths.setProperty(
			QSTRKEY(standardLocations), engine->newFunction(standardLocations));
		globalObject.setProperty(
			QSTRKEY(QStandardPaths), qStandardPaths, STATIC_SCRIPT_VALUE);
	}

	AbstractScriptRunner::initializeEngine(engine);

	engine->setProcessEventsInterval(mProcessEventsInterval);
}

QScriptValue ScriptRunner::writableLocation(
	QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 1)
		return ThrowBadNumberOfArguments(context);

	return QScriptValue(engine,
		QStandardPaths::writableLocation(
			(QStandardPaths::StandardLocation) context->argument(0).toInt32()));
}

QScriptValue ScriptRunner::standardLocations(
	QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 1)
		return ThrowBadNumberOfArguments(context);

	auto locations = QStandardPaths::standardLocations(
		(QStandardPaths::StandardLocation) context->argument(0).toInt32());

	int count = locations.count();
	QScriptValue result = engine->newArray(count);

	for (int i = 0; i < count; i++)
	{
		result.setProperty(i, locations.at(i));
	}

	return result;
}
}
