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
