#pragma once

#include <QDialog>
#include <QStringList>

namespace Ui
{
	class AddFilesToIgnoreListDialog;
}

class QAbstractButton;

namespace Banana
{
	class AbstractProjectFile;

class AddFilesToIgnoreListDialog : public QDialog
{
	Q_OBJECT

public:
	explicit AddFilesToIgnoreListDialog(AbstractProjectFile *file,
										const QString &path,
										QWidget *parent = nullptr);
	virtual ~AddFilesToIgnoreListDialog();

	static bool execute(const QStringList &pathList,
						AbstractProjectFile *file,
						QWidget *parent);

public Q_SLOTS:
	virtual void accept() override;

private slots:
	void onExactEditingFinished();

	void on_radioButtonExact_toggled(bool checked);

	void on_radioButtonExtension_toggled(bool checked);

	void on_radioButtonBeneath_toggled(bool checked);

	void on_radioButtonPattern_toggled(bool checked);

	void on_buttonBox_clicked(QAbstractButton *button);

private:
	void setPath(const QString &path);

	enum
	{
		EXACT,
		EXTENSION,
		BENEATH,
		PATTERN
	};

	Ui::AddFilesToIgnoreListDialog *ui;
	int ignore_what;
	AbstractProjectFile *project_file;
};

}
