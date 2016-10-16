#include "FileSelectDialog.h"

#include "ProjectDirectoryModel.h"
#include "ProjectDirectoryFilterModel.h"

#include "BananaCore/Directory.h"
#include "BananaCore/AbstractFile.h"
#include "BananaCore/AbstractProjectDirectory.h"
#include "BananaCore/Utils.h"

#include "Config.h"
#include "Utils.h"
#include "ProjectTreeView.h"
#include "DeleteFilesDialog.h"

#include <QAbstractButton>
#include <QCoreApplication>
#include <QMessageBox>
#include <QPushButton>
#include <QKeyEvent>
#include <QMenu>
#include <QClipboard>
#include <QMimeData>

using namespace Banana;
#include "ui_FileSelectDialog.h"

namespace Banana
{

FileSelectDialog::FileSelectDialog(ProjectDirectoryModel *model, QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::FileSelectDialog)
	, ignoreAccept(false)
	, dirs(false)
	, popup(false)
{
	ui->setupUi(this);

	setWindowFlags((windowFlags() & ~(Qt::WindowContextHelpButtonHint))
				   | Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint);

	auto tree = ui->treeWidget->getTreeView();
	tree->setModel(model);

	auto caBtn = new QPushButton(tr("Cancel"));
	caBtn->setAutoDefault(true);
	caBtn->setDefault(false);
	ui->buttonBox->addButton(caBtn, QDialogButtonBox::RejectRole);

	auto okBtn = new QPushButton(tr("Select"));
	okBtn->setAutoDefault(true);
	okBtn->setDefault(false);
	ui->buttonBox->addButton(okBtn, QDialogButtonBox::AcceptRole);

	QObject::connect(tree, &QWidget::customContextMenuRequested,
					 this, &FileSelectDialog::onTreeViewCustomContextMenuRequested);
	QObject::connect(tree, &QAbstractItemView::doubleClicked,
					 this, &FileSelectDialog::selectFileAtIndex);
	QObject::connect(tree, &ProjectTreeView::modelSelectionChanged,
					 this, &FileSelectDialog::updateActions);

#ifdef Q_OS_MAC
	Utils::addShortcutForAction(this, QKeySequence(Qt::Key_Backspace), ui->actionEditDelete);
#endif
	Utils::addShortcutForAction(this, QKeySequence::Cut, ui->actionEditCut);
	Utils::addShortcutForAction(this, QKeySequence::Copy, ui->actionEditCopy);
	Utils::addShortcutForAction(this, QKeySequence::Paste, ui->actionEditPaste);

	auto menu = ui->treeWidget->getButtonOptionsMenu();
	QObject::connect(menu, &QMenu::aboutToShow, this, &FileSelectDialog::onContextMenuAboutToShow);
	QObject::connect(menu, &QMenu::aboutToHide, this, &FileSelectDialog::onContextMenuAboutToHide);

	ui->actionShowInGraphicalShell->setToolTip(getShowInGraphicalShellCommandText());
	ui->actionShowInGraphicalShell->setStatusTip(getShowInGraphicalShellHintText());

	updateActions();
}

FileSelectDialog::~FileSelectDialog()
{
	delete ui;
}

QString FileSelectDialog::getSelectedFilePath() const
{
	auto tree = ui->treeWidget->getTreeView();
	auto index = tree->getCurrentFileIndex();

	if (index.isValid())
		return tree->getProjectTreeModel()->filePath(index);

	return QString();
}

QStringList FileSelectDialog::getSelectedFilePathList(bool relative) const
{
	return ui->treeWidget->getTreeView()->getSelectedFilesList(relative);
}

bool FileSelectDialog::execute(const QMetaObject *filter, const QString &file_path, bool multiselect)
{
	if (filter == &Directory::staticMetaObject)
	{
		dirs = true;
		setWindowTitle(multiselect? tr("Select Directories") : tr("Select Directory"));
	} else
	{
		dirs = false;

		if (nullptr != filter)
			setWindowTitle(tr("Select %1").arg(Directory::getFileTypeTitle(filter, multiselect)));
		else
			setWindowTitle(multiselect? tr("Select Files") : tr("Select File"));
	}
	ui->treeWidget->showFilter(!dirs);
	auto tree = ui->treeWidget->getTreeView();

	tree->setSelectionMode(multiselect
						   ? QTreeView::ExtendedSelection
						   : QTreeView::SingleSelection);

	tree->setFileTypeFilter(filter);
	tree->select(file_path);

	show();
	raise();
	exec();

	return result() == Accepted;
}

bool FileSelectDialog::executeMultiSelect(const QMetaObject *filter)
{
	return execute(filter, QString(), true);
}

void FileSelectDialog::accept()
{
	if (popup)
		return;

	QDialog::accept();
}

void FileSelectDialog::reject()
{
	if (popup)
		return;

	QDialog::reject();
}

ProjectDirectoryModel *FileSelectDialog::getModel() const
{
	return ui->treeWidget->getTreeView()->getProjectTreeModel();
}

void FileSelectDialog::onContextMenuAboutToShow()
{
	popup = true;
}

void FileSelectDialog::onContextMenuAboutToHide()
{
	popup = false;
}

void FileSelectDialog::onTreeViewCustomContextMenuRequested(const QPoint &pos)
{
	auto tree = ui->treeWidget->getTreeView();

	QFileInfo info;

	auto index = tree->getCurrentFilterIndex();
	auto project_tree_model = tree->getProjectTreeModel();

	if (index.isValid() && tree->visualRect(index).contains(pos, true))
	{
		index = tree->getCurrentFileIndex();
		if (!index.isValid())
			return;

		info = project_tree_model->fileInfo(index);

		if (info.isDir() || info.isFile())
		{
			updateActions();

			QMenu menu(this);

			menu.addAction(ui->actionOpenFile);
			menu.addAction(ui->actionLocateFile);
			menu.addAction(ui->actionShowInGraphicalShell);
			menu.addSeparator();

			if (!dirs)
			{
				menu.addAction(ui->actionCopyFilePath);
				menu.addAction(ui->actionCopyFileName);
			}
			menu.addAction(ui->actionCopyDirPath);
			menu.addAction(ui->actionCopyDirName);
			menu.addSeparator();

			menu.addAction(ui->actionEditCut);
			menu.addAction(ui->actionEditCopy);
			menu.addAction(ui->actionEditPaste);

			menu.addSeparator();
			menu.addAction(ui->actionEditDelete);

			popup = true;
			menu.exec(tree->mapToGlobal(pos));
			popup = false;
		}
	}
}

void FileSelectDialog::on_buttonBox_clicked(QAbstractButton *button)
{
	switch (ui->buttonBox->buttonRole(button))
	{
		case QDialogButtonBox::AcceptRole:
		{
			if (ignoreAccept)
			{
				ignoreAccept = false;
				return;
			}

			auto tree = ui->treeWidget->getTreeView();
			auto indexes = tree->getSelectedFilesIndexList();
			auto model = tree->getProjectTreeModel();

			for (auto &index : indexes)
			{
				auto info = model->fileInfo(index);
				if ((dirs && info.isDir())
				||	(!dirs && info.isFile()))
				{
					accept();
					return;
				}
			}

			pleaseSelectEntry();

		}	break;

		case QDialogButtonBox::RejectRole:
			reject();
			break;

		default:
			break;
	}
}

void FileSelectDialog::selectFileAtIndex(const QModelIndex &index)
{
	if (!index.isValid())
		return;

	auto tree = ui->treeWidget->getTreeView();
	auto dir_model = tree->getProjectTreeModel();
	auto filter_model = tree->getFilterModel();
	auto info = dir_model->fileInfo(filter_model->mapToSource(index));

	if (info.isDir())
		return;

	accept();
}

void FileSelectDialog::updateActions()
{
	QFileInfo currentFileInfo(getSelectedFilePath());

	auto fileName = currentFileInfo.fileName();

	bool exists = currentFileInfo.exists();

	ui->actionLocateFile->setEnabled(exists);

	if (exists)
	{
		ui->actionLocateFile->setText(tr("Locate '%1' in a Project Tree").arg(fileName));
	} else
	{
		ui->actionLocateFile->setText(tr("Locate in a Project Tree"));
	}

	ui->actionShowInGraphicalShell->setEnabled(exists);
	ui->actionShowInGraphicalShell->setText(getShowInGraphicalShellCommandText(fileName));

	auto model = getModel();

	auto filePaths = getSelectedFilePathList();
	ui->actionOpenFile->setEnabled(nullptr != model && model->canOpenFiles(filePaths));

	if (filePaths.size() > 1)
	{
		ui->actionOpenFile->setText(tr("Open Selected Files"));
		ui->actionEditDelete->setText(tr("Delete Selected..."));
	} else if (filePaths.size() == 1)
	{
		ui->actionOpenFile->setText(tr("Open '%1'").arg(fileName));
		ui->actionEditDelete->setText(tr("Delete '%1'...").arg(fileName));
	} else
	{
		ui->actionOpenFile->setText(tr("Open File"));
		ui->actionEditDelete->setText(tr("Delete..."));
	}

	bool somethingSelected = !filePaths.empty();
	ui->actionEditDelete->setEnabled(somethingSelected);
	ui->actionEditCut->setEnabled(somethingSelected);
	ui->actionEditCopy->setEnabled(somethingSelected);

	auto clipboardData = QApplication::clipboard()->mimeData();
	bool pasteEnabled = false;
	if (nullptr != clipboardData && nullptr != ui->treeWidget->getTreeView()->selectionModel())
		pasteEnabled = clipboardData->hasUrls();

	ui->actionEditPaste->setEnabled(pasteEnabled);
}

void FileSelectDialog::keyPressEvent(QKeyEvent *e)
{
	switch (e->key())
	{
		case Qt::Key_Return:
		case Qt::Key_Enter:
			ignoreAccept = true;
			break;

		default:
			break;
	}

	QDialog::keyPressEvent(e);
}

void FileSelectDialog::pleaseSelectEntry()
{
	QMessageBox::critical(this, QCoreApplication::applicationName(),
						  dirs ? tr("Please select a directory.") : tr("Please select a file."));
}

void FileSelectDialog::on_actionEditDelete_triggered()
{
	auto tree = ui->treeWidget->getTreeView();
	auto model = tree->getProjectTreeModel();
	Q_ASSERT(nullptr != model);

	DeleteFilesDialog dialog(this);

	auto indexes = tree->getSelectedFilesIndexList();
	if (indexes.size() > 0)
		dialog.execute(indexes, model);
}

void FileSelectDialog::on_actionEditCut_triggered()
{
	ui->treeWidget->getTreeView()->cutToClipboard();
}

void FileSelectDialog::on_actionEditCopy_triggered()
{
	ui->treeWidget->getTreeView()->copyToClipboard();
}

void FileSelectDialog::on_actionEditPaste_triggered()
{
	ui->treeWidget->getTreeView()->pasteFromClipboard();
}

void FileSelectDialog::on_actionCopyDirPath_triggered()
{
	ui->treeWidget->getTreeView()->copyDirPaths();
}

void FileSelectDialog::on_actionCopyFilePath_triggered()
{
	ui->treeWidget->getTreeView()->copyFilePaths();
}

void FileSelectDialog::on_actionCopyFileName_triggered()
{
	ui->treeWidget->getTreeView()->copyFileNames();
}

void FileSelectDialog::on_actionCopyDirName_triggered()
{
	ui->treeWidget->getTreeView()->copyDirNames();
}

void FileSelectDialog::on_actionShowInGraphicalShell_triggered()
{
	Utils::ShowInGraphicalShell(getSelectedFilePath());
}

void FileSelectDialog::on_actionOpenFile_triggered()
{
	auto model = getModel();
	if (nullptr != model)
	{
		model->openFiles(getSelectedFilePathList());
	}
}

void FileSelectDialog::on_actionLocateFile_triggered()
{
	auto model = getModel();
	if (nullptr != model)
	{
		model->locateFile(getSelectedFilePath());
	}
}
}
