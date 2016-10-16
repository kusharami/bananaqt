#pragma once

#include <QDialog>
#include <QAbstractTableModel>

#include <vector>
#include <set>

namespace Ui
{
	class SearchPathsDialog;
}

class QItemSelection;

namespace Banana
{
	class ProjectDirectoryModel;
	class Directory;

class SearchPathsDialog;

class SearchPathsTableModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	explicit SearchPathsTableModel(Banana::ProjectDirectoryModel *project_dir_model,
								  QObject *parent = nullptr);
	virtual ~SearchPathsTableModel();

	virtual int rowCount(const QModelIndex &index = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex &index = QModelIndex()) const override;
	virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
	virtual QVariant data(const QModelIndex &index, int role) const override;

	void registerPaths(const QStringList &paths, int order = -1);
	void unregisterPaths(const QModelIndexList &indexes);

	Banana::Directory *getDirectoryAtIndex(const QModelIndex &index) const;

	virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action,
							  int row, int column, const QModelIndex &parent) override;
	virtual Qt::DropActions supportedDropActions() const override;
	virtual Qt::DropActions supportedDragActions() const override;
	virtual QMimeData *mimeData(const QModelIndexList &indexes) const override;
	virtual QStringList mimeTypes() const override;

	void getSavedSelection(QItemSelection &selection) const;

private slots:
	void onDirectoryDestroyed(QObject *dir);

signals:
	void shouldReselect();

protected:
	virtual bool removeRows(int row, int count, const QModelIndex &parent) override;

private:
	void load();
	void clear();
	void connectDirs();
	void disconnectDirs();
	void connectDirectory(Banana::Directory *dir, int order);
	void disconnectDirectory(Banana::Directory *dir);

	std::vector<Banana::Directory *> dirs;
	Banana::ProjectDirectoryModel *project_dir_model;
	std::vector<Banana::Directory *> dirs_to_select;
	bool internalDrop;
};

class SearchPathsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SearchPathsDialog(Banana::ProjectDirectoryModel *project_dir_model,
							   QWidget *parent = nullptr);
	virtual ~SearchPathsDialog();

	virtual void accept() override;
	virtual void reject() override;

private slots:
	void onProjectDirectoryChanged();
	void onSelectionChanged();
	void applySavedPathSelection();

	void on_registerButton_clicked();

	void on_unregisterButton_clicked();

	void on_closeButton_clicked();

	void on_tableView_customContextMenuRequested(const QPoint &pos);

	void on_actionCopyAbsolutePath_triggered();
	void on_actionCopyDirName_triggered();

	void on_actionUnregisterSelected_triggered();

private:
	std::vector<Banana::Directory *> getSelectedDirs() const;

	Ui::SearchPathsDialog *ui;

	Banana::ProjectDirectoryModel *project_dir_model;
	SearchPathsTableModel *paths_model;
	bool popup;
};
}
