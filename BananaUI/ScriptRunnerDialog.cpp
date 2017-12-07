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

using namespace Banana;
#include "ui_ScriptRunnerDialog.h"

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

static const QString sScriptRunnerGroup = QStringLiteral("ScriptRunner");

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
		QFileDialog::getOpenFileName(nullptr, title, dir, filters, nullptr,
			FILE_DIALOG_FLAGS | QFileDialog::DontResolveSymlinks));
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

	auto fileNames = QFileDialog::getOpenFileNames(nullptr, title, dir, filters,
		nullptr, FILE_DIALOG_FLAGS | QFileDialog::DontResolveSymlinks);

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
		QFileDialog::getSaveFileName(nullptr, title, dir, filters, nullptr,
			FILE_DIALOG_FLAGS | QFileDialog::DontResolveSymlinks));
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
			QFileDialog::ShowDirsOnly | FILE_DIALOG_FLAGS |
				QFileDialog::DontResolveSymlinks));
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
			QMessageBox::question(nullptr, QApplication::applicationName(),
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
	lastRespondTime = QDateTime::currentMSecsSinceEpoch();
	checkRespond = true;

	if (nullptr == runner && not stopShow && not isVisible())
	{
		showMe();
	}

	auto view = ui->textAreaLog;
	view->moveCursor(QTextCursor::End);
	view->insertPlainText(text + QLatin1Char('\n'));
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
		lastFilePath, filter, &filter,
		FILE_DIALOG_FLAGS | QFileDialog::DontResolveSymlinks);

	if (!filePaths.isEmpty())
	{
		lastFilePath = filePaths.at(0);

		insertList(filePaths);
	}
}

void ScriptRunnerDialog::on_btnInsertDirectoryPath_clicked()
{
	auto path = QFileDialog::getExistingDirectory(this, tr("Select Directory"),
		lastFilePath,
		QFileDialog::ShowDirsOnly | FILE_DIALOG_FLAGS |
			QFileDialog::DontResolveSymlinks);

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
		if (newTime - lastRespondTime >= 5000)
		{
			log(tr("Not responding more than 5 seconds."));
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
	QApplication::restoreOverrideCursor();
	QApplication::setOverrideCursor(Qt::WaitCursor);
}

void ScriptRunnerDialog::beginWait()
{
	Q_ASSERT(not waiting);
	setWaitCursor();
	waiting = true;
}

void ScriptRunnerDialog::endWait()
{
	QApplication::restoreOverrideCursor();
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
