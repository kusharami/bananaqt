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

#include "ScriptManagerDialog.h"
#include "ui_ScriptManagerDialog.h"

#include "BananaCore/Directory.h"
#include "BananaCore/AbstractProjectFile.h"

#include "QtnProperty/PropertyView.h"

#include "Utils.h"

#include <QMenu>

using namespace Banana;

ScriptManagerDialog::ScriptManagerDialog(
	ScriptManager *manager, QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::ScriptManagerDialog)
	, mManager(manager)
	, mPopup(false)
{
	Q_ASSERT(nullptr != mManager);
	ui->setupUi(this);

	setWindowFlags((windowFlags() & ~(Qt::WindowContextHelpButtonHint)) |
		Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint);

	auto projectFile = dynamic_cast<AbstractProjectFile *>(manager->owner());
	if (projectFile)
	{
		auto topDir = projectFile->getTopDirectory();
		if (nullptr != topDir)
		{
			ui->entriesWidget->setRootDirectory(topDir->getFilePath());
		}
	}

	ui->entriesWidget->setEntries(manager->scriptEntries());

	updateActions();

	QObject::connect(
		mManager, &QObject::destroyed, this, &ScriptManagerDialog::reject);

#ifdef Q_OS_MAC
	Utils::addShortcutForAction(
		this, QKeySequence(Qt::Key_Backspace), ui->actionDeleteAction);
#endif

	Utils::addShortcutForAction(
		this, ui->actionDeleteAction->shortcut(), ui->actionDeleteAction, true);

	QObject::connect(ui->entriesWidget->propertyView(),
		&QtnPropertyView::activePropertyChanged, this,
		&ScriptManagerDialog::updateActions);
}

ScriptManagerDialog::~ScriptManagerDialog()
{
	QObject::disconnect(ui->entriesWidget->propertyView(),
		&QtnPropertyView::activePropertyChanged, this,
		&ScriptManagerDialog::updateActions);
	delete ui;
}

void ScriptManagerDialog::accept()
{
	if (mPopup)
		return;

	mManager->setScriptEntries(ui->entriesWidget->entries());

	QDialog::accept();
}

void ScriptManagerDialog::reject()
{
	if (mPopup)
		return;

	QDialog::reject();
}

void ScriptManagerDialog::on_actionNewAction_triggered()
{
	ui->entriesWidget->addEntry(ScriptManager::Entry());
	updateActions();
}

void ScriptManagerDialog::on_actionDeleteAction_triggered()
{
	auto ew = ui->entriesWidget;
	ew->deleteProperty(ew->getActiveEntryProperty());
	updateActions();
}

void ScriptManagerDialog::on_actionDeleteAll_triggered()
{
	ui->entriesWidget->clear();
	updateActions();
}

void ScriptManagerDialog::updateActions()
{
	auto set = ui->entriesWidget->propertySet();

	bool canDeleteAll = set != nullptr && set->hasChildProperties();
	bool canDelete = ui->entriesWidget->getActiveEntryProperty() != nullptr;

	ui->actionDeleteAll->setEnabled(canDeleteAll);
	ui->deleteAllButton->setEnabled(canDeleteAll);
	ui->actionDeleteAction->setEnabled(canDelete);
	ui->deleteActionButton->setEnabled(canDelete);
}

void ScriptManagerDialog::on_cancelButton_clicked()
{
	reject();
}

void ScriptManagerDialog::on_okButton_clicked()
{
	accept();
}

void ScriptManagerDialog::on_newActionButton_clicked()
{
	ui->actionNewAction->trigger();
}

void ScriptManagerDialog::on_deleteActionButton_clicked()
{
	ui->actionDeleteAction->trigger();
}

void ScriptManagerDialog::on_deleteAllButton_clicked()
{
	ui->actionDeleteAll->trigger();
}

void ScriptManagerDialog::on_entriesWidget_customContextMenuRequested(
	const QPoint &pos)
{
	auto active = ui->entriesWidget->getActiveEntryProperty();

	if (active)
	{
		QMenu menu(this);

		menu.addAction(ui->actionNewAction);
		menu.addAction(ui->actionDeleteAction);

		mPopup = true;
		menu.exec(ui->entriesWidget->mapToGlobal(pos));
		mPopup = false;
	}
}
