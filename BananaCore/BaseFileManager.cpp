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

#include "BaseFileManager.h"

#include "Utils.h"
#include "NamingPolicy.h"

#include <QFileInfo>
#include <QDir>

namespace Banana
{
	BaseFileManager::BaseFileManager()
		: action(Qt::IgnoreAction)
		, project_dir(nullptr)
	{

	}

	bool BaseFileManager::processUrls(Qt::DropAction action, const QDir &pasteDir, const QList<QUrl> &urls)
	{
		QFileInfoList entries;

		for (auto &url : urls)
		{
			if (url.isLocalFile())
				entries.push_back(QFileInfo(url.toLocalFile()));
		}

		return processEntries(action, pasteDir, entries);
	}

	bool BaseFileManager::processEntries(Qt::DropAction action, const QDir &pasteDir, const QFileInfoList &entries)
	{
		if (!entries.isEmpty())
		{
			this->action = action;

			processStarted();

			bool aborted = false;

			for (auto &entry : entries)
			{
				aborted = !processEntry(action, pasteDir, entry);

				if (aborted)
					break;
			}

			if (aborted)
				processAborted();
			else
				processFinished();

			return !aborted;
		}

		return false;
	}

	void BaseFileManager::setProjectDirectory(AbstractProjectDirectory *project_dir)
	{
		this->project_dir = project_dir;
	}

	void BaseFileManager::processStarted()
	{

	}

	void BaseFileManager::processFinished()
	{

	}

	void BaseFileManager::processAborted()
	{

	}

	void BaseFileManager::processFileStarted(const QDir &, const QFileInfo &)
	{

	}

	void BaseFileManager::processFileFinished(const QFileInfo &, const QFileInfo &)
	{

	}

	void BaseFileManager::processFileAborted(const QDir &, const QFileInfo &)
	{

	}

	bool BaseFileManager::wasCanceled() const
	{
		return false;
	}

	int BaseFileManager::shouldReplaceFile(const QFileInfo &)
	{
		return 0;
	}

	int BaseFileManager::error(int, int, const QFileInfo &)
	{
		return 0;
	}

	bool BaseFileManager::processEntry(Qt::DropAction action, const QDir &pasteDir, const QFileInfo &fileSource)
	{
		if (wasCanceled())
			return false;

		QFileInfo fileTarget;

		processFileStarted(pasteDir, fileSource);

		bool cut = (action == Qt::MoveAction);

		bool result = true;

		if (fileSource.exists() || fileSource.isSymLink())
		{
			bool abort = false;
			bool unifyPath = false;

			fileTarget = QFileInfo(pasteDir.filePath(fileSource.fileName()));

			bool sameDir = (fileSource.dir() == pasteDir);

			if (!sameDir)
			{
				if (fileTarget.isDir())
				{
					if (fileSource.isFile() || fileSource.isSymLink())
						unifyPath = true;
				} else
				if (fileTarget.isFile() || fileTarget.isSymLink())
				{
					int shouldReplace = shouldReplaceFile(fileTarget);

					if (0 != (shouldReplace & ANSWER_ABORT))
					{
						abort = true;
					} else
					if (0 == (shouldReplace & ANSWER_AGREE))
					{
						unifyPath = true;
					}
				}
			} else
			{
				if (!cut)
					unifyPath = true;
			}

			if (!abort)
			{
				if (unifyPath)
					fileTarget.setFile(FileNamingPolicy::uniqueFilePath(pasteDir, fileSource));

				if (!sameDir || fileSource != fileTarget)
				{
					if (fileSource.isSymLink())
					{
						if (prepareTargetFilePath(fileTarget)
						&&	Utils::CreateSymLink(fileSource.symLinkTarget(), fileTarget.filePath()))
						{
							if (cut)
							{
								if (!Utils::DeleteFileOrLink(fileSource))
								{
									abort = 0 != (error(DELETE, SYMLINK, fileSource) & ANSWER_ABORT);
								}
							}
						} else
						{
							abort = 0 != (error(CREATE, SYMLINK, fileTarget) & ANSWER_ABORT);
						}
					} else
					if (fileSource.isFile())
					{
						bool ok = false;
						if (prepareTargetFilePath(fileTarget))
						{
							if (cut)
							{
								ok = QFile::rename(fileSource.filePath(), fileTarget.filePath());
								if (!ok)
								{
									abort = 0 != (error(DELETE, FILE, fileSource) & ANSWER_ABORT);
								}
							}

							if ((!cut || !ok) && !abort)
							{
								if (action == Qt::LinkAction)
								{
									ok = Utils::CreateSymLink(fileSource.filePath(), fileTarget.filePath());
								} else
									ok = QFile::copy(fileSource.filePath(), fileTarget.filePath());
							}
						}

						if (!ok && !abort)
						{
							abort = 0 != (error(CREATE, (action == Qt::LinkAction)
														? SYMLINK
														: FILE, fileTarget) & ANSWER_ABORT);
						}
					} else
					if (fileSource.isDir())
					{
						bool ok = false;
						bool crit = false;
						if (QDir().mkpath(fileTarget.path()))
						{
							if ((QDir::cleanPath(QFileInfo(fileTarget.path()).canonicalFilePath()) + "/")
									.startsWith(QDir::cleanPath(fileSource.canonicalFilePath()) + "/", Qt::CaseInsensitive))
							{
								crit = true;
								abort = 0 != (error(COPY_TO_ITSELF, DIRECTORY, fileSource) & ANSWER_ABORT);
							}
						}

						if (!crit && cut
						&&	!fileTarget.exists()
						&&	!fileTarget.isSymLink())
						{
							ok = QDir().rename(fileSource.filePath(), fileTarget.filePath());

							if (!ok)
							{
								abort = 0 != (error(DELETE, DIRECTORY, fileSource) & ANSWER_ABORT);
							}
						}

						if (!ok && !abort && !crit)
						{
							if (action == Qt::LinkAction)
							{
								ok = Utils::CreateSymLink(fileSource.filePath(), fileTarget.filePath());
								if (!ok)
								{
									abort = 0 != (error(CREATE, SYMLINK, fileTarget) & ANSWER_ABORT);
								}
							} else
							if (!QDir().mkpath(fileTarget.filePath()))
							{
								abort = 0 != (error(CREATE, (action == Qt::LinkAction)
													? SYMLINK
													: DIRECTORY, fileTarget) & ANSWER_ABORT);
							} else
							{
								QDir src_dir(fileSource.filePath());
								QDir dst_dir(fileTarget.filePath());

								auto entries = src_dir.entryInfoList(QDir::Dirs |
																	 QDir::Files |
																	 QDir::Readable |
																	 QDir::Writable |
																	 QDir::Executable |
																	 QDir::Modified |
																	 QDir::Hidden |
																	 QDir::System |
																	 QDir::NoDotAndDotDot);
								for (auto &entry : entries)
								{
									if (!processEntry(action, dst_dir, entry))
									{
										abort = true;
										break;
									}
								}

								if (!abort && cut)
								{
									if (!src_dir.removeRecursively())
									{
										abort = 0 != (error(DELETE, DIRECTORY, fileSource) & ANSWER_ABORT);
									}
								}
							}
						}
					}
				}
			}

			if (abort)
				result = false;
		}

		if (result)
			processFileFinished(fileSource, fileTarget);
		else
			processFileAborted(pasteDir, fileSource);

		return result;
	}

	bool BaseFileManager::prepareTargetFilePath(const QFileInfo &info)
	{
		return (Utils::DeleteFileOrLink(info)
			&&	QDir().mkpath(info.path()));
	}
}
