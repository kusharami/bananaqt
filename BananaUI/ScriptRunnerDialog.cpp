/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2016 Alexandra Cherdantseva

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
#include "BananaCore/AbstractProjectDirectory.h"
#include "BananaCore/IProjectGroupDelegate.h"
#include "BananaCore/Utils.h"

#include "Config.h"
#include "FileSelectDialog.h"

#include <QFileDialog>
#include <QSettings>
#include <QPlainTextEdit>
#include <QTextBlock>
#include <QMessageBox>

using namespace Banana;
#include "ui_ScriptRunnerDialog.h"

namespace Banana
{

static const QString sScriptRunnerGroup = QStringLiteral("ScriptRunner");

ScriptRunnerDialog::ScriptRunnerDialog(ScriptRunner *runner, QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::ScriptRunnerDialog)
	, group(nullptr)
	, runner(runner)
{
	Q_ASSERT(nullptr != runner);

	ui->setupUi(this);
	setWindowFlags(
		(windowFlags() &
		 ~(Qt::WindowContextHelpButtonHint | Qt::WindowMinMaxButtonsHint)));

	QObject::connect(
		runner, &ScriptRunner::logPrint, this,
		&ScriptRunnerDialog::onLogPrint);

}

ScriptRunnerDialog::~ScriptRunnerDialog()
{
	delete ui;
}

void ScriptRunnerDialog::execute(ProjectGroup *group,
								 const QString &script_filepath)
{
	this->group = group;

	setScriptFilePath(script_filepath);

	show();
	raise();
	exec();
}

void ScriptRunnerDialog::onLogPrint(const QString &text)
{
	auto view = ui->textAreaLog;
	view->moveCursor(QTextCursor::End);
	view->insertPlainText(text + "\n");
	QApplication::processEvents();
}

void ScriptRunnerDialog::on_buttonBrowse_clicked()
{
	QString filter(Directory::getFilterForExtension(
					   Scripting::szScriptExtension));

	auto filepath = QFileDialog::getOpenFileName(
			this, tr("Select Script File"), ui->editScriptFile->text(),
			filter, &filter, FILE_DIALOG_FLAGS);

	if (!filepath.isEmpty())
		setScriptFilePath(filepath);
}

void ScriptRunnerDialog::on_buttonBox_clicked(QAbstractButton *button)
{
	switch (ui->buttonBox->buttonRole(button))
	{
		case QDialogButtonBox::ApplyRole:
			apply();
			break;

		default:
			accept();
			break;
	}
}

void ScriptRunnerDialog::apply()
{
	registerScript();

	auto filepath = ui->editScriptFile->text();

	if (filepath.isEmpty())
	{
		QMessageBox::critical(
			this, QCoreApplication::applicationName(),
			tr("Script file is not selected."));
	} else
	if (!QFile::exists(filepath))
	{
		QMessageBox::critical(
			this, QCoreApplication::applicationName(),
			tr("File '%1' is not found.").arg(filepath));
	} else
	{
		runner->setParentWidget(this);

		ui->textAreaLog->setPlainText(QString());

		setEnabled(false);

		if (!runner->execute(filepath, ui->editArguments->toPlainText()))
		{
			QMessageBox::critical(
				this, tr(
					"Execute Script Error"), runner->getErrorMessage());
		}

		setEnabled(true);
	}
}

void ScriptRunnerDialog::setScriptFilePath(const QString &filepath)
{
	ui->editScriptFile->setText(filepath);

	if (!filepath.isEmpty())
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
	auto filePaths = QFileDialog::getOpenFileNames(
			this, tr("Select Files"), lastFilePath, filter, &filter,
			FILE_DIALOG_FLAGS |
			QFileDialog::DontResolveSymlinks);

	if (!filePaths.isEmpty())
	{
		lastFilePath = filePaths.at(0);

		insertList(filePaths);
	}
}

void ScriptRunnerDialog::on_btnInsertDirectoryPath_clicked()
{
	auto path = QFileDialog::getExistingDirectory(
			this, tr("Select Directory"), lastFilePath,
			QFileDialog::ShowDirsOnly |
			FILE_DIALOG_FLAGS |
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
		QMessageBox::critical(
			this, QCoreApplication::applicationName(),
			tr("There is no active project directory."));
	}
}

void ScriptRunnerDialog::closeEvent(QCloseEvent *event)
{
	if (!isEnabled())
		event->ignore();
	else
		QDialog::closeEvent(event);
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

}
