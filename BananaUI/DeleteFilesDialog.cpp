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

#include "DeleteFilesDialog.h"

#include "ProjectDirectoryModel.h"
#include "BananaCore/AbstractProjectDirectory.h"
#include "BananaCore/AbstractFile.h"
#include "BananaCore/Utils.h"

#include <QDir>
#include <QMessageBox>

#include <functional>

using namespace Banana;
#include "ui_DeleteFilesDialog.h"

namespace Banana
{

DeleteFilesDialog::DeleteFilesDialog(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::DeleteFilesDialog)
	, list_model(nullptr)
{
	ui->setupUi(this);

	setWindowFlags((windowFlags() & ~(Qt::WindowContextHelpButtonHint))
				   | Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint);
}

DeleteFilesDialog::~DeleteFilesDialog()
{
	delete ui;
}

bool DeleteFilesDialog::execute(const QModelIndexList &to_delete, ProjectDirectoryModel *source_model)
{
	ui->listView->setModel(nullptr);
	delete list_model;
	list_model = new DeleteFilesListModel(source_model, to_delete, this);
	ui->listView->setModel(list_model);

	show();
	raise();
	exec();

	return result() == Accepted;
}

void DeleteFilesDialog::accept()
{
	if (list_model->deleteCheckedEntries())
		QDialog::accept();
}

DeleteFilesListModel::DeleteFilesListModel(ProjectDirectoryModel *source_model,
										   const QModelIndexList &to_delete,
										   QObject *parent)
	: QAbstractListModel(parent)
	, source_model(source_model)
{
	for (auto &index : to_delete)
		addIndexToDelete(index);

	finalizeEntries();
}

int DeleteFilesListModel::rowCount(const QModelIndex &) const
{
	return static_cast<int>(to_delete.size());
}

Qt::ItemFlags DeleteFilesListModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;

	return QAbstractListModel::flags(index) | Qt::ItemIsUserCheckable;
}

QVariant DeleteFilesListModel::data(const QModelIndex &index, int role) const
{
	if (index.isValid())
	{
		switch (role)
		{
			case Qt::DecorationRole:
				return to_delete.at(index.row()).icon;

			case Qt::ToolTipRole:
			case Qt::DisplayRole:
				return to_delete.at(index.row()).file_info.filePath();

			case Qt::CheckStateRole:
				return QVariant(to_delete.at(index.row()).checked ? Qt::Checked : Qt::Unchecked);

			default:
				break;
		}
	}

	return QVariant();
}

bool DeleteFilesListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (index.isValid() && role == Qt::CheckStateRole)
	{
		auto state = (Qt::CheckState) value.toInt();
		switch (state)
		{
			case Qt::Checked:
			case Qt::Unchecked:
			{
				auto &entry = to_delete[index.row()];
				entry.checked = (state == Qt::Checked);

				if (!entry.checked)
				{
					auto entry_ptr = &entry;

					do
					{
						int parent_index = entry_ptr->parent_index;
						if (parent_index < 0)
							break;

						entry_ptr = &to_delete[parent_index];

						if (entry_ptr->checked)
						{
							entry_ptr->checked = false;

							auto i = this->index(parent_index);
							emit dataChanged(i, i);
						}
					} while (true);
				}

				if (entry.file_info.isDir() && !entry.file_info.isSymLink())
				{
					QString pfx(QDir::cleanPath(entry.file_info.filePath()) + "/");
					if (!entry.checked)
					{
						for (auto it = to_delete.begin(); it != to_delete.end(); ++it)
						{
							if (&*it != &entry
							&&	QDir::cleanPath(it->file_info.path()).startsWith(pfx, Qt::CaseInsensitive))
							{
								if (!it->checked)
									return true;
							}
						}
					}

					int start_index = -1;
					int end_index = -1;

					for (auto it = to_delete.begin(); it != to_delete.end(); ++it)
					{
						if (&*it != &entry
						&&	QDir::cleanPath(it->file_info.filePath()).startsWith(pfx, Qt::CaseInsensitive))
						{
							end_index = static_cast<int>(it - to_delete.begin());
							if (start_index < 0)
								start_index = end_index;

							it->checked = entry.checked;
						} else
						if (start_index >= 0)
						{
							emit dataChanged(this->index(start_index), this->index(end_index));

							start_index = -1;
						}
					}

					if (start_index >= 0)
						emit dataChanged(this->index(start_index), this->index(end_index));
				}
				return true;
			}

			default:
				break;
		}
	}

	return false;
}

bool DeleteFilesListModel::deleteCheckedEntries()
{
	beginResetModel();

	int deletedCount = 0;
	int checkedCount = 0;

	auto projectDir = source_model->getProjectDirectory();

	QString path;

	for (auto it = to_delete.begin(); it != to_delete.end(); )
	{
		bool deleted = false;

		if (it->checked)
		{
			auto &fileInfo = it->file_info;
			if (!fileInfo.isDir() || fileInfo.isSymLink())
			{
				checkedCount++;
				path = fileInfo.filePath();

				tryDeleteFileSysObjectFrom(projectDir, path);

				auto index = source_model->index(path);
				if (index.isValid())
				{
					if (!fileInfo.isSymLink() && source_model->remove(index))
						deleted = true;
				}

				if (!deleted)
				{
					deleted = Utils::DeleteFileOrLink(fileInfo);
				}
			}
		}

		if (deleted)
		{
			it = to_delete.erase(it);
			deletedCount++;
		} else
			++it;
	}

	for (auto it = to_delete.rbegin(); it != to_delete.rend();)
	{
		bool deleted = false;

		if (it->checked)
		{
			auto &file_info = it->file_info;
			if (file_info.isDir() && !file_info.isSymLink())
			{
				checkedCount++;
				path = file_info.filePath();

				tryDeleteFileSysObjectFrom(projectDir, path);

				auto index = source_model->index(path);
				if (index.isValid())
				{
					if (source_model->rmdir(index))
						deleted = true;
				}

				if (!deleted)
				{
					if (!file_info.exists() || QDir().rmdir(path))
						deleted = true;
				}
			}
		}

		if (deleted)
		{
			it = std::vector<EntryToDelete>::reverse_iterator(to_delete.erase(it.base() - 1));
			deletedCount++;
		} else
			++it;
	}

	endResetModel();

	if (0 == checkedCount)
	{
		QMessageBox::warning(nullptr, QCoreApplication::applicationName(), tr("Nothing checked!"));
		return false;
	}

	EntryToDelete *file_entry_ptr = nullptr;
	EntryToDelete *link_entry_ptr = nullptr;
	EntryToDelete *dir_entry_ptr = nullptr;
	int files_count = 0;
	int dirs_count = 0;
	int links_count = 0;

	for (auto &entry : to_delete)
	{
		if (entry.checked)
		{
			if (entry.file_info.isSymLink())
			{
				link_entry_ptr = &entry;
				links_count++;
			} else
			if (entry.file_info.isDir())
			{
				dir_entry_ptr = &entry;
				dirs_count++;
			} else
			{
				file_entry_ptr = &entry;
				files_count++;
			}
		}
	}

	if (file_entry_ptr == nullptr && link_entry_ptr == nullptr && dir_entry_ptr == nullptr)
		return true;

	QString message;
	QString message_fmt1 = tr("Unable to delete %1 '%2'!");
	QString message_fmt2 = tr("Unable to delete %1 '%2' and some other files and/or directories!");

	if (nullptr != file_entry_ptr)
	{
		QString path(file_entry_ptr->file_info.filePath());
		QString file_str = tr("file", "Unable to delete");
		if (files_count == 1 && links_count == 0 && dirs_count == 0)
			message = message_fmt1.arg(file_str, path);
		else
			message = message_fmt2.arg(file_str, path);
	} else
	if (nullptr != link_entry_ptr)
	{
		QString path(link_entry_ptr->file_info.filePath());
		QString link_str = tr("link", "Unable to delete");
		if (files_count == 0 && links_count == 1 && dirs_count == 0)
			message = message_fmt1.arg(link_str, path);
		else
			message = message_fmt2.arg(link_str, path);
	} else
	if (nullptr != dir_entry_ptr)
	{
		QString path(dir_entry_ptr->file_info.filePath());
		QString dir_str = tr("directory", "Unable to delete");
		if (files_count == 0 && links_count == 0 && dirs_count == 1)
			message = message_fmt1.arg(dir_str, path);
		else
			message = message_fmt2.arg(dir_str, path);
	}

	QMessageBox::critical(nullptr, QCoreApplication::applicationName(), message);

	return false;
}

void DeleteFilesListModel::addIndexToDelete(const QModelIndex &index)
{
	if (index.isValid())
	{
		auto fileInfo = source_model->fileInfo(index);
		fileInfo.refresh();
		addEntryToDelete(fileInfo, source_model->fileIcon(index));
	}
}

void DeleteFilesListModel::addEntryToDelete(const QFileInfo &entry, const QIcon &icon)
{
	using namespace std::placeholders;

	auto it = std::find_if(to_delete.begin(), to_delete.end(),
						   std::bind(&DeleteFilesListModel::checkEntryIsAdded, entry, _1));

	if (it == to_delete.end())
	{
		to_delete.push_back({ entry, icon, -1, true });

		if (entry.isDir() && !entry.isSymLink())
		{
			auto entries = QDir(entry.filePath()).entryInfoList(QDir::Dirs | QDir::Files |
										 QDir::Readable | QDir::Writable | QDir::Executable |
										 QDir::Modified | QDir::Hidden | QDir::System |
										 QDir::NoDotAndDotDot);

			for (auto &entry : entries)
				addEntryToDelete(entry, source_model->icon(entry));
		}
	}
}

void DeleteFilesListModel::finalizeEntries()
{
	using namespace std::placeholders;

	std::sort(to_delete.begin(), to_delete.end(), &DeleteFilesListModel::entryLessThan);

	for (auto &entry : to_delete)
	{
		QFileInfo entryParentInfo(entry.file_info.path());

		auto it = std::find_if(to_delete.begin(), to_delete.end(),
							   std::bind(&DeleteFilesListModel::checkEntryIsAdded, std::cref(entryParentInfo), _1));

		if (it != to_delete.end())
			entry.parent_index = static_cast<int>(it - to_delete.begin());
	}
}

bool DeleteFilesListModel::tryDeleteFileSysObjectFrom(AbstractProjectDirectory *project_dir, const QString &path)
{
	while (true)
	{
		auto obj = project_dir->findFileSystemObject(path, true);

		if (nullptr == obj)
			return true;

		auto file = dynamic_cast<AbstractFile *>(obj);
		if (nullptr != file)
		{
			if (!canDeleteFile(file))
				break;
		} else
		{
			if (!canDeleteDir(obj->getObject()))
				break;
		}

		delete obj;
	}

	return false;
}

bool DeleteFilesListModel::canDeleteFile(AbstractFile *file)
{
	return !file->isBound();
}

bool DeleteFilesListModel::canDeleteDir(QObject *dir)
{
	for (auto child : dir->children())
	{
		auto file = dynamic_cast<AbstractFile *>(child);

		if (nullptr != file)
		{
			if (!canDeleteFile(file))
				return false;
		} else
		{
			if (!canDeleteDir(child))
				return false;
		}
	}

	return true;
}

bool DeleteFilesListModel::entryLessThan(const EntryToDelete &a, const EntryToDelete &b)
{
	return QString::compare(a.file_info.filePath(), b.file_info.filePath(), Qt::CaseInsensitive) < 0;
}

bool DeleteFilesListModel::checkEntryIsAdded(const QFileInfo &entry, const EntryToDelete &check)
{
	return (QString::compare(entry.filePath(),
							 check.file_info.filePath(),
							 Qt::CaseInsensitive) == 0);
}

void DeleteFilesDialog::on_deleteButton_clicked()
{
	accept();
}

void DeleteFilesDialog::on_closeButton_clicked()
{
	reject();
}
}
