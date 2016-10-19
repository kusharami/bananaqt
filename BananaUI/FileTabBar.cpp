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

#include "FileTabBar.h"

#include "BananaCore/AbstractFile.h"
#include "BananaCore/AbstractProjectDirectory.h"
#include "BananaCore/ProjectGroup.h"
#include "BananaCore/UndoStack.h"

#include <QUndoGroup>

namespace Banana
{
	FileTabBar::FileTabBar(QWidget *parent)
		: QTabBar(parent)
	{
		QObject::connect(this, &QTabBar::tabCloseRequested,
						 this, &FileTabBar::onTabCloseRequested);

		QObject::connect(this, &QTabBar::tabMoved,
						 this, &FileTabBar::onTabMoved);

		setDocumentMode(true);
		setTabsClosable(true);
		setUsesScrollButtons(true);
		setMovable(true);
	}

	void FileTabBar::setCanOpen(const CanOpenFunc &canOpen)
	{
		this->canOpen = canOpen;
	}

	void FileTabBar::setCanClose(const CanCloseFunc &canClose)
	{
		this->canClose = canClose;
	}

	AbstractFile *FileTabBar::getFileAtIndex(int index) const
	{
		if (index >= 0 && index < static_cast<int>(openedFiles.size()))
			return openedFiles.at(index);

		return nullptr;
	}

	AbstractFile *FileTabBar::getCurrentFile() const
	{
		return getFileAtIndex(currentIndex());
	}

	UndoStack *FileTabBar::getCurrentUndoStack() const
	{
		return getFileUndoStack(getCurrentFile());
	}

	UndoStack *FileTabBar::getFileUndoStack(AbstractFile *file)
	{
		if (nullptr != file)
		{
			auto dataObject = dynamic_cast<Object *>(file->getData(false));
			Q_ASSERT(nullptr != dataObject);

			return dataObject->getUndoStack();
		}

		return nullptr;
	}

	bool FileTabBar::fileOpen(AbstractFile *file)
	{
		Q_ASSERT(nullptr != canOpen);

		if (canOpen(file))
		{
			int index = getFileIndex(file, true);
			if (index < 0)
			{
				file->bind();
				if (!file->open())
				{
					file->unbind(false);
					emit fileOpenError(file);
					return false;
				}

				openedFiles.push_back(file);

				int newIndex = addTab(getTitleForFile(file));
				setCurrentIndex(newIndex);
				setTabToolTip(newIndex, getToolTipForFile(file));

				connectFile(file);

				auto projectDir = dynamic_cast<AbstractProjectDirectory *>(file->getTopDirectory());
				Q_ASSERT(nullptr != projectDir);
				auto projectGroup = projectDir->getProjectGroup();
				Q_ASSERT(nullptr != projectGroup);
				auto undoGroup = projectGroup->getUndoGroup();
				if (nullptr != undoGroup)
				{
					auto fileData = dynamic_cast<Object *>(file->getData(false));
					Q_ASSERT(nullptr != fileData);
					auto undoStack = fileData->getUndoStack();
					if (nullptr != undoStack)
						undoGroup->addStack(undoStack);
				}

				emit fileOpened(file);
			} else
				setCurrentIndex(index);

			return true;
		}

		return false;
	}

	void FileTabBar::fileClose(AbstractFile *file, bool multiple)
	{
		int index = getFileIndex(file);

		if (index >= 0)
		{
			closeTab(index, multiple);
		}
	}

	bool FileTabBar::isFileOpen(AbstractFile *file) const
	{
		return getFileIndex(file) >= 0;
	}

	void FileTabBar::onTabCloseRequested(int index)
	{
		closeTab(index, false);
	}

	void FileTabBar::closeTab(int index, bool multiple)
	{
		Q_ASSERT(index >= 0 && index < static_cast<int>(openedFiles.size()));
		Q_ASSERT(nullptr != canClose);

		auto file = openedFiles.at(index);

		if (canClose(file, multiple))
		{
			file->unbind(false);
			if (file->isOpen())
				doFileClose(file);
		}
	}

	void FileTabBar::onTabMoved(int from, int to)
	{
		auto data = openedFiles.data();
		std::swap(data[from], data[to]);
	}

	void FileTabBar::onFileClosed()
	{
		doFileClose(dynamic_cast<AbstractFile *>(sender()));
	}

	void FileTabBar::onFileDestroyed(QObject *object)
	{
		int index = getFileIndex(object, false);

		if (index >= 0)
		{
			openedFiles.erase(openedFiles.begin() + index);

			removeTab(index);

			emit fileClosed(static_cast<AbstractFile *>(object));
		}
	}

	void FileTabBar::onFileFlagsChanged()
	{
		updateTabText();

		emit fileFlagsChanged(dynamic_cast<AbstractFile *>(sender()));
	}

	void FileTabBar::updateTabText()
	{
		auto file = dynamic_cast<AbstractFile *>(sender());
		int index = getFileIndex(file);

		if (index >= 0)
		{
			setTabText(index, getTitleForFile(file));
			setTabToolTip(index, getToolTipForFile(file));
		}
	}

	QString FileTabBar::getFileIsModifiedFmt()
	{
		return tr("%1[*]", "File is modified");
	}

	void FileTabBar::doFileClose(AbstractFile *file)
	{
		int index = getFileIndex(file);

		if (index >= 0)
		{
			openedFiles.erase(openedFiles.begin() + index);
			removeTab(index);

			disconnectFile(file);
			emit fileClosed(file);
		}
	}

	QString FileTabBar::getTitleForFile(AbstractFile *file)
	{
		Q_ASSERT(nullptr != file);

		QString result(file->getFileName());

		if (file->isModified())
			result = getFileIsModifiedFmt().arg(result);

		return result;
	}

	QString FileTabBar::getToolTipForFile(AbstractFile *file)
	{
		Q_ASSERT(nullptr != file);

		QString result(file->getFilePath());

		if (file->isModified())
			result = getFileIsModifiedFmt().arg(result);

		return result;
	}

	void FileTabBar::connectFile(AbstractFile *file)
	{
		if (nullptr != file)
		{
			QObject::connect(file, &QObject::destroyed,
							 this, &FileTabBar::onFileDestroyed);

			QObject::connect(file, &AbstractFile::flagsChanged,
							 this, &FileTabBar::onFileFlagsChanged);

			QObject::connect(file, &AbstractFile::pathChanged,
							 this, &FileTabBar::updateTabText);

			QObject::connect(file, &AbstractFile::fileClosed,
							 this, &FileTabBar::onFileClosed);
		}
	}

	void FileTabBar::disconnectFile(AbstractFile *file)
	{
		if (nullptr != file)
		{
			QObject::disconnect(file, &QObject::destroyed,
								this, &FileTabBar::onFileDestroyed);

			QObject::disconnect(file, &AbstractFile::flagsChanged,
								this, &FileTabBar::onFileFlagsChanged);

			QObject::disconnect(file, &AbstractFile::pathChanged,
								this, &FileTabBar::updateTabText);

			QObject::disconnect(file, &AbstractFile::fileClosed,
								this, &FileTabBar::onFileClosed);
		}
	}

	int FileTabBar::getFileIndex(QObject *fileObject, bool valid) const
	{
		auto file = valid
				? dynamic_cast<AbstractFile *>(fileObject)
				: static_cast<AbstractFile *>(fileObject);
		if (nullptr != file)
		{
			auto it = std::find(openedFiles.begin(), openedFiles.end(), file);

			if (valid && openedFiles.end() == it)
			{
				QString filePath(file->getFilePath());
				it = std::find_if(openedFiles.begin(), openedFiles.end(),
				[&filePath](AbstractFile *a) -> bool
				{
					return (0 == filePath.compare(a->getFilePath(), Qt::CaseInsensitive));
				});
			}

			if (openedFiles.end() != it)
				return it - openedFiles.begin();
		}

		return -1;
	}
}
