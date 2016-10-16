#pragma once

#include <QDialog>

#include <vector>

namespace Ui
{
	class NewFileDialog;
}

class QAbstractButton;

namespace Banana
{
	class AbstractFile;
	class AbstractProjectDirectory;

class NewFileDialog : public QDialog
{
	Q_OBJECT

public:
	explicit NewFileDialog(Banana::AbstractProjectDirectory *project_dir, const QString &path, QWidget *parent = nullptr);
	virtual ~NewFileDialog();

	Banana::AbstractFile *getResultFile() const;
	bool execute();

public Q_SLOTS:
	virtual void accept() override;

private slots:
	void on_buttonBox_clicked(QAbstractButton *button);

	void on_buttonBrowse_clicked();

	void on_editName_textChanged(const QString &text);

private:
	struct FileTypeInfo
	{
		QString name;
		const char *extension;
	};

	QString getInputName() const;
	bool validateInputName();
	const QMetaObject *getFileType() const;
	const FileTypeInfo &getFileTypeInfo() const;
	QString getResultFilePath();

	Ui::NewFileDialog *ui;
	Banana::AbstractProjectDirectory *project_dir;
	Banana::AbstractFile *result_file;

	std::vector<FileTypeInfo> file_types;
};
}
