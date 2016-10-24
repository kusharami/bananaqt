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

#include "ScriptRunner.h"

#include "ProjectGroup.h"
#include "AbstractProjectDirectory.h"
#include "AbstractProjectFile.h"
#include "Utils.h"
#include "ScriptUtils.h"
#include "ScriptTemplates.h"
#include "Config.h"

#include <QScriptEngine>
#include <QFileDialog>
#include <QCoreApplication>

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
	static QString Format(QScriptContext *context)
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

	static QScriptValue Format(QScriptContext *context, QScriptEngine *)
	{
		if (context->argumentCount() < 2)
			return ThrowBadNumberOfArguments(context);

		return QScriptValue(Format(context));
	}

	static QScriptValue Print(QScriptContext *context, QScriptEngine *)
	{
		QString text(Format(context));

		auto callee = context->callee();
		auto prototype = callee.property("prototype");
		auto runner = static_cast<ScriptRunner *>(prototype.toQObject());
		if (nullptr != runner)
			runner->log(text);

		return QScriptValue(text);
	}

	static QScriptValue LoadFromJson(QScriptContext *context, QScriptEngine *engine)
	{
		if (context->argumentCount() == 1)
		{
			QVariantMap vmap;
			bool ok = Utils::LoadVariantMapFromFile(vmap, context->argument(0).toString());
			if (ok)
				return Utils::VariantToScriptValue(vmap, engine);
		}

		return QScriptValue();
	}

	static QScriptValue SaveToJson(QScriptContext *context, QScriptEngine *)
	{
		if (context->argumentCount() == 2)
		{
			auto v = context->argument(0).toVariant();
			if (v.type() == QVariant::Map)
			{
				bool ok = Utils::SaveVariantMapToFile(v.toMap(), context->argument(1).toString());

				return QScriptValue(ok);
			}
		}

		return QScriptValue();
	}

	static QScriptValue LoadFileTree(QScriptContext *context, QScriptEngine *engine)
	{
		if (context->argumentCount() == 2)
		{
			auto result = context->argument(1);

			if (result.isArray())
			{
				int len = result.property("length").toInt32();

				QStringList lst;

				for (int i = 0; i < len; i++)
				{
					lst.push_back(result.property(i).toString());
				}

				lst = Utils::ListDirectoryContents(context->argument(0).toString(), lst);

				len = lst.size();

				result = engine->newArray(len);

				for (int i = 0; i < len; i++)
				{
					result.setProperty(i, lst.at(i));
				}

				return result;
			}
		}

		return QScriptValue();
	}

	ScriptRunner::ScriptRunner(ProjectGroup *projectGroup)
		: projectGroup(projectGroup)
		, parentWidget(nullptr)
	{
	}

	void ScriptRunner::setParentWidget(QWidget *parentWidget)
	{
		this->parentWidget = parentWidget;
	}

	QString ScriptRunner::requestExistingFilePath(const QString &title, const QString &dir, const QString &filters) const
	{
		QString selected_filter;
		return QFileDialog::getOpenFileName(parentWidget, title, dir, filters, &selected_filter,
											FILE_DIALOG_FLAGS |
											QFileDialog::DontResolveSymlinks);
	}

	QString ScriptRunner::requestNewFilePath(const QString &title, const QString &dir, const QString &filters) const
	{
		QString selected_filter;
		return QFileDialog::getSaveFileName(parentWidget, title, dir, filters, &selected_filter,
											FILE_DIALOG_FLAGS |
											QFileDialog::DontResolveSymlinks);
	}

	QString ScriptRunner::requestDirectoryPath(const QString &title, const QString &dir) const
	{
		return QFileDialog::getExistingDirectory(parentWidget, title, dir,
												 QFileDialog::ShowDirsOnly |
												 FILE_DIALOG_FLAGS |
												 QFileDialog::DontResolveSymlinks);
	}

	void ScriptRunner::log(const QString &text)
	{
		emit logPrint(text);
	}

	bool ScriptRunner::executeScript(QScriptEngine *engine, const QString &script, const QString &filePath)
	{
		QScriptValue result;
		try
		{
			result = engine->evaluate(script, filePath);
		} catch (...)
		{
			if (engine->hasUncaughtException())
				result = engine->uncaughtException();
		}

		if (result.isError())
		{
			savedErrorMessage = QString::fromLatin1("(%1:%2) %3")
					.arg(filePath)
					.arg(result.property("lineNumber").toInt32())
					.arg(result.toString());
			log(savedErrorMessage);
			for (auto &backTraceLine : engine->uncaughtExceptionBacktrace())
				log(backTraceLine);
			return false;
		}

		return true;
	}

	bool ScriptRunner::execute(const QString &filePath, const QString &prepareScript)
	{
		QScriptEngine engine;

		initializeEngine(&engine);

		auto globalObject = engine.globalObject();

		globalObject.setProperty("projects", engine.newQObject(projectGroup));

		savedErrorMessage.clear();

		if (!prepareScript.isEmpty())
		{
			if (!executeScript(&engine, prepareScript, "PrepareScript"))
				return false;
		}

		if (!filePath.isEmpty())
		{
			QString script_text;
			if (Utils::LoadTextFromFile(script_text, filePath))
			{
				return executeScript(&engine, script_text, filePath);
			}

			savedErrorMessage = tr("Unable to load script file '%1'.").arg(filePath);
			log(savedErrorMessage);
			return false;
		}

		return true;
	}

	const QString &ScriptRunner::getErrorMessage() const
	{
		return savedErrorMessage;
	}

	void ScriptRunner::initializeEngine(QScriptEngine *engine)
	{
		qScriptRegisterMetaType(engine,
								QObjectToScriptValue<Banana::AbstractFile>,
								QObjectFromScriptValue<Banana::AbstractFile>);

		qScriptRegisterMetaType(engine,
								DescendantToScriptValue<Point, QPoint>,
								DescendantFromScriptValue<Point, QPoint>);

		qScriptRegisterMetaType(engine,
								DescendantToScriptValue<Point, QPointF>,
								DescendantFromScriptValue<Point, QPointF>);

		qScriptRegisterMetaType(engine,
								DescendantToScriptValue<Size, QSize>,
								DescendantFromScriptValue<Size, QSize>);

		qScriptRegisterMetaType(engine,
								DescendantToScriptValue<Size, QSizeF>,
								DescendantFromScriptValue<Size, QSizeF>);

		qScriptRegisterMetaType(engine,
								DescendantToScriptValue<Rect, QRect>,
								DescendantFromScriptValue<Rect, QRect>);

		qScriptRegisterMetaType(engine,
								DescendantToScriptValue<Rect, QRectF>,
								DescendantFromScriptValue<Rect, QRectF>);

		qScriptRegisterMetaType(engine,
								Font::ToScriptValue,
								Font::FromScriptValue);

		qScriptRegisterMetaType(engine,
								Dir::ToScriptValue,
								Dir::FromScriptValue);

		qScriptRegisterMetaType(engine,
								FileInfo::ToScriptValue,
								FileInfo::FromScriptValue);

		qScriptRegisterMetaType(engine,
								Color::ToScriptValue,
								Color::FromScriptValue);

		qScriptRegisterMetaType(engine,
								EnumToScriptValue<QFont::Style>,
								EnumFromScriptValue<QFont::Style>);

		qScriptRegisterMetaType(engine,
								EnumToScriptValue<QDir::Filters>,
								EnumFromScriptValue<QDir::Filters>);

		qScriptRegisterMetaType(engine,
								EnumToScriptValue<QDir::SortFlags>,
								EnumFromScriptValue<QDir::SortFlags>);

		QMetaType::registerConverter<QFont::Style, qint32>();

		auto globalObject = engine->globalObject();

		globalObject.setProperty("QPoint", engine->newFunction(Point::Constructor));
		globalObject.setProperty("QPointF", engine->newFunction(Point::ConstructorF));
		globalObject.setProperty("QSize", engine->newFunction(Size::Constructor));
		globalObject.setProperty("QSizeF", engine->newFunction(Size::ConstructorF));
		globalObject.setProperty("QRect", engine->newFunction(Rect::Constructor));
		globalObject.setProperty("QRectF", engine->newFunction(Rect::ConstructorF));
		globalObject.setProperty("QFileInfo", engine->newFunction(FileInfo::Constructor));
		globalObject.setProperty("QColor", engine->newFunction(Color::Constructor));

		auto qdirObject = engine->newFunction(Dir::Constructor);
		globalObject.setProperty("QDir", qdirObject);

		qdirObject.setProperty("Dirs", (int) QDir::Dirs, STATIC_SCRIPT_VALUE);
		qdirObject.setProperty("Files", (int) QDir::Files, STATIC_SCRIPT_VALUE);
		qdirObject.setProperty("Drives", (int) QDir::Drives, STATIC_SCRIPT_VALUE);
		qdirObject.setProperty("NoSymLinks", (int) QDir::NoSymLinks, STATIC_SCRIPT_VALUE);
		qdirObject.setProperty("Readable", (int) QDir::Readable, STATIC_SCRIPT_VALUE);
		qdirObject.setProperty("Writable", (int) QDir::Writable, STATIC_SCRIPT_VALUE);
		qdirObject.setProperty("Executable", (int) QDir::Executable, STATIC_SCRIPT_VALUE);
		qdirObject.setProperty("Modified", (int) QDir::Modified, STATIC_SCRIPT_VALUE);
		qdirObject.setProperty("Hidden", (int) QDir::Hidden, STATIC_SCRIPT_VALUE);
		qdirObject.setProperty("System", (int) QDir::System, STATIC_SCRIPT_VALUE);
		qdirObject.setProperty("AllEntries", (int) QDir::AllEntries, STATIC_SCRIPT_VALUE);
		qdirObject.setProperty("AllDirs", (int) QDir::AllDirs, STATIC_SCRIPT_VALUE);
		qdirObject.setProperty("CaseSensitive", (int) QDir::CaseSensitive, STATIC_SCRIPT_VALUE);
		qdirObject.setProperty("NoDots", (int) QDir::NoDotAndDotDot, STATIC_SCRIPT_VALUE);
		qdirObject.setProperty("NoFilter", (int) QDir::NoFilter, STATIC_SCRIPT_VALUE);

		qdirObject.setProperty("Name", (int) QDir::Name, STATIC_SCRIPT_VALUE);
		qdirObject.setProperty("Time", (int) QDir::Time, STATIC_SCRIPT_VALUE);
		qdirObject.setProperty("Size", (int) QDir::Size, STATIC_SCRIPT_VALUE);
		qdirObject.setProperty("Unsorted", (int) QDir::Unsorted, STATIC_SCRIPT_VALUE);
		qdirObject.setProperty("DirsFirst", (int) QDir::DirsFirst, STATIC_SCRIPT_VALUE);
		qdirObject.setProperty("Reversed", (int) QDir::Reversed, STATIC_SCRIPT_VALUE);
		qdirObject.setProperty("IgnoreCase", (int) QDir::IgnoreCase, STATIC_SCRIPT_VALUE);
		qdirObject.setProperty("DirsLast", (int) QDir::DirsLast, STATIC_SCRIPT_VALUE);
		qdirObject.setProperty("LocaleAware", (int) QDir::LocaleAware, STATIC_SCRIPT_VALUE);
		qdirObject.setProperty("Type", (int) QDir::Type, STATIC_SCRIPT_VALUE);
		qdirObject.setProperty("NoSort", (int) QDir::NoSort, STATIC_SCRIPT_VALUE);

		auto qfontObject = engine->newFunction(Font::Constructor);
		globalObject.setProperty("QFont", qfontObject);
		qfontObject.setProperty("StyleNormal", (int) QFont::StyleNormal, STATIC_SCRIPT_VALUE);
		qfontObject.setProperty("StyleItalic", (int) QFont::StyleItalic, STATIC_SCRIPT_VALUE);
		qfontObject.setProperty("StyleOblique", (int) QFont::StyleOblique, STATIC_SCRIPT_VALUE);

		auto systemOobject = engine->newQObject(this);
		globalObject.setProperty("system", systemOobject, STATIC_SCRIPT_VALUE);
		globalObject.setProperty("print", engine->newFunction(Print, systemOobject));

		systemOobject.setProperty("loadFromJson", engine->newFunction(LoadFromJson, systemOobject));
		systemOobject.setProperty("saveToJson", engine->newFunction(SaveToJson, systemOobject));
		systemOobject.setProperty("loadFileTree", engine->newFunction(LoadFileTree, systemOobject));
		systemOobject.setProperty("stringFormat", engine->newFunction(Format, systemOobject));
	}
}
