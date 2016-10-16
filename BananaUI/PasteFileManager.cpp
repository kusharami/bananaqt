#include "PasteFileManager.h"

#include "BananaCore/AbstractProjectDirectory.h"
#include "BananaCore/AbstractFile.h"

#include <QProgressDialog>
#include <QMessageBox>
#include <QApplication>
#include <QDesktopWidget>
#include <QPushButton>
#include <QLabel>
#include <QDir>
#include <QStyle>

namespace Banana
{

PasteFileManager::PasteFileManager(QWidget *parent_widget)
	: QObject(parent_widget)
	, parent_widget(parent_widget)
	, dialog(nullptr)
	, replace_button(0)
{
}

bool PasteFileManager::wasCanceled() const
{
	return (nullptr == dialog || dialog->wasCanceled());
}

int PasteFileManager::shouldReplaceFile(const QFileInfo &info)
{
	int result = 0;

	QMessageBox::StandardButton button;
	if (replace_button == 0)
	{
		button = QMessageBox::question(parent_widget, QApplication::applicationName(),
									  tr("File '%1' is already exists. Do you want to replace it?\n"
										 "If no, the enumerated name will be given.")
									  .arg(info.filePath()),
									   QMessageBox::Yes | QMessageBox::No |
									   QMessageBox::YesToAll | QMessageBox::NoToAll |
									   QMessageBox::Abort,
									   QMessageBox::Yes);
	} else
		button = (QMessageBox::StandardButton) replace_button;

	switch (button)
	{
		case QMessageBox::YesToAll:
			replace_button = QMessageBox::YesToAll;
			result |= ANSWER_ALL;
		case QMessageBox::Yes:
			result |= ANSWER_AGREE;
			break;

		case QMessageBox::NoToAll:
			replace_button = QMessageBox::NoToAll;
			result |= ANSWER_ALL;
		case QMessageBox::No:
			break;

		case QMessageBox::Abort:
			result |= ANSWER_ABORT;
			break;

		default:
			break;
	}

	return result;
}

int PasteFileManager::error(int action, int what, const QFileInfo &info)
{
	QString message_fmt;

	switch (action)
	{
		case DELETE:
			switch (what)
			{
				case FILE:
					message_fmt = tr("Unable to delete file '%1'!");
					break;

				case SYMLINK:
					message_fmt = tr("Unable to delete symbolic link '%1'!");
					break;

				case DIRECTORY:
					message_fmt = tr("Unable to delete directory '%1'!");
					break;
			}

			break;

		case CREATE:
			switch (what)
			{
				case FILE:
					message_fmt = tr("Unable to write file '%1'!");
					break;

				case SYMLINK:
					message_fmt = tr("Unable to create symbolic link '%1'!");
					break;

				case DIRECTORY:
					message_fmt = tr("Unable to create directory '%1'!");
					break;
			}

		case COPY_TO_ITSELF:
			switch (what)
			{
				case FILE:
				case SYMLINK:
					return 0;

				case DIRECTORY:
					message_fmt = tr("Unable to copy/move directory '%1' to its descendant directory or to itself!");
					break;
			}
			break;
	}

	if (QMessageBox::Abort ==
		QMessageBox::critical(parent_widget, QApplication::applicationName(),
							  message_fmt.arg(info.filePath()),
							  QMessageBox::Ignore | QMessageBox::Abort, QMessageBox::Ignore))
		return ANSWER_ABORT;

	return 0;
}

void PasteFileManager::processStarted()
{
	if (nullptr != dialog)
		delete dialog;

	dialog = new QProgressDialog(parent_widget);

	class Label : public QLabel
	{
	public:
		Label()
		{
			this->setWordWrap(true);
		}

	protected:
		virtual QSize sizeHint() const override
		{
			return QSize(500, 60);
		}
	};

	auto label = new Label;

	dialog->setMaximum(100);
	dialog->setWindowModality(Qt::WindowModal);
	dialog->setLabel(label);
	dialog->setCancelButton(new QPushButton(tr("Abort"), dialog));
	dialog->setAutoClose(true);
	dialog->setValue(0);

	dialog->show();
	dialog->raise();

	QApplication::processEvents();
}

void PasteFileManager::processFinished()
{
	dialog->setValue(100);

	QApplication::processEvents();

	delete dialog;
	dialog = nullptr;
}

void PasteFileManager::processAborted()
{
	QApplication::processEvents();

	delete dialog;
	dialog = nullptr;
}

void PasteFileManager::processFileStarted(const QDir &pasteDir, const QFileInfo &file_src)
{
	auto file = dynamic_cast<AbstractFile *>(project_dir->findFileSystemObject(file_src.filePath(), false));
	if (nullptr != file)
		file->unwatchFile();

	switch (action)
	{
		case Qt::MoveAction:
			dialog->setLabelText(tr("Move '%1' to '%2'").arg(file_src.filePath(), pasteDir.path()));
			break;

		case Qt::CopyAction:
			dialog->setLabelText(tr("Copy '%1' to '%2'").arg(file_src.filePath(), pasteDir.path()));
			break;

		case Qt::LinkAction:
			dialog->setLabelText(tr("Make link to '%1' in '%2'").arg(file_src.filePath(), pasteDir.path()));
			break;

		default:
			break;
	}

	QApplication::processEvents();
}

void PasteFileManager::processFileFinished(const QFileInfo &file_src, const QFileInfo &file_dst)
{
	auto fsys = project_dir->findFileSystemObject(file_src.filePath(), false);

	do
	{
		QFileInfo info(file_src.filePath());
		if (!info.exists() && !info.isSymLink()
		&&	0 != QString::compare(file_src.filePath(), file_dst.filePath(), Qt::CaseInsensitive))
		{
			auto fsys2 = project_dir->findFileSystemObject(file_dst.filePath(), false);

			if (nullptr != fsys2)
			{
				if (nullptr != dynamic_cast<AbstractFile *>(fsys2))
					break;
			} else
			{
				fsys2 = project_dir->findFileSystemObject(file_dst.filePath(), true);

				if (fsys2 != fsys)
				{
					auto file = dynamic_cast<AbstractFile *>(fsys2);
					if (nullptr != file)
					{
						file->setModified(true);
						break;
					}
				}

				fsys2 = nullptr;
			}
			if (nullptr != fsys)
			{
				auto file = dynamic_cast<AbstractFile *>(fsys);

				if (nullptr != file)
				{
					auto old_data = file->getData(false);
					QMetaObject::Connection connection;

					if (nullptr != old_data)
					{
						connection = QObject::connect(old_data, &QObject::destroyed, [&old_data]() mutable
						{
							old_data = nullptr;
						});
					}

					file->setLoadError(true);

					auto result = project_dir->initFileSystemObject(file, file_dst.filePath());

					Q_ASSERT(nullptr != result);

					file->setLoadError(false);

					if (nullptr != old_data)
					{
						auto new_data = file->getData(false);

						if (new_data != old_data && nullptr != new_data)
						{
							auto object = dynamic_cast<Object *>(old_data);
							if (nullptr != object)
								object->setModified(true);
						}

						QObject::disconnect(connection);
					}
				} else
				{
					auto dir = dynamic_cast<Directory *>(fsys);

					Q_ASSERT(nullptr != dir);

					auto new_dir = dynamic_cast<Directory *>(fsys2);

					if (nullptr != new_dir)
					{
						std::set<AbstractFile *> files;
						Directory::getFilesIn(dir, files);

						for (auto file : files)
						{
							file->setLoadError(true);

							if (file->parent() == dir)
								file->setParent(new_dir);

							file->updateFilePath();
							file->setLoadError(false);
						}

						delete dir;
						fsys = new_dir;
					} else
					{
						new_dir = new Directory(file_dst.fileName());
						auto result = project_dir->initFileSystemObject(new_dir, file_dst.filePath());
						Q_ASSERT(nullptr != result);
					}
				}
			}
		}
	} while (false);

	int value = dialog->value() + 1;
	if (value > dialog->maximum())
		value = 0;
	dialog->setValue(value);

	endFileProcess(fsys);
}

void PasteFileManager::processFileAborted(const QDir &, const QFileInfo &file_src)
{
	endFileProcess(project_dir->findFileSystemObject(file_src.filePath(), false));
}

void PasteFileManager::endFileProcess(AbstractFileSystemObject *fsys)
{
	auto file = dynamic_cast<AbstractFile *>(fsys);
	if (nullptr != file)
	{
		file->watchFile();
	}

	QApplication::processEvents();
}
}
