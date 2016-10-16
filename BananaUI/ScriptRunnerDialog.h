#pragma once

#include <QDialog>

namespace Ui
{
	class ScriptRunnerDialog;
}

class QAbstractButton;

namespace Banana
{
	class ProjectGroup;
	class ScriptRunner;

class ScriptRunnerDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ScriptRunnerDialog(Banana::ScriptRunner *runner,
								QWidget *parent = nullptr);
	virtual ~ScriptRunnerDialog();

	void execute(ProjectGroup *group, const QString &script_filepath);

private slots:
	void onLogPrint(const QString &text);

	void on_buttonBrowse_clicked();

	void on_buttonBox_clicked(QAbstractButton *button);

	void on_btnInsertFilePath_clicked();

	void on_btnInsertDirectoryPath_clicked();

	void on_btnInsertProjectItem_clicked();

signals:
	void registerFilePath(const QString &filepath);

protected:
	virtual void closeEvent(QCloseEvent *event) override;

private:
	void insertList(const QStringList &value);
	void apply();
	void setScriptFilePath(const QString &filepath);
	void registerScript();

	Ui::ScriptRunnerDialog *ui;

	QString lastFilePath;
	ProjectGroup *group;
	ScriptRunner *runner;
};
}
