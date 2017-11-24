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

#include "ProjectGroup.h"
#include "AbstractProjectDirectory.h"
#include "AbstractProjectFile.h"
#include "Utils.h"
#include "ScriptUtils.h"
#include "ScriptTemplates.h"
#include "ScriptManager.h"
#include "Config.h"

#include <QScriptEngine>
#include <QScriptContextInfo>

namespace Scripting
{
void init()
{
	(void) QT_TRANSLATE_NOOP("FileFormatNamePlural", ".js");
}

const char szScriptExtension[] = QT_TRANSLATE_NOOP("FileFormatName", ".js");
}

using namespace Scripting;

namespace Banana
{
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
		if (currentFileInfo.isFile())
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

	QString scriptText;
	bool loadOk = runner->loadScriptFile(resolvedFilePath, scriptText);

	if (not loadOk)
	{
		out = context->throwError(
			QScriptContext::UnknownError, runner->savedErrorMessage);

		return;
	}

	auto prevActivationObject = context->activationObject();
	context->setActivationObject(engine->globalObject());

	out = evaluateFileContent(engine, scriptText, resolvedFilePath);

	context->setActivationObject(prevActivationObject);
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

	if (not arg.isQObject())
		return IncompatibleArgumentType(context, i);

	auto obj = dynamic_cast<Object *>(arg.toQObject());
	if (nullptr == obj)
		return IncompatibleArgumentType(context, i);

	i++;

	if (i >= argc)
		return ThrowBadNumberOfArguments(context);

	auto callback = context->argument(i);

	if (not callback.isFunction())
		return IncompatibleArgumentType(context, i);

	obj->beginMacro(macroName);

	arg = callback.call(arg);

	obj->endMacro();

	return arg;
}

static QScriptValue importScript(QScriptContext *context, QScriptEngine *e)
{
	if (context->argumentCount() < 1)
		return ThrowBadNumberOfArguments(context);

	Q_ASSERT(e->currentContext() == context);

	auto callee = context->callee();

	auto prototype = callee.property(QSTRKEY(prototype));
	auto runner = static_cast<ScriptRunner *>(prototype.toQObject());
	if (nullptr == runner)
	{
		return IncompatibleArgumentType(context, -1);
	}

	auto arg = context->argument(0);
	if (not arg.isString())
	{
		return IncompatibleArgumentType(context, 0);
	}

	QScriptValue result;
	internalEvalScript(runner, arg.toString(), result);

	return result;
}

static QScriptValue print(QScriptContext *context, QScriptEngine *e)
{
	auto callee = context->callee();
	auto prototype = callee.property(QSTRKEY(prototype));
	auto runner = static_cast<ScriptRunner *>(prototype.toQObject());
	if (nullptr == runner)
	{
		return IncompatibleArgumentType(context, -1);
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
			return Utils::VariantToScriptValue(vmap, engine);
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
	, mProcessEventsInterval(20)
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
	return executeCustom([this, &script, &filePath]() -> QScriptValue {
		return evaluateFileContent(activeEngine, script, filePath);
	});
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
			result = engine->currentContext()->throwError(
				QScriptContext::UnknownError, tr("Unknown error"));
		}
	}

	if (result.isError())
	{
		auto fileName = result.property(QSTRKEY(fileName)).toString();
		auto lineNumber = result.property(QSTRKEY(lineNumber)).toInt32();
		savedErrorMessage = QStringLiteral("(%1:%2) %3")
								.arg(fileName)
								.arg(lineNumber)
								.arg(result.toString());
		log(savedErrorMessage);
		for (auto &backTraceLine : engine->uncaughtExceptionBacktrace())
		{
			log(backTraceLine);
		}
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
			if (not executeScript(
					prepareScript, QStringLiteral("PrepareScript")))
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

			if (not exec.isFunction())
			{
				break;
			}

			auto execPtr = &exec;

			for (auto target : targets)
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

void ScriptRunner::initializeEngine(QScriptEngine *engine)
{
	Q_ASSERT(nullptr != engine);
	activeEngine = engine;

	qScriptRegisterMetaType(engine, QObjectToScriptValue<Banana::AbstractFile>,
		QObjectFromScriptValue<Banana::AbstractFile>);

	qScriptRegisterMetaType(engine, QObjectToScriptValue<Banana::Directory>,
		QObjectFromScriptValue<Banana::Directory>);

	qScriptRegisterMetaType(engine, DescendantToScriptValue<Point, QPoint>,
		DescendantFromScriptValue<Point, QPoint>);

	qScriptRegisterMetaType(engine, DescendantToScriptValue<Point, QPointF>,
		DescendantFromScriptValue<Point, QPointF>);

	qScriptRegisterMetaType(engine, DescendantToScriptValue<Size, QSize>,
		DescendantFromScriptValue<Size, QSize>);

	qScriptRegisterMetaType(engine, DescendantToScriptValue<Size, QSizeF>,
		DescendantFromScriptValue<Size, QSizeF>);

	qScriptRegisterMetaType(engine, DescendantToScriptValue<Rect, QRect>,
		DescendantFromScriptValue<Rect, QRect>);

	qScriptRegisterMetaType(engine, DescendantToScriptValue<Rect, QRectF>,
		DescendantFromScriptValue<Rect, QRectF>);

	qScriptRegisterMetaType(engine, Font::ToScriptValue, Font::FromScriptValue);

	qScriptRegisterMetaType(engine, Dir::ToScriptValue, Dir::FromScriptValue);

	qScriptRegisterMetaType(
		engine, FileInfo::ToScriptValue, FileInfo::FromScriptValue);

	qScriptRegisterMetaType(
		engine, Color::ToScriptValue, Color::FromScriptValue);

	qScriptRegisterMetaType(engine, EnumToScriptValue<QFont::Style>,
		EnumFromScriptValue<QFont::Style>);

	qScriptRegisterMetaType(engine, EnumToScriptValue<QDir::Filters>,
		EnumFromScriptValue<QDir::Filters>);

	qScriptRegisterMetaType(engine, EnumToScriptValue<QDir::SortFlags>,
		EnumFromScriptValue<QDir::SortFlags>);

	QMetaType::registerConverter<QFont::Style, qint32>();
	QMetaType::registerConverter<QDir::Filters, qint32>();
	QMetaType::registerConverter<QDir::SortFlags, qint32>();

	auto globalObject = engine->globalObject();

	globalObject.setProperty(
		QSTRKEY(QPoint), engine->newFunction(Point::Constructor));
	globalObject.setProperty(
		QSTRKEY(QPointF), engine->newFunction(Point::ConstructorF));
	globalObject.setProperty(
		QSTRKEY(QSize), engine->newFunction(Size::Constructor));
	globalObject.setProperty(
		QSTRKEY(QSizeF), engine->newFunction(Size::ConstructorF));
	globalObject.setProperty(
		QSTRKEY(QRect), engine->newFunction(Rect::Constructor));
	globalObject.setProperty(
		QSTRKEY(QRectF), engine->newFunction(Rect::ConstructorF));
	globalObject.setProperty(
		QSTRKEY(QFileInfo), engine->newFunction(FileInfo::Constructor));
	globalObject.setProperty(
		QSTRKEY(QColor), engine->newFunction(Color::Constructor));

	auto qdirObject = engine->newFunction(Dir::Constructor);
	globalObject.setProperty(QSTRKEY(QDir), qdirObject);

	qdirObject.setProperty(
		QSTRKEY(Dirs), (int) QDir::Dirs, STATIC_SCRIPT_VALUE);
	qdirObject.setProperty(
		QSTRKEY(Files), (int) QDir::Files, STATIC_SCRIPT_VALUE);
	qdirObject.setProperty(
		QSTRKEY(Drives), (int) QDir::Drives, STATIC_SCRIPT_VALUE);
	qdirObject.setProperty(
		QSTRKEY(NoSymLinks), (int) QDir::NoSymLinks, STATIC_SCRIPT_VALUE);
	qdirObject.setProperty(
		QSTRKEY(Readable), (int) QDir::Readable, STATIC_SCRIPT_VALUE);
	qdirObject.setProperty(
		QSTRKEY(Writable), (int) QDir::Writable, STATIC_SCRIPT_VALUE);
	qdirObject.setProperty(
		QSTRKEY(Executable), (int) QDir::Executable, STATIC_SCRIPT_VALUE);
	qdirObject.setProperty(
		QSTRKEY(Modified), (int) QDir::Modified, STATIC_SCRIPT_VALUE);
	qdirObject.setProperty(
		QSTRKEY(Hidden), (int) QDir::Hidden, STATIC_SCRIPT_VALUE);
	qdirObject.setProperty(
		QSTRKEY(System), (int) QDir::System, STATIC_SCRIPT_VALUE);
	qdirObject.setProperty(
		QSTRKEY(AllEntries), (int) QDir::AllEntries, STATIC_SCRIPT_VALUE);
	qdirObject.setProperty(
		QSTRKEY(AllDirs), (int) QDir::AllDirs, STATIC_SCRIPT_VALUE);
	qdirObject.setProperty(
		QSTRKEY(CaseSensitive), (int) QDir::CaseSensitive, STATIC_SCRIPT_VALUE);
	qdirObject.setProperty(
		QSTRKEY(NoDots), (int) QDir::NoDotAndDotDot, STATIC_SCRIPT_VALUE);
	qdirObject.setProperty(
		QSTRKEY(NoFilter), (int) QDir::NoFilter, STATIC_SCRIPT_VALUE);

	qdirObject.setProperty(
		QSTRKEY(Name), (int) QDir::Name, STATIC_SCRIPT_VALUE);
	qdirObject.setProperty(
		QSTRKEY(Time), (int) QDir::Time, STATIC_SCRIPT_VALUE);
	qdirObject.setProperty(
		QSTRKEY(Size), (int) QDir::Size, STATIC_SCRIPT_VALUE);
	qdirObject.setProperty(
		QSTRKEY(Unsorted), (int) QDir::Unsorted, STATIC_SCRIPT_VALUE);
	qdirObject.setProperty(
		QSTRKEY(DirsFirst), (int) QDir::DirsFirst, STATIC_SCRIPT_VALUE);
	qdirObject.setProperty(
		QSTRKEY(Reversed), (int) QDir::Reversed, STATIC_SCRIPT_VALUE);
	qdirObject.setProperty(
		QSTRKEY(IgnoreCase), (int) QDir::IgnoreCase, STATIC_SCRIPT_VALUE);
	qdirObject.setProperty(
		QSTRKEY(DirsLast), (int) QDir::DirsLast, STATIC_SCRIPT_VALUE);
	qdirObject.setProperty(
		QSTRKEY(LocaleAware), (int) QDir::LocaleAware, STATIC_SCRIPT_VALUE);
	qdirObject.setProperty(
		QSTRKEY(Type), (int) QDir::Type, STATIC_SCRIPT_VALUE);
	qdirObject.setProperty(
		QSTRKEY(NoSort), (int) QDir::NoSort, STATIC_SCRIPT_VALUE);

	auto qfontObject = engine->newFunction(Font::Constructor);
	globalObject.setProperty(QSTRKEY(QFont), qfontObject);
	qfontObject.setProperty(
		QSTRKEY(StyleNormal), (int) QFont::StyleNormal, STATIC_SCRIPT_VALUE);
	qfontObject.setProperty(
		QSTRKEY(StyleItalic), (int) QFont::StyleItalic, STATIC_SCRIPT_VALUE);
	qfontObject.setProperty(
		QSTRKEY(StyleOblique), (int) QFont::StyleOblique, STATIC_SCRIPT_VALUE);

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
	systemObject.setProperty(QSTRKEY(stringFormat), stringFormatFunc);
	globalObject.setProperty(QSTRKEY(strf), stringFormatFunc);

	globalObject.setProperty(
		QSTRKEY(macro), engine->newFunction(macro, globalObject));

	globalObject.setProperty(
		QSTRKEY(importScript), engine->newFunction(importScript, systemObject));

	AbstractScriptRunner::initializeEngine(engine);

	engine->setProcessEventsInterval(mProcessEventsInterval);
}
}
