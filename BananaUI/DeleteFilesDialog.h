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
