#pragma once

#include <QDialog>
#include <QModelIndexList>
#include <QFileInfo>
#include <QAbstractListModel>
#include <QIcon>

#include <set>

namespace Ui
{
	class DeleteFilesDialog;
}

namespace Banana
{
	class AbstractFile;
	class AbstractProjectDirectory;
	class ProjectDirectoryModel;

class DeleteFilesListModel : public QAbstractListModel
{
	Q_OBJECT

public:
	explicit DeleteFilesListModel(Banana::ProjectDirectoryModel *source_model,
							  const QModelIndexList &to_delete,
							  QObject *parent = nullptr);

	virtual int rowCount(const QModelIndex &) const override;
	virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
	virtual QVariant data(const QModelIndex &index, int role) const override;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;

	bool deleteCheckedEntries();

private:
	struct EntryToDelete
	{
		QFileInfo file_info;
		QIcon icon;
		int parent_index;
		bool checked;
	};

	void addIndexToDelete(const QModelIndex &index);
	void addEntryToDelete(const QFileInfo &entry, const QIcon &icon);

	void finalizeEntries();

	static bool tryDeleteFileSysObjectFrom(Banana::AbstractProjectDirectory *project_dir, const QString &path);
	static bool canDeleteFile(Banana::AbstractFile *file);
	static bool canDeleteDir(QObject *dir);

	static bool entryLessThan(const EntryToDelete &a, const EntryToDelete &b);
	static bool checkEntryIsAdded(const QFileInfo &entry, const EntryToDelete &check);

	std::vector<EntryToDelete> to_delete;
	Banana::ProjectDirectoryModel *source_model;
};

class DeleteFilesDialog : public QDialog
{
	Q_OBJECT

public:
	explicit DeleteFilesDialog(QWidget *parent = nullptr);
	virtual ~DeleteFilesDialog();

	bool execute(const QModelIndexList &to_delete, Banana::ProjectDirectoryModel *source_model);

private slots:
	void on_deleteButton_clicked();

	void on_closeButton_clicked();

public Q_SLOTS:
	virtual void accept() override;

private:
	Ui::DeleteFilesDialog *ui;

	DeleteFilesListModel *list_model;
};
}
