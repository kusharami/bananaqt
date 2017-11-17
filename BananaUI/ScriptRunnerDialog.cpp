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

#include "BananaCore/ProjectGroup.h"
#include "BananaCore/Config.h"
#include "BananaCore/ScriptRunner.h"
#include "BananaCore/ScriptTemplates.h"
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

using namespace Banana;
#include "ui_ScriptRunnerDialog.h"

using namespace Scripting;

namespace Banana
{
struct ScriptSubDialogHandler
{
	ScriptRunnerDialog *mDialog;
	bool mShouldResumeTimer;

	ScriptSubDialogHandler(QScriptContext *context);
	~ScriptSubDialogHandler();

	QString defaultDir() const;
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
		error = IncompatibleArgumentType(context, 0);
		return false;
	}

	title = arg.toString();

	dir.clear();

	if (argc >= 2)
	{
		arg = context->argument(1);
		if (not arg.isString())
		{
			error = IncompatibleArgumentType(context, 1);
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

	if (context->argumentCount() >= 2)
	{
		auto arg = context->argument(2);
		if (not arg.isString())
		{
			error = IncompatibleArgumentType(context, 2);
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
	, timer(nullptr)
	, runner(nullptr)
	, abortDelegate(nullptr)
	, execCount(0)
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
	delete timer;
	delete ui;
}

void ScriptRunnerDialog::showModal(
	ScriptRunner *runner, ProjectGroup *group, const QString &scriptFilePath)
{
	Q_ASSERT(nullptr != runner);
	this->runner = runner;

	ui->widgetArguments->show();
	ui->buttonBrowse->show();
	ui->runButton->show();
	ui->editScriptFile->setReadOnly(false);

	ui->textAreaLog->clear();

	this->group = group;

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
	if (execCount > 0)
	{
		stopShow = true;
		if (QMessageBox::Yes !=
			QMessageBox::question(nullptr, QApplication::applicationName(),
				tr("Do you want to abort script execution?"),
				QMessageBox::Yes | QMessageBox::No))
		{
			stopShow = false;
			if (nullptr == runner && timer == nullptr && not isVisible())
			{
				showMe();
			}
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
	system.setProperty(QSTRKEY(requestNewFilePath),
		engine->newFunction(requestNewFilePath, thiz));
	system.setProperty(QSTRKEY(requestDirectoryPath),
		engine->newFunction(requestDirectoryPath, thiz));
}

void ScriptRunnerDialog::beforeScriptExecution(const QString &filePath)
{
	execCount++;
	beginWait();

	ui->textAreaLog->clear();
	if (runner)
	{
		ui->widgetScriptFile->setEnabled(false);
		ui->argButtonGroup->setEnabled(false);
		ui->runButton->setEnabled(false);
	} else
	{
		ui->widgetArguments->hide();
		ui->buttonBrowse->hide();
		ui->runButton->hide();
		ui->editScriptFile->setReadOnly(true);
		setScriptFilePath(filePath, false);
		emit shouldDisableParentWindow();
	}

	startTimer();
	stopShow = false;

	showProgressBar();
}

void ScriptRunnerDialog::afterScriptExecution(bool ok, const QString &message)
{
	endTimer();

	endWait();

	if (not ok)
		scriptRuntimeError(message);

	hideProgressBar();

	Q_ASSERT(execCount > 0);
	execCount--;

	if (runner)
	{
		ui->runButton->setEnabled(true);
		ui->argButtonGroup->setEnabled(true);
		ui->widgetScriptFile->setEnabled(true);
	} else
	{
		emit shouldEnableParentWindow();

		if (isVisible())
			exec();
	}
}

void ScriptRunnerDialog::timeout()
{
	endTimer();
	if (execCount > 0)
	{
		log(tr("Not responding more than 5 seconds."));
	}
}

void ScriptRunnerDialog::log(const QString &text)
{
	if (nullptr == runner && not stopShow && not isVisible())
	{
		showMe();
	}

	auto view = ui->textAreaLog;
	view->moveCursor(QTextCursor::End);
	view->insertPlainText(text + "\n");
}

void ScriptRunnerDialog::on_buttonBrowse_clicked()
{
	QString filter(
		Directory::getFilterForExtension(Scripting::szScriptExtension));

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

	if (QFile::exists(filepath))
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

void ScriptRunnerDialog::endTimer()
{
	if (timer)
	{
		timer->stop();
		delete timer;
		timer = nullptr;
	}
}

void ScriptRunnerDialog::showMe()
{
	emit shouldEnableParentWindow();
	setModal(true);
	show();
	raise();

	if (execCount > 0)
	{
		beginWait();
	}
}

void ScriptRunnerDialog::beginWait()
{
	QApplication::restoreOverrideCursor();
	QApplication::setOverrideCursor(Qt::WaitCursor);
}

void ScriptRunnerDialog::endWait()
{
	QApplication::restoreOverrideCursor();
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
	runScript();
}

void ScriptRunnerDialog::startTimer()
{
	delete timer;
	timer = new QTimer;
	timer->setInterval(5000);
	timer->start();
	QObject::connect(
		timer, &QTimer::timeout, this, &ScriptRunnerDialog::timeout);
}

ScriptSubDialogHandler::ScriptSubDialogHandler(QScriptContext *context)
	: mShouldResumeTimer(false)
{
	auto callee = context->callee();

	auto prototype = callee.property(QSTRKEY(prototype));
	mDialog = static_cast<ScriptRunnerDialog *>(prototype.toQObject());
	Q_ASSERT(nullptr != mDialog);
	if (mDialog->timer != nullptr)
	{
		mDialog->endTimer();
		mShouldResumeTimer = true;
	}
	mDialog->endWait();
}

ScriptSubDialogHandler::~ScriptSubDialogHandler()
{
	if (mShouldResumeTimer)
	{
		mDialog->startTimer();
	}
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
}
