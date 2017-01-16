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

#include "BananaCore/BaseFileManager.h"

class QProgressDialog;

namespace Banana
{
	class AbstractFileSystemObject;

class PasteFileManager : public QObject, public Banana::BaseFileManager
{
	Q_OBJECT

public:
	PasteFileManager(QWidget *parent_widget = nullptr);

protected:
	virtual bool wasCanceled() const override;
	virtual int shouldReplaceFile(const QFileInfo &info) override;
	virtual int error(int action, int what, const QFileInfo &info) override;

	virtual void processStarted() override;
	virtual void processFinished() override;
	virtual void processAborted() override;
	virtual void processFileStarted(const QDir &pasteDir, const QFileInfo &file_src) override;
	virtual void processFileFinished(const QFileInfo &file_src, const QFileInfo &file_dst) override;
	virtual void processFileAborted(const QDir &pasteDir, const QFileInfo &file_src) override;

private:
	void endFileProcess(Banana::AbstractFileSystemObject *fsys);

	QWidget *parent_widget;
	QProgressDialog *dialog;
	int replace_button;
};
}
