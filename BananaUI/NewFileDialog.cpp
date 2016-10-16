#include "NewFileDialog.h"

#include "BananaCore/AbstractProjectDirectory.h"
#include "BananaCore/Utils.h"
#include "BananaCore/AbstractFile.h"
#include "Config.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QCoreApplication>
#include <QSettings>
#include <QAbstractButton>

using namespace Banana;
#include "ui_NewFileDialog.h"

namespace Banana
{

NewFileDialog::NewFileDialog(AbstractProjectDirectory *project_dir, const QString &path, QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::NewFileDialog)
	, project_dir(project_dir)
	, result_file(nullptr)
{
	Q_ASSERT(nullptr != project_dir);

	ui->setupUi(this);

	setWindowFlags((windowFlags() & ~(Qt::WindowContextHelpButtonHint | Qt::WindowMinMaxButtonsHint))
				|	Qt::MSWindowsFixedSizeDialogHint
				|	Qt::CustomizeWindowHint);

	ui->editCreateIn->setText(project_dir->getAbsoluteFilePathFor(path));

	auto extensions = project_dir->getFileExtensions(true);

	for (auto ext : extensions)
	{
		file_types.push_back({ QCoreApplication::translate("FileFormatName", ext), ext });
	}

	std::sort(file_types.begin(), file_types.end(), [](const FileTypeInfo &a, const FileTypeInfo &b) -> bool
	{
		return QString::compare(a.name, b.name, Qt::CaseInsensitive) < 0;
	});

	auto extension = getLastNewFileExtension();

	auto it = std::find_if(file_types.begin(), file_types.end(), [&extension](const FileTypeInfo &a) -> bool
	{
		return 0 == QString::compare(a.extension, extension, Qt::CaseInsensitive);
	});

	for (auto &type : file_types)
	{
		ui->comboBoxFileType->addItem(type.name);
	}

	ui->comboBoxFileType->setCurrentIndex(it == file_types.end() ? 0 : static_cast<int>(it - file_types.begin()));
}

NewFileDialog::~NewFileDialog()
{
	delete ui;
}

AbstractFile *NewFileDialog::getResultFile() const
{
	return result_file;
}

bool NewFileDialog::execute()
{
	result_file = nullptr;

	show();
	raise();
	exec();

	return result() == Accepted;
}

void NewFileDialog::accept()
{
	if (validateInputName())
	{
		auto result_file_path = getResultFilePath();

		auto result = project_dir->newFile(result_file_path);

		if (nullptr != result)
		{
			result_file = result;
			QDialog::accept();

			setLastNewFileExtension(getFileTypeInfo().extension);
		}
	}
}

void NewFileDialog::on_buttonBox_clicked(QAbstractButton *button)
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

void NewFileDialog::on_buttonBrowse_clicked()
{
	auto path = QFileDialog::getExistingDirectory(this, tr("Select New File Directory"),
												  ui->editCreateIn->text(),
												  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (!path.isEmpty())
		ui->editCreateIn->setText(path);
}

void NewFileDialog::on_editName_textChanged(const QString &text)
{
	if (text.trimmed().isEmpty())
		ui->editName->setText("");
}

QString NewFileDialog::getInputName() const
{
	return ui->editName->text().trimmed();
}

bool NewFileDialog::validateInputName()
{
	auto actual_name = getInputName();

	if (actual_name.isEmpty())
	{
		QMessageBox::critical(this, QCoreApplication::applicationName(),
						tr("%1 name cannot be empty!").arg(getFileTypeInfo().name));
		return false;
	}

	auto valid_name = Utils::ConvertToFileName(actual_name);
	if (0 != QString::compare(actual_name, valid_name, Qt::CaseSensitive))
	{
		QMessageBox::critical(this, QCoreApplication::applicationName(),
							  tr("'%2' is invalid %1 name. Try '%3'.")
							  .arg(getFileTypeInfo().name, actual_name, valid_name));

		return false;
	}

	return true;
}

const QMetaObject *NewFileDialog::getFileType() const
{
	return Directory::getFileTypeByExtensionPtr(getFileTypeInfo().extension);
}

const NewFileDialog::FileTypeInfo &NewFileDialog::getFileTypeInfo() const
{
	return file_types.at(ui->comboBoxFileType->currentIndex());
}

QString NewFileDialog::getResultFilePath()
{
	QDir dir(ui->editCreateIn->text());
	return QDir::cleanPath(dir.absoluteFilePath(getInputName() + getFileTypeInfo().extension));

}
}
