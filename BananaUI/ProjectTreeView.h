#pragma once

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

class ProjectTreeView : public QTreeView
{
	Q_OBJECT

public:
	explicit ProjectTreeView(QWidget *parent = nullptr);

	void select(Banana::AbstractFileSystemObject *file, bool expand = false);
	void select(const QString &filepath, bool expand = false);

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
	void setProjectDirectory(Banana::AbstractProjectDirectory *dir);
	void setFileTypeFilter(const QMetaObject *meta_object);

	void setReadOnly(bool yes);
	virtual void setModel(QAbstractItemModel *model) override;
	Banana::ProjectDirectoryModel *getProjectTreeModel() const;
	Banana::ProjectDirectoryFilterModel *getFilterModel() const;

signals:
	void modelSelectionChanged();

private slots:
	void onFilterModelAboutToBeReset();
	void onFilterModelReset();

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
