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

#pragma once

#include "BananaUI/ISelectionDelegate.h"

#include <QTreeView>

class QFileInfo;
class QDir;
class QProgressDialog;

namespace Banana
{
class AbstractProjectDirectory;
class AbstractFileSystemObject;
class ProjectDirectoryModel;
class ProjectDirectoryFilterModel;

class ProjectTreeView
	: public QTreeView
	, public ISelectionDelegate
{
	Q_OBJECT

public:
	explicit ProjectTreeView(QWidget *parent = nullptr);

	void select(AbstractFileSystemObject *file, bool expand = false);
	void select(const QString &filepath, bool expand = false);

	virtual QObjectList getSelectedItems() const override;
	virtual void setSelectedItems(const QObjectList &items) override;

	QModelIndexList getSelectedFilesIndexList() const;
	QStringList getSelectedFilesList(bool relative) const;

	void copyFilePaths();
	void copyDirPaths();
	void copyFileNames();
	void copyDirNames();

	void cutToClipboard();
	void copyToClipboard(bool cut = false);
	void pasteFromClipboard();

	QModelIndex getCurrentFileIndex() const;
	QModelIndex getCurrentFilterIndex() const;
	void setProjectDirectory(AbstractProjectDirectory *dir);
	void setFileTypeFilter(const QMetaObject *meta_object);

	void setReadOnly(bool yes);
	virtual void setModel(QAbstractItemModel *model) override;
	ProjectDirectoryModel *getProjectTreeModel() const;

	ProjectDirectoryFilterModel *getFilterModel() const;
	void setFilterModel(ProjectDirectoryFilterModel *model);

signals:
	void modelSelectionChanged();

private:
	void onFilterModelAboutToBeReset();
	void onFilterModelReset();
	bool expandIfFilter();

protected:
	virtual void dragEnterEvent(QDragEnterEvent *event) override;
	virtual void dragMoveEvent(QDragMoveEvent *event) override;
	virtual void dropEvent(QDropEvent *event) override;

private:
	void convertDropAction(QDropEvent *event);

private:
	void saveExpandedDirs(const QModelIndex &parent_index);

	QString savedCurrent;
	QStringList expanded;
	Banana::ProjectDirectoryModel *projectDirModel;
	Banana::ProjectDirectoryFilterModel *filterModel;
};
}
