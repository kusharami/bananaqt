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
