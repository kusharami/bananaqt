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

#include "ScriptRunnerDialog.h"

#include "BananaScript/ScriptRunner.h"
#include "BananaScript/ScriptUtils.h"

#include "BananaCore/ProjectGroup.h"
#include "BananaCore/Config.h"
#include "BananaCore/AbstractProjectDirectory.h"
#include "BananaCore/IProjectGroupDelegate.h"
#include "BananaCore/Utils.h"
#include "BananaCore/IAbortDelegate.h"

#include "QtnProperty/CustomPropertyEditorDialog.h"

#include "Config.h"
#include "FileSelectDialog.h"

#include <QFileDialog>
#include <QSettings>
#include <QPlainTextEdit>
#include <QTextBlock>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QMainWindow>
#include <QTimer>
#include <QScriptEngine>
#include <QStandardPaths>
#include <QDateTime>
#include <QInputDialog>

using namespace Banana;
#include "ui_ScriptRunnerDialog.h"

#define DialogFlags \
	(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint)

using namespace Script;

namespace Banana
{
struct ScriptSubDialogHandler
{
	ScriptRunnerDialog *mDialog;

	ScriptSubDialogHandler(QScriptContext *context);
	ScriptSubDialogHandler(ScriptRunnerDialog *dialog);
	~ScriptSubDialogHandler();

	QString defaultDir() const;

private:
	void init();
};

bool ScriptRunnerDialog::cursorIsSet = false;

static const QString sScriptRunnerGroup = QStringLiteral("ScriptRunner");

static QScriptValue inputString(QScriptContext *context, QScriptEngine *engine)
{
	QString title;
	int argShift = -1;
	switch (context->argumentCount())
	{
		case 4:
			title = context->argument(0).toString();
			argShift = 0;
			Q_FALLTHROUGH();
		case 3:
		case 2:
		case 1:
		{
			auto autoComplete = context->argument(3 + argShift);
			if (not autoComplete.isUndefined() && not autoComplete.isArray())
			{
				return ThrowIncompatibleArgumentType(context, 4 + argShift);
			}

			auto label = context->argument(1 + argShift).toString();
			auto textV = context->argument(2 + argShift);
			auto text = textV.isUndefined() ? QString() : textV.toString();

			unsigned length = autoComplete.isArray()
				? autoComplete.property(QSTRKEY(length)).toUInt32()
				: 0;

			bool ok = false;
			if (length == 0)
			{
				text = QInputDialog::getText(nullptr, title, label,
					QLineEdit::Normal, text, &ok, DialogFlags);
			} else
			{
				QInputDialog dlg;

				dlg.setWindowFlags(DialogFlags);

				QStringList itemNames;
				for (unsigned i = 0; i < length; i++)
				{
					itemNames.append(autoComplete.property(i).toString());
				}

				dlg.setWindowTitle(title);
				dlg.setLabelText(label);

				dlg.setComboBoxItems(itemNames);
				dlg.setComboBoxEditable(true);
				dlg.setTextValue(text);

				ok = (dlg.exec() != 0);

				if (ok)
				{
					text = dlg.textValue();
				}
			}

			if (ok)
			{
				return QScriptValue(engine, text);
			}

			return QScriptValue(engine, QScriptValue::UndefinedValue);
		}

		default:
			break;
	}

	return ThrowBadNumberOfArguments(context);
}

static QScriptValue chooseItem(QScriptContext *context, QScriptEngine *engine)
{
	QString title = QApplication::applicationDisplayName();
	int argShift = -1;
	switch (context->argumentCount())
	{
		case 4:
			title = context->argument(0).toString();
			argShift = 0;
			Q_FALLTHROUGH();
		case 3:
		case 2:
		{
			auto items = context->argument(2 + argShift);
			if (not items.isArray())
			{
				return ThrowIncompatibleArgumentType(context, 3 + argShift);
			}

			int currentIndex = context->argument(3 + argShift).toInt32();

			int length =
				items.isArray() ? items.property(QSTRKEY(length)).toInt32() : 0;

			if (currentIndex < 0 || currentIndex >= length)
			{
				return ThrowIncompatibleArgumentType(context, 4 + argShift);
			}

			auto label = context->argument(1 + argShift).toString();

			QStringList itemNames;
			itemNames.reserve(length);

			for (int i = 0; i < length; i++)
			{
				auto originalStr = items.property(i).toString();
				QString indexedStr = originalStr;
				int j = 0;

				while (itemNames.indexOf(indexedStr) >= 0)
				{
					indexedStr =
						originalStr + QChar(' ') + QString::number(++j);
				}

				itemNames.append(indexedStr);
			}

			QInputDialog dlg;

			dlg.setWindowFlags(DialogFlags);
			dlg.setWindowTitle(title);
			dlg.setLabelText(label);

			dlg.setComboBoxItems(itemNames);
			dlg.setComboBoxEditable(false);
			dlg.setTextValue(itemNames.at(currentIndex));

			bool ok = (dlg.exec() != 0);

			if (ok)
			{
				int index = itemNames.indexOf(dlg.textValue());

				return items.property(index);
			}

			return QScriptValue(engine, QScriptValue::UndefinedValue);
		}
		default:
			break;
	}

	return ThrowBadNumberOfArguments(context);
}

static QScriptValue inputNumber(QScriptContext *context, QScriptEngine *engine)
{
	QString title = QApplication::applicationDisplayName();
	int argShift = -1;
	switch (context->argumentCount())
	{
		case 6:
			title = context->argument(0).toString();
			argShift = 0;
			Q_FALLTHROUGH();
		case 5:
		case 4:
		case 3:
		case 2:
		case 1:
		{
			auto label = context->argument(1 + argShift).toString();

			double value = context->argument(2 + argShift).toNumber();
			int precision = context->argument(3 + argShift).toInt32();

			bool ok = false;

			auto minV = context->argument(4 + argShift);
			auto maxV = context->argument(5 + argShift);
			double min = minV.isUndefined()
				? -std::numeric_limits<double>::infinity()
				: minV.toNumber();

			double max = maxV.isUndefined()
				? std::numeric_limits<double>::infinity()
				: maxV.toNumber();

			if (precision > 0)
			{
				double result = QInputDialog::getDouble(nullptr, title, label,
					value, min, max, precision, &ok, DialogFlags);

				if (ok)
				{
					return QScriptValue(engine, result);
				}
			} else
			{
				Q_CONSTEXPR int minInt = std::numeric_limits<int>::min();
				Q_CONSTEXPR int maxInt = std::numeric_limits<int>::max();
				Q_CONSTEXPR auto minIntF = double(minInt);
				Q_CONSTEXPR auto maxIntF = double(maxInt);

				int intValue;

				if (value < minIntF)
					intValue = minInt;
				else if (value > maxIntF)
					intValue = maxInt;
				else
					intValue = int(value);

				int result = QInputDialog::getInt(nullptr, title, label,
					intValue, int(std::max(minIntF, min)),
					int(std::min(maxIntF, max)), 1, &ok, DialogFlags);

				if (ok)
				{
					return QScriptValue(engine, result);
				}
			}

			return QScriptValue(engine, QScriptValue::UndefinedValue);
		}

		default:
			break;
	}

	return ThrowBadNumberOfArguments(context);
}

static QScriptValue propertyEditor(
	QScriptContext *context, QScriptEngine *engine)
{
	QString title;
	int argShift = -1;
	switch (context->argumentCount())
	{
		case 3:
			title = context->argument(0).toString();
			argShift = 0;
			Q_FALLTHROUGH();
		case 2:
		{
			auto label = context->argument(1 + argShift).toString();

			CustomPropertyEditorDialog dlg;
			dlg.setWindowModality(Qt::ApplicationModal);
			dlg.setWindowTitle(title);

			auto v = ScriptValueToVariant(context->argument(2 + argShift));

			if (dlg.execute(label, v))
			{
				return VariantToScriptValue(v, engine);
			}

			return QScriptValue(engine, QScriptValue::UndefinedValue);
		}

		default:
			break;
	}

	return ThrowBadNumberOfArguments(context);
}

static QScriptValue messageBox(QScriptContext *context, QScriptEngine *engine)
{
	QString title;
	int argShift = -1;
	switch (context->argumentCount())
	{
		case 4:
			title = context->argument(0).toString();
			argShift = 0;
			Q_FALLTHROUGH();
		case 3:
		{
			auto text = context->argument(1 + argShift).toString();
			int icon = context->argument(2 + argShift).toInt32();
			auto buttons = context->argument(3 + argShift);

			QMessageBox::StandardButtons btns(
				buttons.isNumber() ? buttons.toInt32() : 0);

			struct PushButton
			{
				QScriptValue thisObject;
				QScriptValue action;
				QPushButton *button;
				QMessageBox::ButtonRole role;
				bool isDefaultButton;
				bool isEscapeButton;

				void callToAction()
				{
					if (action.isFunction())
					{
						action.call(thisObject);
					}
				}
			};

			std::vector<PushButton> pushButtons;

			if (buttons.isArray())
			{
				auto length = buttons.property(QSTRKEY(length)).toInt32();

				for (int i = 0; i < length; i++)
				{
					auto arg = buttons.property(i);

					if (arg.isNumber())
					{
						btns |= QMessageBox::StandardButton(arg.toInt32());
					} else if (arg.isObject())
					{
						QScriptValue name = arg.property(QSTRKEY(name));

						if (name.isString())
						{
							auto pushButton = new QPushButton(name.toString());

							int role = QMessageBox::AcceptRole;

							QScriptValue roleValue =
								arg.property(QSTRKEY(role));

							if (roleValue.isNumber())
							{
								role = roleValue.toInt32();

								if (role < QMessageBox::InvalidRole ||
									role > QMessageBox::NRoles)
								{
									role = QMessageBox::InvalidRole;
								}
							}

							auto isDefault = arg.property(QSTRKEY(isDefault));
							auto isEscape = arg.property(QSTRKEY(isEscape));

							auto action = arg.property(QSTRKEY(action));

							pushButtons.emplace_back();
							PushButton &pb = pushButtons.back();
							pb.thisObject = arg;
							pb.action = action;
							pb.button = pushButton;
							pb.isDefaultButton = isDefault.isUndefined()
								? false
								: isDefault.toBool();
							pb.isEscapeButton = isEscape.isUndefined()
								? false
								: isEscape.toBool();
							pb.role = QMessageBox::ButtonRole(role);
						}
					}
				}
			}

			QMessageBox::Icon icn = QMessageBox::NoIcon;

			switch (icon)
			{
				case QMessageBox::NoIcon:
				case QMessageBox::Information:
				case QMessageBox::Warning:
				case QMessageBox::Critical:
				case QMessageBox::Question:
					icn = QMessageBox::Icon(icon);
					break;
			}

			QMessageBox dialog(icn, title, text, btns);
			dialog.QDialog::setWindowTitle(title);

			for (PushButton &entry : pushButtons)
			{
				QPushButton *button = entry.button;
				button->setParent(&dialog);

				dialog.addButton(button, entry.role);

				switch (entry.role)
				{
					case QMessageBox::InvalidRole:
					case QMessageBox::ActionRole:
					case QMessageBox::ResetRole:
					case QMessageBox::ApplyRole:
					case QMessageBox::HelpRole:
					case QMessageBox::NRoles:
						button->disconnect();
						QObject::connect(button, &QAbstractButton::clicked,
							[&entry]() { entry.callToAction(); });
						break;

					case QMessageBox::AcceptRole:
					case QMessageBox::RejectRole:
					case QMessageBox::YesRole:
					case QMessageBox::NoRole:
					case QMessageBox::DestructiveRole:
						if (entry.isDefaultButton)
						{
							dialog.setDefaultButton(button);
						}
						if (entry.isEscapeButton)
						{
							dialog.setEscapeButton(button);
						}
						break;
				}
			}

			/*
var button = {};
button.name = "Black";
button.role = QButton.ActionRole;
button.action = function() { print(this); };
button.toString = function() { return this.name; }
print(system.messageBox("Hello World!", QIcon.Warning, [button, QButton.Ok]));
*/

			auto escapeButton = dialog.escapeButton();
			if (!escapeButton)
			{
				auto buttons = dialog.buttons();
				for (auto button : buttons)
				{
					if (dialog.buttonRole(button) == QMessageBox::RejectRole)
					{
						if (escapeButton)
						{
							escapeButton = nullptr;
							break;
						}
						escapeButton = button;
					}
				}

				int closeButtonCount = 0;

				for (auto button : buttons)
				{
					switch (dialog.buttonRole(button))
					{
						case QMessageBox::InvalidRole:
						case QMessageBox::ActionRole:
						case QMessageBox::ResetRole:
						case QMessageBox::ApplyRole:
						case QMessageBox::HelpRole:
						case QMessageBox::NRoles:
							if (dialog.standardButton(button) ==
								QMessageBox::NoButton)
							{
								break;
							}
							Q_FALLTHROUGH();
						case QMessageBox::AcceptRole:
						case QMessageBox::RejectRole:
						case QMessageBox::YesRole:
						case QMessageBox::NoRole:
						case QMessageBox::DestructiveRole:
							closeButtonCount++;
							if (!escapeButton && buttons.count() == 1)
							{
								escapeButton = button;
							}
							break;
					}
				}

				if (!escapeButton && closeButtonCount == 0)
				{
					escapeButton = dialog.addButton(QMessageBox::Close);
					dialog.setDefaultButton(QMessageBox::Close);
				}
			}

			if (escapeButton)
			{
				dialog.setEscapeButton(escapeButton);
			} else
			{
				dialog.setWindowFlags(
					Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
			}

			if (dialog.exec() == -1)
			{
				return QScriptValue(engine, int(QMessageBox::Cancel));
			}

			auto clickedButton = dialog.clickedButton();

			for (PushButton &entry : pushButtons)
			{
				if (clickedButton == entry.button)
				{
					entry.callToAction();
					return entry.thisObject;
				}
			}

			return QScriptValue(
				engine, int(dialog.standardButton(clickedButton)));
		}

		default:
			break;
	}

	return ThrowBadNumberOfArguments(context);
}

static bool resolveTitleDir(
	QScriptContext *context, QScriptValue &error, QString &title, QString &dir)
{
	int argc = context->argumentCount();
	if (argc < 1)
	{
		error = ThrowBadNumberOfArguments(context);
		return false;
	}

	auto arg = context->argument(0);
	if (not arg.isString())
	{
		error = ThrowIncompatibleArgumentType(context, 1);
		return false;
	}

	title = arg.toString();

	dir.clear();

	if (argc >= 2)
	{
		arg = context->argument(1);
		if (not arg.isString())
		{
			error = ThrowIncompatibleArgumentType(context, 2);
			return false;
		}

		dir = arg.toString();
	}

	return true;
}

static bool resolveTitleDirFilters(QScriptContext *context, QScriptValue &error,
	QString &title, QString &dir, QString &filters)
{
	if (not resolveTitleDir(context, error, title, dir))
		return false;

	if (context->argumentCount() >= 3)
	{
		auto arg = context->argument(2);
		if (not arg.isString())
		{
			error = ThrowIncompatibleArgumentType(context, 3);
			return false;
		}

		filters = arg.toString();
	}

	return true;
}

static QScriptValue requestExistingFilePath(
	QScriptContext *context, QScriptEngine *engine)
{
	QScriptValue result;
	QString title;
	QString dir;
	QString filters;
	if (not resolveTitleDirFilters(context, result, title, dir, filters))
		return result;

	ScriptSubDialogHandler dialogHandler(context);

	if (dir.isEmpty())
		dir = dialogHandler.defaultDir();
	return QScriptValue(engine,
		QFileDialog::getOpenFileName(
			nullptr, title, dir, filters, nullptr, FILE_DIALOG_FLAGS));
}

static QScriptValue requestExistingFilePaths(
	QScriptContext *context, QScriptEngine *engine)
{
	QScriptValue result;
	QString title;
	QString dir;
	QString filters;
	if (not resolveTitleDirFilters(context, result, title, dir, filters))
		return result;

	ScriptSubDialogHandler dialogHandler(context);

	if (dir.isEmpty())
		dir = dialogHandler.defaultDir();

	auto fileNames = QFileDialog::getOpenFileNames(
		nullptr, title, dir, filters, nullptr, FILE_DIALOG_FLAGS);

	auto count = quint32(fileNames.length());
	result = engine->newArray(count);

	for (quint32 i = 0; i < count; i++)
	{
		result.setProperty(i, QScriptValue(engine, fileNames.at(i)));
	}

	return result;
}

static QScriptValue requestNewFilePath(
	QScriptContext *context, QScriptEngine *engine)
{
	QScriptValue result;
	QString title;
	QString dir;
	QString filters;
	if (not resolveTitleDirFilters(context, result, title, dir, filters))
		return result;

	ScriptSubDialogHandler dialogHandler(context);
	if (dir.isEmpty())
		dir = dialogHandler.defaultDir();
	return QScriptValue(engine,
		QFileDialog::getSaveFileName(
			nullptr, title, dir, filters, nullptr, FILE_DIALOG_FLAGS));
}

static QScriptValue requestDirectoryPath(
	QScriptContext *context, QScriptEngine *engine)
{
	QScriptValue result;
	QString title;
	QString dir;

	if (not resolveTitleDir(context, result, title, dir))
		return result;

	ScriptSubDialogHandler dialogHandler(context);

	if (dir.isEmpty())
		dir = dialogHandler.defaultDir();
	return QScriptValue(engine,
		QFileDialog::getExistingDirectory(nullptr, title, dir,
			QFileDialog::ShowDirsOnly | FILE_DIALOG_FLAGS));
}

ScriptRunnerDialog::ScriptRunnerDialog(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::ScriptRunnerDialog)
	, group(nullptr)
	, runner(nullptr)
	, abortDelegate(nullptr)
	, lastRespondTime(0)
	, timerId(0)
	, running(false)
	, checkRespond(false)
	, waiting(false)
	, stopShow(false)
{
	ui->setupUi(this);
	setWindowFlags((windowFlags() & ~(Qt::WindowContextHelpButtonHint)) |
		Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint);

	ui->textAreaLog->setMaximumBlockCount(2000);

	hideProgressBar();
}

ScriptRunnerDialog::~ScriptRunnerDialog()
{
	delete ui;
}

void ScriptRunnerDialog::showModal(
	ScriptRunner *runner, ProjectGroup *group, const QString &scriptFilePath)
{
	Q_ASSERT(nullptr != runner);
	this->runner = runner;

	ui->widgetArguments->show();
	ui->buttonBrowse->show();
	ui->runButton->setText(getRunText());
	ui->runButton->show();
	ui->editScriptFile->setReadOnly(false);

	ui->textAreaLog->clear();

	this->group = group;

	setAbortDelegate(runner);
	setScriptFilePath(scriptFilePath);
	stopShow = false;

	setModal(true);
	show();
	raise();
	exec();
	this->runner = nullptr;
}

bool ScriptRunnerDialog::abort()
{
	if (running)
	{
		ScriptSubDialogHandler handler(this);

		stopShow = true;
		if (QMessageBox::Yes !=
			QMessageBox::question(nullptr, QString(),
				tr("Do you want to abort script execution?"),
				QMessageBox::Yes | QMessageBox::No))
		{
			stopShow = false;
			return false;
		}
	}

	if (abortDelegate)
		return abortDelegate->abort();

	return true;
}

void ScriptRunnerDialog::restoreCursor()
{
	if (cursorIsSet)
	{
		QApplication::restoreOverrideCursor();
		cursorIsSet = false;
	}
}

void ScriptRunnerDialog::initializeEngine(QScriptEngine *engine)
{
	auto globalObject = engine->globalObject();

	auto system = globalObject.property(QSTRKEY(system));

	auto thiz = engine->newQObject(this);

	system.setProperty(QSTRKEY(requestExistingFilePath),
		engine->newFunction(requestExistingFilePath, thiz));
	system.setProperty(QSTRKEY(requestExistingFilePaths),
		engine->newFunction(requestExistingFilePaths, thiz));
	system.setProperty(QSTRKEY(requestNewFilePath),
		engine->newFunction(requestNewFilePath, thiz));
	system.setProperty(QSTRKEY(requestDirectoryPath),
		engine->newFunction(requestDirectoryPath, thiz));

	system.setProperty(QSTRKEY(inputString), engine->newFunction(inputString));
	system.setProperty(QSTRKEY(chooseItem), engine->newFunction(chooseItem));
	system.setProperty(QSTRKEY(inputNumber), engine->newFunction(inputNumber));
	system.setProperty(
		QSTRKEY(propertyEditor), engine->newFunction(propertyEditor));
	system.setProperty(QSTRKEY(messageBox), engine->newFunction(messageBox));

	auto qIconObject = engine->newObject();
	auto qButtonObject = engine->newObject();

	enum
	{
		Information = QMessageBox::Information,
		Warning = QMessageBox::Warning,
		Critical = QMessageBox::Critical,
		Question = QMessageBox::Question,
		NoButton = QMessageBox::NoButton,
		Ok = QMessageBox::Ok,
		Save = QMessageBox::Save,
		SaveAll = QMessageBox::SaveAll,
		Open = QMessageBox::Open,
		Yes = QMessageBox::Yes,
		YesToAll = QMessageBox::YesToAll,
		No = QMessageBox::No,
		NoToAll = QMessageBox::NoToAll,
		Abort = QMessageBox::Abort,
		Retry = QMessageBox::Retry,
		Ignore = QMessageBox::Ignore,
		Close = QMessageBox::Close,
		Cancel = QMessageBox::Cancel,
		Discard = QMessageBox::Discard,
		Help = QMessageBox::Help,
		Apply = QMessageBox::Apply,
		Reset = QMessageBox::Reset,
		RestoreDefaults = QMessageBox::RestoreDefaults,
		InvalidRole = QMessageBox::InvalidRole,
		AcceptRole = QMessageBox::AcceptRole,
		RejectRole = QMessageBox::RejectRole,
		DestructiveRole = QMessageBox::DestructiveRole,
		ActionRole = QMessageBox::ActionRole,
		HelpRole = QMessageBox::HelpRole,
		YesRole = QMessageBox::YesRole,
		NoRole = QMessageBox::NoRole,
		ResetRole = QMessageBox::ResetRole,
		ApplyRole = QMessageBox::ApplyRole,
		NRoles = QMessageBox::NRoles,
	};

	SCRIPT_REG_ENUM(qIconObject, Information);
	SCRIPT_REG_ENUM(qIconObject, Warning);
	SCRIPT_REG_ENUM(qIconObject, Critical);
	SCRIPT_REG_ENUM(qIconObject, Question);

	SCRIPT_REG_ENUM(qButtonObject, NoButton);
	SCRIPT_REG_ENUM(qButtonObject, Ok);
	SCRIPT_REG_ENUM(qButtonObject, Save);
	SCRIPT_REG_ENUM(qButtonObject, SaveAll);
	SCRIPT_REG_ENUM(qButtonObject, Open);
	SCRIPT_REG_ENUM(qButtonObject, Yes);
	SCRIPT_REG_ENUM(qButtonObject, YesToAll);
	SCRIPT_REG_ENUM(qButtonObject, No);
	SCRIPT_REG_ENUM(qButtonObject, NoToAll);
	SCRIPT_REG_ENUM(qButtonObject, Abort);
	SCRIPT_REG_ENUM(qButtonObject, Retry);
	SCRIPT_REG_ENUM(qButtonObject, Ignore);
	SCRIPT_REG_ENUM(qButtonObject, Close);
	SCRIPT_REG_ENUM(qButtonObject, Cancel);
	SCRIPT_REG_ENUM(qButtonObject, Discard);
	SCRIPT_REG_ENUM(qButtonObject, Help);
	SCRIPT_REG_ENUM(qButtonObject, Apply);
	SCRIPT_REG_ENUM(qButtonObject, Reset);
	SCRIPT_REG_ENUM(qButtonObject, RestoreDefaults);

	SCRIPT_REG_ENUM(qButtonObject, InvalidRole);
	SCRIPT_REG_ENUM(qButtonObject, AcceptRole);
	SCRIPT_REG_ENUM(qButtonObject, RejectRole);
	SCRIPT_REG_ENUM(qButtonObject, DestructiveRole);
	SCRIPT_REG_ENUM(qButtonObject, ActionRole);
	SCRIPT_REG_ENUM(qButtonObject, HelpRole);
	SCRIPT_REG_ENUM(qButtonObject, YesRole);
	SCRIPT_REG_ENUM(qButtonObject, NoRole);
	SCRIPT_REG_ENUM(qButtonObject, ResetRole);
	SCRIPT_REG_ENUM(qButtonObject, ApplyRole);
	SCRIPT_REG_ENUM(qButtonObject, NRoles);

	globalObject.setProperty(QSTRKEY(QIcon), qIconObject, STATIC_SCRIPT_VALUE);
	globalObject.setProperty(
		QSTRKEY(QButton), qButtonObject, STATIC_SCRIPT_VALUE);
}

void ScriptRunnerDialog::beforeScriptExecution(const QString &filePath)
{
	timerId = startTimer(1000);
	Q_ASSERT(not running);
	running = true;
	beginWait();

	checkRespond = true;
	lastRespondTime = QDateTime::currentMSecsSinceEpoch();

	ui->runButton->setText(getAbortText());
	ui->runButton->show();

	ui->textAreaLog->clear();
	if (runner)
	{
		ui->widgetScriptFile->setEnabled(false);
		ui->argButtonGroup->setEnabled(false);
	} else
	{
		ui->widgetArguments->hide();
		ui->buttonBrowse->hide();
		ui->editScriptFile->setReadOnly(true);
		setScriptFilePath(filePath, false);
		emit shouldDisableParentWindow();
	}

	stopShow = false;

	showProgressBar();
}

void ScriptRunnerDialog::afterScriptExecution(bool ok, const QString &message)
{
	killTimer(timerId);
	checkRespond = false;
	endWait();

	Q_ASSERT(running);
	running = false;

	if (not ok)
		scriptRuntimeError(message);

	hideProgressBar();

	if (runner)
	{
		ui->runButton->setText(getRunText());
		ui->argButtonGroup->setEnabled(true);
		ui->widgetScriptFile->setEnabled(true);
	} else
	{
		ui->runButton->hide();
		emit shouldEnableParentWindow();

		if (isVisible())
			exec();
	}
}

void ScriptRunnerDialog::log(const QString &text)
{
	qint64 newRespondTime = QDateTime::currentMSecsSinceEpoch();
	bool shouldProcess = newRespondTime - lastRespondTime >= 200;
	lastRespondTime = newRespondTime;
	checkRespond = true;

	if (nullptr == runner && not stopShow && not isVisible())
	{
		showMe();
	}

	auto view = ui->textAreaLog;
	view->moveCursor(QTextCursor::End);
	view->insertPlainText(text + QLatin1Char('\n'));
	if (shouldProcess)
	{
		QApplication::processEvents();
	}
}

void ScriptRunnerDialog::on_buttonBrowse_clicked()
{
	QString filter(Directory::getFilterForExtension(szScriptExtension));

	auto filepath = QFileDialog::getOpenFileName(this, tr("Select Script File"),
		ui->editScriptFile->text(), filter, &filter, FILE_DIALOG_FLAGS);

	if (!filepath.isEmpty())
		setScriptFilePath(filepath);
}

void ScriptRunnerDialog::runScript()
{
	registerScript();

	auto filePath = ui->editScriptFile->text();

	if (not filePath.isEmpty() && not QFile::exists(filePath))
	{
		QMessageBox::critical(this, QCoreApplication::applicationName(),
			tr("File '%1' is not found.").arg(filePath));
	} else
	{
		runner->setDelegate(this);
		runner->execute(filePath, ui->editArguments->toPlainText());
	}
}

void ScriptRunnerDialog::setScriptFilePath(const QString &filepath, bool reg)
{
	ui->editScriptFile->setText(QDir::toNativeSeparators(filepath));

	if (reg && !filepath.isEmpty())
	{
		QSettings settings;

		settings.beginGroup(sScriptRunnerGroup);

		auto args = settings.value(filepath, QString());

		if (args.type() == QVariant::String)
			ui->editArguments->setPlainText(args.toString());

		settings.endGroup();
	}
}

void ScriptRunnerDialog::registerScript()
{
	auto filepath = ui->editScriptFile->text();

	if (QFileInfo(filepath).isFile())
	{
		emit registerFilePath(filepath);

		auto args = ui->editArguments->toPlainText();

		if (!args.isEmpty())
		{
			QSettings settings;

			settings.beginGroup(sScriptRunnerGroup);

			settings.setValue(filepath, args);

			settings.endGroup();
		}
	}
}

void ScriptRunnerDialog::on_btnInsertFilePath_clicked()
{
	QString filter;
	auto filePaths = QFileDialog::getOpenFileNames(this, tr("Select Files"),
		lastFilePath, filter, &filter, FILE_DIALOG_FLAGS);

	if (!filePaths.isEmpty())
	{
		lastFilePath = filePaths.at(0);

		insertList(filePaths);
	}
}

void ScriptRunnerDialog::on_btnInsertDirectoryPath_clicked()
{
	auto path = QFileDialog::getExistingDirectory(this, tr("Select Directory"),
		lastFilePath, QFileDialog::ShowDirsOnly | FILE_DIALOG_FLAGS);

	if (!path.isEmpty())
	{
		lastFilePath = path;

		insertList({ path });
	}
}

void ScriptRunnerDialog::on_btnInsertProjectItem_clicked()
{
	auto project_dir = group->getActiveProjectDirectory();

	if (nullptr != project_dir)
	{
		FileSelectDialog dialog(
			group->getDelegate()->getProjectTreeModel(), this);

		if (dialog.executeMultiSelect())
		{
			insertList(dialog.getSelectedFilePathList(true));
		}
	} else
	{
		QMessageBox::critical(this, QCoreApplication::applicationName(),
			tr("There is no active project directory."));
	}
}

void ScriptRunnerDialog::closeEvent(QCloseEvent *event)
{
	if (not abort())
	{
		event->ignore();
	} else
	{
		QDialog::closeEvent(event);
	}
}

void ScriptRunnerDialog::timerEvent(QTimerEvent *event)
{
	QDialog::timerEvent(event);

	if (running && checkRespond && waiting)
	{
		auto newTime = QDateTime::currentMSecsSinceEpoch();
		if (newTime - lastRespondTime >= CHECK_RESPOND_SECONDS * 1000)
		{
			log(tr("Not responding more than %1 seconds.")
					.arg(CHECK_RESPOND_SECONDS));
			checkRespond = false;
		}
	}
}

void ScriptRunnerDialog::showMe()
{
	emit shouldEnableParentWindow();
	setModal(true);
	show();
	raise();

	if (running)
	{
		setWaitCursor();
	}
}

void ScriptRunnerDialog::setWaitCursor()
{
	restoreCursor();
	QApplication::setOverrideCursor(Qt::WaitCursor);
	cursorIsSet = true;
}

void ScriptRunnerDialog::beginWait()
{
	Q_ASSERT(not waiting);
	setWaitCursor();
	waiting = true;
}

void ScriptRunnerDialog::endWait()
{
	restoreCursor();
	waiting = false;
}

void ScriptRunnerDialog::showProgressBar()
{
	ui->horizontalSpacer->changeSize(
		0, 20, QSizePolicy::Ignored, QSizePolicy::Maximum);
	ui->progressBar->show();
}

void ScriptRunnerDialog::hideProgressBar()
{
	ui->horizontalSpacer->changeSize(
		40, 20, QSizePolicy::Expanding, QSizePolicy::Maximum);
	ui->progressBar->hide();
}

void ScriptRunnerDialog::scriptRuntimeError(const QString &message)
{
	if (not message.isEmpty())
	{
		QMessageBox::critical(this, tr("Script Runtime Error"), message);
	}
}

void ScriptRunnerDialog::insertList(const QStringList &value)
{
	auto cursor = ui->editArguments->textCursor();

	cursor.removeSelectedText();

	int count = value.count();

	if (count == 1)
		cursor.insertText(Utils::QuoteString(value.at(0)));
	else
	{
		QString str = "[\n";

		for (int i = 0; i < count; i++)
		{
			str.append(Utils::QuoteString(value.at(i)));

			if (i < count - 1)
				str.append(',');

			str.append('\n');
		}

		str.append("]");

		cursor.insertText(str);
	}
}

void ScriptRunnerDialog::on_closeButton_clicked()
{
	stopShow = true;
	close();
}

void ScriptRunnerDialog::on_runButton_clicked()
{
	if (running)
	{
		abort();
	} else
	{
		runScript();
	}
}

QString ScriptRunnerDialog::getRunText()
{
	return tr("Run");
}

QString ScriptRunnerDialog::getAbortText()
{
	return tr("Abort");
}

ScriptSubDialogHandler::ScriptSubDialogHandler(QScriptContext *context)
{
	auto callee = context->callee();

	auto prototype = callee.property(QSTRKEY(prototype));
	mDialog = static_cast<ScriptRunnerDialog *>(prototype.toQObject());
	init();
}

ScriptSubDialogHandler::ScriptSubDialogHandler(ScriptRunnerDialog *dialog)
	: mDialog(dialog)
{
	init();
}

ScriptSubDialogHandler::~ScriptSubDialogHandler()
{
	Q_ASSERT(nullptr != mDialog);
	mDialog->beginWait();
}

QString ScriptSubDialogHandler::defaultDir() const
{
	if (mDialog->group)
	{
		auto projectDir = mDialog->group->getActiveProjectDirectory();
		if (projectDir)
			return projectDir->getFilePath();
	}

	return QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
}

void ScriptSubDialogHandler::init()
{
	Q_ASSERT(nullptr != mDialog);
	mDialog->endWait();
}
}
