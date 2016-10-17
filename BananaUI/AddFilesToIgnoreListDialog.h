/*
 * Banana Qt Libraries
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
