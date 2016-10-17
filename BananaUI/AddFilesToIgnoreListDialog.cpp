/*
 * MIT License
 *
 * Copyright (c) 2016 Alexandra Cherdantseva
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "AddFilesToIgnoreListDialog.h"

#include "BananaCore/AbstractProjectFile.h"
#include "BananaCore/Directory.h"

#include <QFileInfo>
#include <QAbstractButton>
#include <QMessageBox>

using namespace Banana;
#include "ui_AddFilesToIgnoreListDialog.h"

namespace Banana
{

AddFilesToIgnoreListDialog::AddFilesToIgnoreListDialog(Banana::AbstractProjectFile *file, const QString &path, QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::AddFilesToIgnoreListDialog)
	, ignore_what(EXACT)
	, project_file(file)
{
	ui->setupUi(this);

	setWindowFlags((windowFlags() & ~(Qt::WindowContextHelpButtonHint
				|	Qt::WindowMinMaxButtonsHint))
				|	Qt::MSWindowsFixedSizeDialogHint
				|	Qt::CustomizeWindowHint);

	setPath(path);

	QObject::connect(ui->lineEditExact, &QLineEdit::editingFinished,
					 this, &AddFilesToIgnoreListDialog::onExactEditingFinished);
}

AddFilesToIgnoreListDialog::~AddFilesToIgnoreListDialog()
{
	delete ui;
}

bool AddFilesToIgnoreListDialog::execute(const QStringList &pathList, AbstractProjectFile *file, QWidget *parent)
{
	if (pathList.count() == 1)
	{
		AddFilesToIgnoreListDialog dlg(file, pathList.at(0), parent);

		dlg.show();
		dlg.raise();
		dlg.exec();

		return dlg.result() == Accepted;
	}

	auto project_dir = file->getTopDirectory();

	auto ignored = file->getIgnoredFilesList();

	for (auto &path : pathList)
		ignored.push_back(project_dir->getRelativeFilePathFor(path));

	file->setIgnoredFilesList(ignored);

	return true;
}

void AddFilesToIgnoreListDialog::accept()
{
	QString pattern;

	switch (ignore_what)
	{
		case EXACT:
			pattern = ui->lineEditExact->text();
			break;

		case EXTENSION:
			pattern = ui->lineEditExtension->text();
			if (!pattern.isEmpty())
				pattern = "*" + pattern;
			break;

		case BENEATH:
			pattern = ui->comboBoxPath->currentText();
			if (!pattern.isEmpty())
				pattern = pattern + "/*";
			break;

		case PATTERN:
			pattern = ui->lineEditPattern->text();
			break;
	}

	if (pattern.isEmpty())
	{
		QMessageBox::critical(this, QCoreApplication::applicationName(), tr("Empty pattern!"));
	} else
	{
		auto ignored = project_file->getIgnoredFilesList();

		ignored.push_back(pattern);

		project_file->setIgnoredFilesList(ignored);

		QDialog::accept();
	}
}

void AddFilesToIgnoreListDialog::onExactEditingFinished()
{
	setPath(ui->lineEditExact->text());
}

void AddFilesToIgnoreListDialog::setPath(const QString &path)
{
	auto project_dir = project_file->getTopDirectory();

	QFileInfo info(project_dir->getRelativeFilePathFor(path));

	ui->lineEditExact->setText(info.filePath());
	auto extension = info.suffix();
	if (!extension.isEmpty())
		extension = "." + extension;
	ui->lineEditExtension->setText(extension);

	QStringList items;

	do
	{
		auto p = info.path();
		if (p.isEmpty() || p == "." || p == "..")
			break;

		items.prepend(p);

		info.setFile(p);
	} while (true);

	ui->comboBoxPath->clear();
	ui->comboBoxPath->insertItems(0, items);
	ui->comboBoxPath->setCurrentIndex(items.count() - 1);
}

void AddFilesToIgnoreListDialog::on_radioButtonExact_toggled(bool checked)
{
	if (checked)
		ignore_what = EXACT;
}

void AddFilesToIgnoreListDialog::on_radioButtonExtension_toggled(bool checked)
{
	if (checked)
		ignore_what = EXTENSION;
}

void AddFilesToIgnoreListDialog::on_radioButtonBeneath_toggled(bool checked)
{
	if (checked)
		ignore_what = BENEATH;
}

void AddFilesToIgnoreListDialog::on_radioButtonPattern_toggled(bool checked)
{
	if (checked)
		ignore_what = PATTERN;
}

void AddFilesToIgnoreListDialog::on_buttonBox_clicked(QAbstractButton *button)
{
	switch (ui->buttonBox->buttonRole(button))
	{
		case QDialogButtonBox::AcceptRole:
			accept();
			break;

		case QDialogButtonBox::RejectRole:
			reject();
			break;

		default:
			break;
	}
}
}
