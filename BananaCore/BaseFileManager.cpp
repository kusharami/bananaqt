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

	bool BaseFileManager::processEntry(Qt::DropAction action, const QDir &pasteDir, const QFileInfo &file_src)
	{
		if (wasCanceled())
			return false;

		QFileInfo file_dst;

		processFileStarted(pasteDir, file_src);

		bool cut = (action == Qt::MoveAction);

		bool result = true;

		if (file_src.exists() || file_src.isSymLink())
		{
			bool abort = false;
			bool unify_path = false;

			file_dst = QFileInfo(pasteDir.filePath(file_src.fileName()));

			bool same_dir = (file_src.dir() == pasteDir);

			if (!same_dir)
			{
				if (file_dst.isDir())
				{
					if (file_src.isFile() || file_src.isSymLink())
						unify_path = true;
				} else
				if (file_dst.isFile() || file_dst.isSymLink())
				{
					int should_replace = shouldReplaceFile(file_dst);

					if (0 != (should_replace & ANSWER_ABORT))
					{
						abort = true;
					} else
					if (0 == (should_replace & ANSWER_AGREE))
					{
						unify_path = true;
					}
				}
			} else
			{
				if (!cut)
					unify_path = true;
			}

			if (!abort)
			{
				if (unify_path)
					file_dst.setFile(FileNamingPolicy::uniqueFilePath(pasteDir, file_src));

				if (!same_dir || file_src != file_dst)
				{
					if (file_src.isSymLink())
					{
						if (prepareTargetFilePath(file_dst)
						&&	Utils::CreateSymLink(file_src.symLinkTarget(), file_dst.filePath()))
						{
							if (cut)
							{
								QFile::remove(file_src.filePath());
								QFileInfo check(file_src.filePath());
								if (check.exists() || check.isSymLink())
								{
									abort = 0 != (error(DELETE, SYMLINK, file_src) & ANSWER_ABORT);
								}
							}
						} else
						{
							abort = 0 != (error(CREATE, SYMLINK, file_dst) & ANSWER_ABORT);
						}
					} else
					if (file_src.isFile())
					{
						bool ok = false;
						if (prepareTargetFilePath(file_dst))
						{
							if (cut)
							{
								ok = QFile::rename(file_src.filePath(), file_dst.filePath());
								if (!ok)
								{
									abort = 0 != (error(DELETE, FILE, file_src) & ANSWER_ABORT);
								}
							}

							if ((!cut || !ok) && !abort)
							{
								if (action == Qt::LinkAction)
								{
									ok = Utils::CreateSymLink(file_src.filePath(), file_dst.filePath());
								} else
									ok = QFile::copy(file_src.filePath(), file_dst.filePath());
							}
						}

						if (!ok && !abort)
						{
							abort = 0 != (error(CREATE, (action == Qt::LinkAction)
														? SYMLINK
														: FILE, file_dst) & ANSWER_ABORT);
						}
					} else
					if (file_src.isDir())
					{
						bool ok = false;
						bool crit = false;
						if (QDir().mkpath(file_dst.path()))
						{
							if ((QDir::cleanPath(QFileInfo(file_dst.path()).canonicalFilePath()) + "/")
									.startsWith(QDir::cleanPath(file_src.canonicalFilePath()) + "/", Qt::CaseInsensitive))
							{
								crit = true;
								abort = 0 != (error(COPY_TO_ITSELF, DIRECTORY, file_src) & ANSWER_ABORT);
							}
						}

						if (!crit && cut
						&&	!file_dst.exists()
						&&	!file_dst.isSymLink())
						{
							ok = QDir().rename(file_src.filePath(), file_dst.filePath());

							if (!ok)
							{
								abort = 0 != (error(DELETE, DIRECTORY, file_src) & ANSWER_ABORT);
							}
						}

						if (!ok && !abort && !crit)
						{
							if (action == Qt::LinkAction)
							{
								ok = Utils::CreateSymLink(file_src.filePath(), file_dst.filePath());
								if (!ok)
								{
									abort = 0 != (error(CREATE, SYMLINK, file_dst) & ANSWER_ABORT);
								}
							} else
							if (!QDir().mkpath(file_dst.filePath()))
							{
								abort = 0 != (error(CREATE, (action == Qt::LinkAction)
													? SYMLINK
													: DIRECTORY, file_dst) & ANSWER_ABORT);
							} else
							{
								QDir src_dir(file_src.filePath());
								QDir dst_dir(file_dst.filePath());

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
										abort = 0 != (error(DELETE, DIRECTORY, file_src) & ANSWER_ABORT);
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
			processFileFinished(file_src, file_dst);
		else
			processFileAborted(pasteDir, file_src);

		return result;
	}

	bool BaseFileManager::prepareTargetFilePath(const QFileInfo &info)
	{
		return (!(info.exists() || info.isSymLink())
				||	((info.isFile() || info.isSymLink()) && QFile::remove(info.filePath())))
				&& QDir().mkpath(info.path());
	}
}
