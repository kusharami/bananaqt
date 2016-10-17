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

#include <QTabBar>
#include <QMetaMethod>

#include <vector>
#include <functional>

namespace Banana
{
	class UndoStack;
	class AbstractFile;

class FileTabBar : public QTabBar
{
	Q_OBJECT

public:
	explicit FileTabBar(QWidget *parent = nullptr);

	typedef std::function<bool (Banana::AbstractFile *)> CanOpenFunc;
	typedef std::function<bool (Banana::AbstractFile *, bool multiple)> CanCloseFunc;

	void setCanOpen(const CanOpenFunc &canOpen);
	void setCanClose(const CanCloseFunc &canClose);

	Banana::AbstractFile *getFileAtIndex(int index) const;
	Banana::AbstractFile *getCurrentFile() const;
	Banana::UndoStack *getCurrentUndoStack() const;

	bool fileOpen(Banana::AbstractFile *file);
	void fileClose(Banana::AbstractFile *file, bool multiple = false);

	bool isFileOpen(Banana::AbstractFile *file) const;

signals:
	void fileOpenError(Banana::AbstractFile *file);
	void fileOpened(Banana::AbstractFile *file);
	void fileFlagsChanged(Banana::AbstractFile *file);
	void fileClosed(Banana::AbstractFile *file);

private slots:
	void onTabCloseRequested(int index);
	void onTabMoved(int from, int to);
	void onFileClosed();
	void onFileDestroyed(QObject *object);
	void onFileFlagsChanged();
	void updateTabText();

private:
	static QString getFileIsModifiedFmt();

	void closeTab(int index, bool multiple);
	void doFileClose(Banana::AbstractFile *file);
	static QString getTitleForFile(Banana::AbstractFile *file);
	static QString getToolTipForFile(Banana::AbstractFile *file);

	void connectFile(Banana::AbstractFile *file);
	void disconnectFile(Banana::AbstractFile *file);

	typedef std::vector<Banana::AbstractFile *> FileVector;

	int getFileIndex(QObject *fileObject, bool valid = false) const;

	CanOpenFunc canOpen;
	CanCloseFunc canClose;
	FileVector openedFiles;
};

}
