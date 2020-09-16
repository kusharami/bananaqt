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

#include "ProjectDirectoryFilterModel.h"

#include "ProjectDirectoryModel.h"
#include "BananaCore/AbstractProjectDirectory.h"

#include <QApplication>
#include <QStyle>

namespace Banana
{
ProjectDirectoryFilterModel::ProjectDirectoryFilterModel(QObject *parent)
	: QSortFilterProxyModel(parent)
	, project_tree_model(nullptr)
	, project_file(nullptr)
	, hideIgnored(true)
	, readOnly(false)
	, showDirsOnly(false)
{
	setDynamicSortFilter(true);
}

void ProjectDirectoryFilterModel::setSourceModel(
	QAbstractItemModel *sourceModel)
{
	if (sourceModel != project_tree_model)
	{
		disconnectSourceModel();
		project_tree_model = nullptr;
		if (nullptr != sourceModel)
		{
			project_tree_model =
				dynamic_cast<ProjectDirectoryModel *>(sourceModel);
			Q_ASSERT(nullptr != project_tree_model);
		}

		connectSourceModel();
		QSortFilterProxyModel::setSourceModel(project_tree_model);

		applyProjectFile();
	}
}

QVariant ProjectDirectoryFilterModel::data(
	const QModelIndex &index, int role) const
{
	if (role == Qt::TextColorRole)
	{
		if (!hideIgnored && ignore.isValid() && !ignore.isEmpty())
		{
			auto source_model =
				static_cast<ProjectDirectoryModel *>(sourceModel());

			if (source_model != nullptr)
			{
				auto project_dir = source_model->getProjectDirectory();
				if (nullptr != project_dir)
				{
					if (ignore.exactMatch(project_dir->getRelativeFilePathFor(
							source_model->fileInfo(mapToSource(index))
								.filePath())))
					{
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
						return QApplication::style()->standardPalette().color(
							QPalette::PlaceholderText);
#else
						return QApplication::style()->standardPalette().color(
							QPalette::Disabled, QPalette::Text);
#endif
					}
				}
			}
		}
	}

	return QSortFilterProxyModel::data(index, role);
}

bool ProjectDirectoryFilterModel::shouldHideIgnored() const
{
	return hideIgnored;
}

void ProjectDirectoryFilterModel::setHideIgnored(bool hide)
{
	if (hideIgnored != hide)
	{
		beginResetModel();

		hideIgnored = hide;

		endResetModel();
	}
}

void ProjectDirectoryFilterModel::setIgnoreFilter(const QRegExp &re)
{
	if (ignore != re)
	{
		beginResetModel();

		ignore = re;

		endResetModel();
	}
}

bool ProjectDirectoryFilterModel::isReadOnly() const
{
	return readOnly;
}

const QRegExp &ProjectDirectoryFilterModel::getIgnoreFilter() const
{
	return ignore;
}

void ProjectDirectoryFilterModel::setFileTypeFilter(
	const QMetaObject *meta_object)
{
	beginResetModel();
	show_extensions.clear();

	if (meta_object == &Directory::staticMetaObject)
	{
		showDirsOnly = true;
	} else
	{
		showDirsOnly = false;
		auto source_model = static_cast<ProjectDirectoryModel *>(sourceModel());

		if (source_model != nullptr)
		{
			auto project_dir = source_model->getProjectDirectory();
			if (nullptr != project_dir)
			{
				show_extensions = Directory::getFileTypeExtensions(meta_object);
			}
		}
	}

	endResetModel();
}

void ProjectDirectoryFilterModel::setFilterRegExp(const QRegExp &re)
{
	beginResetModel();

	QSortFilterProxyModel::setFilterRegExp(re);

	endResetModel();
}

void ProjectDirectoryFilterModel::onSourceModelDestroyed()
{
	disconnectProjectFile();
	QSortFilterProxyModel::setSourceModel(nullptr);
	project_tree_model = nullptr;
}

void ProjectDirectoryFilterModel::applyProjectFile()
{
	disconnectProjectFile();

	project_file = nullptr;
	if (nullptr != project_tree_model)
	{
		auto project_dir = project_tree_model->getProjectDirectory();
		if (nullptr != project_dir)
			project_file = project_dir->getProjectFile();
	}

	connectProjectFile();

	applyFilters();
}

void ProjectDirectoryFilterModel::onProjectFileDestroyed()
{
	project_file = nullptr;
}

void ProjectDirectoryFilterModel::applyFilters()
{
	if (nullptr != project_file)
	{
		setHideIgnored(project_file->getHideIgnoredFiles());

		auto wildcards = project_file->getIgnoredFilesList();
		for (int i = 0, count = wildcards.count(); i < count; i++)
		{
			wildcards[i] = "(.*/)*" +
				wildcards[i]
					.replace('\\', "\\\\")
					.replace('.', "\\.")
					.replace('*', ".*")
					.replace('?', ".");
		}
		QString pattern = "(" + wildcards.join(")|(") + ")";

		setIgnoreFilter(QRegExp(pattern, Qt::CaseInsensitive));
	} else
	{
		setIgnoreFilter(QRegExp());
	}
}

void ProjectDirectoryFilterModel::setReadOnly(bool yes)
{
	readOnly = yes;
}

void ProjectDirectoryFilterModel::showDirectoriesOnly(bool show)
{
	if (showDirsOnly != show)
	{
		beginResetModel();

		showDirsOnly = show;

		endResetModel();
	}
}

bool ProjectDirectoryFilterModel::filterAcceptsColumn(
	int source_column, const QModelIndex &) const
{
	return (0 == source_column);
}

bool ProjectDirectoryFilterModel::filterAcceptsRow(
	int source_row, const QModelIndex &source_parent) const
{
	if (project_tree_model != nullptr)
	{
		auto project_dir = project_tree_model->getProjectDirectory();
		if (nullptr == project_dir)
			return false;

		auto index = project_tree_model->index(source_row, 0, source_parent);

		if (index.isValid())
		{
			auto fileinfo = project_tree_model->fileInfo(index);

			if (fileinfo.isRoot())
				return true;

			auto filePath = QDir::cleanPath(fileinfo.filePath());

			if (fileinfo.isDir() &&
				!filePath.startsWith(
					QDir::cleanPath(project_dir->getFilePath()) + "/",
					Qt::CaseInsensitive))
				return true;

			filePath = project_dir->getRelativeFilePathFor(filePath);

			if (hideIgnored && ignore.isValid() && !ignore.isEmpty())
			{
				if (ignore.exactMatch(filePath))
					return false;
			}

			if (fileinfo.isDir())
				return true;

			auto re = filterRegExp();

			if (!re.isEmpty() && re.isValid())
			{
				if (!re.exactMatch(filePath))
					return false;
			}

			if (showDirsOnly)
			{
				if (!fileinfo.isDir())
					return false;
			}

			if (show_extensions.empty())
				return true;

			for (auto extension : show_extensions)
			{
				if (filePath.endsWith(extension, Qt::CaseInsensitive))
					return true;
			}
		}
	}

	return false;
}

bool ProjectDirectoryFilterModel::lessThan(
	const QModelIndex &source_left, const QModelIndex &source_right) const
{
	if (project_tree_model != nullptr)
	{
		auto left_info = project_tree_model->fileInfo(source_left);
		auto right_info = project_tree_model->fileInfo(source_right);

		if (left_info.isDir() && !right_info.isDir())
			return true;

		if (!left_info.isDir() && right_info.isDir())
			return false;

		auto project_dir = project_tree_model->getProjectDirectory();
		if (nullptr != project_dir)
		{
			if (!hideIgnored && ignore.isValid() && !ignore.isEmpty())
			{
				bool ignore_left = ignore.exactMatch(
					project_dir->getRelativeFilePathFor(left_info.filePath()));
				bool ignore_right = ignore.exactMatch(
					project_dir->getRelativeFilePathFor(right_info.filePath()));

				if (ignore_left && !ignore_right)
					return false;

				if (!ignore_left && ignore_right)
					return true;
			}
		}

		int compare_names = QString::localeAwareCompare(
			left_info.baseName(), right_info.baseName());

		if (compare_names < 0)
			return true;

		int compare_suffixes = QString::localeAwareCompare(
			left_info.completeSuffix(), right_info.completeSuffix());

		if (compare_names == 0 && compare_suffixes < 0)
			return true;

		return false;
	}

	return false;
}

Qt::DropActions ProjectDirectoryFilterModel::supportedDropActions() const
{
	if (readOnly)
		return Qt::CopyAction;

	if (nullptr != project_tree_model)
		return project_tree_model->supportedDropActions();

	return QSortFilterProxyModel::supportedDropActions();
}

Qt::DropActions ProjectDirectoryFilterModel::supportedDragActions() const
{
	if (readOnly)
		return Qt::CopyAction;

	if (nullptr != project_tree_model)
		return project_tree_model->supportedDragActions();

	return QSortFilterProxyModel::supportedDragActions();
}

void ProjectDirectoryFilterModel::connectSourceModel()
{
	if (nullptr != project_tree_model)
	{
		QObject::connect(project_tree_model, &QObject::destroyed, this,
			&ProjectDirectoryFilterModel::onSourceModelDestroyed);
		QObject::connect(project_tree_model,
			&ProjectDirectoryModel::projectDirectoryChanged, this,
			&ProjectDirectoryFilterModel::applyProjectFile);
	}
}

void ProjectDirectoryFilterModel::disconnectSourceModel()
{
	if (nullptr != project_tree_model)
	{
		disconnectProjectFile();

		QObject::disconnect(project_tree_model, &QObject::destroyed, this,
			&ProjectDirectoryFilterModel::onSourceModelDestroyed);
		QObject::disconnect(project_tree_model,
			&ProjectDirectoryModel::projectDirectoryChanged, this,
			&ProjectDirectoryFilterModel::applyProjectFile);

		QSortFilterProxyModel::setSourceModel(nullptr);
		project_tree_model = nullptr;
	}
}

void ProjectDirectoryFilterModel::connectProjectFile()
{
	if (nullptr != project_file)
	{
		QObject::connect(project_file, &QObject::destroyed, this,
			&ProjectDirectoryFilterModel::onProjectFileDestroyed);
		QObject::connect(project_file,
			&AbstractProjectFile::changedHideIgnoredFiles, this,
			&ProjectDirectoryFilterModel::applyFilters);
		QObject::connect(project_file,
			&AbstractProjectFile::changedIgnoredFilesPattern, this,
			&ProjectDirectoryFilterModel::applyFilters);
	}
}

void ProjectDirectoryFilterModel::disconnectProjectFile()
{
	if (nullptr != project_file)
	{
		QObject::disconnect(project_file, &QObject::destroyed, this,
			&ProjectDirectoryFilterModel::onProjectFileDestroyed);
		QObject::disconnect(project_file,
			&AbstractProjectFile::changedHideIgnoredFiles, this,
			&ProjectDirectoryFilterModel::applyFilters);
		QObject::disconnect(project_file,
			&AbstractProjectFile::changedIgnoredFilesPattern, this,
			&ProjectDirectoryFilterModel::applyFilters);

		project_file = nullptr;
	}
}
}
