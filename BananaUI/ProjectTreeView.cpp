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

#include "ProjectTreeView.h"

#include "ProjectDirectoryModel.h"
#include "ProjectDirectoryFilterModel.h"
#include "DeleteFilesDialog.h"

#include "BananaCore/AbstractProjectDirectory.h"
#include "BananaCore/Utils.h"
#include "BananaCore/BaseFileManager.h"

#include <QClipboard>
#include <QMimeData>
#include <QApplication>
#include <QList>
#include <QUrl>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>

namespace Banana
{
static const QString kCutSelFormat("application/x-kde-cutselection");

ProjectTreeView::ProjectTreeView(QWidget *parent)
	: QTreeView(parent)
	, projectDirModel(nullptr)
	, filterModel(new ProjectDirectoryFilterModel(this))
{
	QObject::connect(filterModel, &QAbstractItemModel::modelAboutToBeReset,
		this, &ProjectTreeView::onFilterModelAboutToBeReset);
	QObject::connect(filterModel, &QAbstractItemModel::modelReset, this,
		&ProjectTreeView::onFilterModelReset);
}

void ProjectTreeView::select(AbstractFileSystemObject *file, bool expand)
{
	if (nullptr != file)
		select(file->getFilePath(), expand);
}

void ProjectTreeView::select(const QString &filePath, bool expand)
{
	if (filePath.isEmpty())
		return;

	AbstractProjectDirectory *project_dir =
		projectDirModel->getProjectDirectory();

	QFileInfo fileInfo;

	if (nullptr != project_dir)
		fileInfo.setFile(project_dir->getAbsoluteFilePathFor(filePath, true));
	else
		fileInfo.setFile(filePath);

	if (!fileInfo.exists() || fileInfo.isRelative())
		return;

	QModelIndexList to_expand;
	QModelIndex fileIndex;
	auto pathInfo = fileInfo;

	do
	{
		fileIndex = projectDirModel->index(pathInfo.path());

		if (!fileIndex.isValid())
			break;

		to_expand.push_back(fileIndex);

		if (fileIndex == this->rootIndex())
			break;

		if (pathInfo.isRoot())
			break;

		pathInfo.setFile(pathInfo.path());
	} while (true);

	for (auto it = to_expand.rbegin(); it != to_expand.rend(); ++it)
	{
		while (projectDirModel->canFetchMore(*it))
		{
			projectDirModel->fetchMore(*it);
		}

		this->expand(filterModel->mapFromSource(*it));
	}

	fileIndex = projectDirModel->index(fileInfo.filePath());
	auto index = filterModel->mapFromSource(fileIndex);

	if (index.isValid())
	{
		clearSelection();

		if (expand)
			this->expand(index);

		setCurrentIndex(index);

		std::shared_ptr<QMetaObject::Connection> c(new QMetaObject::Connection);
		*c.get() = QObject::connect(filterModel,
			&QAbstractItemModel::layoutChanged, [this, fileInfo, c]() mutable {
				QObject::disconnect(*c.get());
				c = nullptr;
				auto index = projectDirModel->index(fileInfo.filePath());
				index = filterModel->mapFromSource(index);
				scrollTo(index, QTreeView::PositionAtCenter);
			});
	}
}

QModelIndexList ProjectTreeView::getSelectedFilesIndexList() const
{
	auto sel_model = selectionModel();

	if (nullptr != sel_model)
		return filterModel->mapSelectionToSource(sel_model->selection())
			.indexes();

	return QModelIndexList();
}

QStringList ProjectTreeView::getSelectedFilesList(bool relative) const
{
	QStringList result;

	if (nullptr != projectDirModel)
	{
		auto indexes = getSelectedFilesIndexList();

		auto project_dir = projectDirModel->getProjectDirectory();

		for (auto &index : indexes)
		{
			if (index.isValid())
			{
				auto info = projectDirModel->fileInfo(index);

				if (relative && nullptr != project_dir)
				{
					result.push_back(QDir::toNativeSeparators(
						project_dir->getRelativeFilePathFor(info.filePath())));
				} else
				{
					result.push_back(QDir::toNativeSeparators(info.filePath()));
				}
			}
		}
	}

	return result;
}

void ProjectTreeView::copyFilePaths()
{
	auto indexes = getSelectedFilesIndexList();

	if (!indexes.empty())
	{
		auto mime = new QMimeData;

		QStringList list;

		for (auto &index : indexes)
		{
			auto info = projectDirModel->fileInfo(index);

			if (info.isFile())
				list.push_back(QDir::toNativeSeparators(info.filePath()));
		}

		mime->setText(list.join('\n'));

		QApplication::clipboard()->setMimeData(mime);
	}
}

void ProjectTreeView::copyDirPaths()
{
	auto indexes = getSelectedFilesIndexList();

	if (!indexes.empty())
	{
		auto mime = new QMimeData;

		QSet<QString> set;

		for (auto &index : indexes)
		{
			auto info = projectDirModel->fileInfo(index);

			if (info.isFile())
				set.insert(QDir::toNativeSeparators(info.path()));
			else if (info.isDir())
				set.insert(QDir::toNativeSeparators(info.filePath()));
		}

		mime->setText(set.toList().join('\n'));

		QApplication::clipboard()->setMimeData(mime);
	}
}

void ProjectTreeView::copyFileNames()
{
	auto indexes = getSelectedFilesIndexList();

	if (!indexes.empty())
	{
		auto mime = new QMimeData;

		QSet<QString> set;

		for (auto &index : indexes)
		{
			auto info = projectDirModel->fileInfo(index);

			if (info.isFile())
				set.insert(info.fileName());
		}

		mime->setText(set.toList().join('\n'));

		QApplication::clipboard()->setMimeData(mime);
	}
}

void ProjectTreeView::copyDirNames()
{
	auto indexes = getSelectedFilesIndexList();

	if (!indexes.empty())
	{
		auto mime = new QMimeData;

		QSet<QString> set;

		for (auto &index : indexes)
		{
			auto info = projectDirModel->fileInfo(index);

			if (info.isFile())
				set.insert(QFileInfo(info.path()).fileName());
			else if (info.isDir())
				set.insert(info.fileName());
		}

		mime->setText(set.toList().join('\n'));

		QApplication::clipboard()->setMimeData(mime);
	}
}

void ProjectTreeView::cutToClipboard()
{
	copyToClipboard(true);
}

void ProjectTreeView::copyToClipboard(bool cut)
{
	auto sel_model = selectionModel();

	if (nullptr != sel_model)
	{
		auto mime = new QMimeData;
		QList<QUrl> urls;

		for (auto &index : sel_model->selectedIndexes())
		{
			urls.push_back(QUrl::fromLocalFile(
				projectDirModel->filePath(filterModel->mapToSource(index))));
		}

		mime->setUrls(urls);
		auto uri_list = mime->data("text/uri-list");
		mime->setData("text/plain", uri_list);
		mime->setData("x-special/gnome-copied-files",
			QByteArray(cut ? "cut\n" : "copy\n").append(uri_list));

		if (cut)
			mime->setData(kCutSelFormat, "1");

		QApplication::clipboard()->setMimeData(mime);
	}
}

void ProjectTreeView::pasteFromClipboard()
{
	auto sel_model = selectionModel();

	if (nullptr != sel_model)
	{
		auto mime = QApplication::clipboard()->mimeData();

		if (nullptr != mime && mime->hasUrls())
		{
			auto findex = getCurrentFileIndex();
			QDir pasteDir;

			if (findex.isValid())
			{
				auto info = projectDirModel->fileInfo(findex);

				if (!info.isDir())
					pasteDir = QDir(info.path());
				else
					pasteDir = QDir(info.filePath());
			} else
				pasteDir =
					QDir(projectDirModel->getProjectDirectory()->getFilePath());

			if (!pasteDir.exists())
				return;

			bool cut = false;

			if (mime->hasFormat(kCutSelFormat))
			{
				bool ok = false;
				cut =
					(0 != QString(mime->data(kCutSelFormat)).toInt(&ok) || !ok);
			}

			if (projectDirModel->getFileManager()->processUrls(
					cut ? Qt::MoveAction : Qt::CopyAction, pasteDir,
					mime->urls()))
			{
				if (cut)
					QApplication::clipboard()->clear();
			}
		}
	}
}

QModelIndex ProjectTreeView::getCurrentFileIndex() const
{
	return filterModel->mapToSource(getCurrentFilterIndex());
}

QModelIndex ProjectTreeView::getCurrentFilterIndex() const
{
	auto sel_model = selectionModel();

	if (nullptr != sel_model)
	{
		if (sel_model->hasSelection())
			return currentIndex();
	}

	return QModelIndex();
}

void ProjectTreeView::setProjectDirectory(AbstractProjectDirectory *dir)
{
	QTreeView::setModel(nullptr);

	if (nullptr != projectDirModel)
	{
		auto root_index = projectDirModel->setProjectDirectory(dir);

		if (nullptr != dir)
		{
			QTreeView::setModel(filterModel);
			setRootIndex(filterModel->mapFromSource(root_index));

			QObject::connect(selectionModel(),
				&QItemSelectionModel::selectionChanged, this,
				&ProjectTreeView::modelSelectionChanged);
		}
	}
}

void ProjectTreeView::setFileTypeFilter(const QMetaObject *meta_object)
{
	if (nullptr != projectDirModel)
	{
		QTreeView::setModel(nullptr);
		filterModel->setFileTypeFilter(meta_object);

		setProjectDirectory(projectDirModel->getProjectDirectory());
	}
}

void ProjectTreeView::setReadOnly(bool yes)
{
	Q_ASSERT(nullptr != filterModel);
	filterModel->setReadOnly(yes);
}

void ProjectTreeView::setModel(QAbstractItemModel *model)
{
	if (projectDirModel != model)
	{
		QTreeView::setModel(nullptr);
		projectDirModel = dynamic_cast<ProjectDirectoryModel *>(model);

		filterModel->setSourceModel(projectDirModel);

		sortByColumn(0, Qt::AscendingOrder);

		onFilterModelReset();
	}
}

ProjectDirectoryModel *ProjectTreeView::getProjectTreeModel() const
{
	return projectDirModel;
}

ProjectDirectoryFilterModel *ProjectTreeView::getFilterModel() const
{
	return filterModel;
}

void ProjectTreeView::onFilterModelAboutToBeReset()
{
	savedCurrent.clear();

	if (projectDirModel)
	{
		auto index = getCurrentFileIndex();

		if (index.isValid())
			savedCurrent = projectDirModel->fileInfo(index).filePath();
	}

	expanded.clear();

	if (nullptr != projectDirModel)
		saveExpandedDirs(rootIndex());
}

void ProjectTreeView::onFilterModelReset()
{
	AbstractProjectDirectory *project_dir = nullptr;

	if (nullptr != projectDirModel)
		project_dir = projectDirModel->getProjectDirectory();

	setProjectDirectory(project_dir);

	if (nullptr != projectDirModel)
	{
		for (auto &path : expanded)
		{
			auto index = projectDirModel->index(path);

			if (index.isValid())
			{
				index = filterModel->mapFromSource(index);

				if (index.isValid())
					expand(index);
			}
		}
	}

	expanded.clear();

	if (!savedCurrent.isEmpty())
	{
		select(savedCurrent, false);
		savedCurrent.clear();
	}
}

void ProjectTreeView::saveExpandedDirs(const QModelIndex &parent_index)
{
	int count = filterModel->rowCount(parent_index);

	for (int i = 0; i < count; i++)
	{
		auto index = filterModel->index(i, 0, parent_index);

		if (index.isValid() && isExpanded(index))
		{
			index = filterModel->mapToSource(index);

			if (index.isValid())
			{
				auto info = projectDirModel->fileInfo(index);
				expanded.push_back(info.filePath());
			}
		}
	}

	for (int i = 0; i < count; i++)
	{
		auto index = filterModel->index(i, 0, parent_index);

		if (index.isValid() && isExpanded(index))
			saveExpandedDirs(index);
	}
}

void ProjectTreeView::dragEnterEvent(QDragEnterEvent *event)
{
	QTreeView::dragEnterEvent(event);

	convertDropAction(event);
	event->accept();
}

void ProjectTreeView::dragMoveEvent(QDragMoveEvent *event)
{
	QTreeView::dragMoveEvent(event);

	convertDropAction(event);
	event->accept();
}

void ProjectTreeView::dropEvent(QDropEvent *event)
{
	convertDropAction(event);

	QTreeView::dropEvent(event);
}

void ProjectTreeView::convertDropAction(QDropEvent *event)
{
	auto model = getProjectTreeModel();
	Q_ASSERT(nullptr != model);

	event->setDropAction(
		model->convertDropAction(event->mimeData(), event->proposedAction()));
}
}
