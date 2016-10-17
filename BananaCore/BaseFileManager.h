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

#pragma once

#include <QObject>
#include <QList>
#include <QUrl>
#include <QFileInfoList>

#ifdef DELETE
#undef DELETE
#endif

class QFileInfo;
class QDir;

namespace Banana
{
	class AbstractProjectDirectory;

	class BaseFileManager
	{
	public:
		enum
		{
			ANSWER_AGREE = 1,
			ANSWER_ALL = 1 << 1,
			ANSWER_ABORT = 1 << 2,

			DELETE = 0,
			CREATE,
			COPY_TO_ITSELF,

			FILE = 0,
			DIRECTORY,
			SYMLINK
		};

		BaseFileManager();
		virtual ~BaseFileManager() {}

		bool processUrls(Qt::DropAction action, const QDir &pasteDir, const QList<QUrl> &urls);
		bool processEntries(Qt::DropAction action, const QDir &pasteDir, const QFileInfoList &entries);

		void setProjectDirectory(AbstractProjectDirectory *project_dir);

	protected:
		virtual void processStarted();
		virtual void processFinished();
		virtual void processAborted();
		virtual void processFileStarted(const QDir &pasteDir, const QFileInfo &file_src);
		virtual void processFileFinished(const QFileInfo &file_src, const QFileInfo &file_dst);
		virtual void processFileAborted(const QDir &pasteDir, const QFileInfo &file_src);

		virtual bool wasCanceled() const;
		virtual int shouldReplaceFile(const QFileInfo &);
		virtual int error(int action, int what, const QFileInfo &info);

		Qt::DropAction action;
		AbstractProjectDirectory *project_dir;

	private:
		bool processEntry(Qt::DropAction action, const QDir &pasteDir, const QFileInfo &file_src);
		static bool prepareTargetFilePath(const QFileInfo &info);
	};

}
