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

#include <functional>

namespace Ui
{
	class FileSelectDialog;
}

class QAbstractButton;

namespace Banana
{
	class ProjectDirectoryModel;

class FileSelectDialog : public QDialog
{
	Q_OBJECT

public:
	explicit FileSelectDialog(Banana::ProjectDirectoryModel *model, QWidget *parent = nullptr);
	virtual ~FileSelectDialog();

	QString getSelectedFilePath() const;
	QStringList getSelectedFilePathList(bool relative = false) const;

	bool execute(const QMetaObject *filter = nullptr,
				 const QString &file_path = QString(),
				 bool multiselect = false);
	bool executeMultiSelect(const QMetaObject *filter = nullptr);

	virtual void accept() override;
	virtual void reject() override;

	Banana::ProjectDirectoryModel *getModel() const;

private slots:
	void onContextMenuAboutToShow();
	void onContextMenuAboutToHide();
	void onTreeViewCustomContextMenuRequested(const QPoint &pos);
	void selectFileAtIndex(const QModelIndex &index);
	void updateActions();

	void on_buttonBox_clicked(QAbstractButton *button);

	void on_actionEditDelete_triggered();

	void on_actionEditCut_triggered();

	void on_actionEditCopy_triggered();

	void on_actionEditPaste_triggered();

	void on_actionCopyDirPath_triggered();

	void on_actionCopyFilePath_triggered();

	void on_actionCopyFileName_triggered();

	void on_actionCopyDirName_triggered();

	void on_actionShowInGraphicalShell_triggered();

	void on_actionOpenFile_triggered();

	void on_actionLocateFile_triggered();

protected:
	virtual void keyPressEvent(QKeyEvent *e) override;

private:
	void pleaseSelectEntry();

	Ui::FileSelectDialog *ui;

	bool ignoreAccept;
	bool dirs;
	bool popup;
};
}
