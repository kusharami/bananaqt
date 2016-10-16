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
