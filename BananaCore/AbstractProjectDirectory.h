#pragma once

#include "Directory.h"
#include "AbstractProjectFile.h"

#include <vector>
#include <functional>

namespace Banana
{
	enum class Answer;
	class ProjectGroup;
	class OpenedFiles;
	struct IProjectGroupDelegate;
	class AbstractProjectDirectory : public RootDirectory
	{
		Q_OBJECT
		Q_PROPERTY(AbstractProjectFile *projectFile
				   READ getProjectFile
				   SCRIPTABLE true
				   DESIGNABLE false)

	public:
		explicit AbstractProjectDirectory(const QString &path);

		void setParent(QObject *parent);

		virtual QString getFilePath(const AbstractDirectory *relativeTo = nullptr) const override;
		virtual QString getFileName() const override;
		virtual QString getFixedName(const QString &source) const override;

		std::vector<const char *> getFileExtensions(bool toCreate) const;

		AbstractProjectFile *getProjectFile() const;

		Q_INVOKABLE Banana::AbstractFile *newFile(const QString &path, bool first = true, bool multiple = false);

		Q_INVOKABLE Banana::AbstractFile *addFile(const QString &path,
												bool mustExist,
												bool verbose = true);
		Q_INVOKABLE Banana::AbstractFile *linkFile(const QString &target,
												 const QString &link,
												 bool mustExist,
												 bool verbose = true);

		Q_INVOKABLE Banana::Directory *addSearchPath(const QString &path, int order, bool verbose = true);
		Q_INVOKABLE Banana::Directory *addDirectory(const QString &path,
												bool mustExist,
												bool verbose = true);
		Q_INVOKABLE Banana::Directory *linkDirectory(const QString &target,
												 const QString &link,
												 bool mustExist,
												 bool verbose = true);

		Q_INVOKABLE Banana::AbstractFile *addExistingFile(const QString &path, bool verbose = true);
		Q_INVOKABLE Banana::AbstractFile *linkExistingFile(const QString &target,
														 const QString &link, bool verbose = true);
		Q_INVOKABLE Banana::AbstractFile *findFile(const QString &path);
		Q_INVOKABLE Banana::Directory *findDirectory(const QString &path);

		Q_INVOKABLE bool saveFile(Banana::AbstractFile *file);
		Q_INVOKABLE bool saveFileAs(Banana::AbstractFile *file, const QString &filePath,
									const QString &selectedFilter,
									bool verbose = true,
									bool first = true,
									bool multiple = false);

		template <typename T>
		T *newFileAs(const QString &path)
		{
			auto result = newFile(path);
			if (nullptr != result)
				Q_ASSERT(nullptr != dynamic_cast<T *>(result));

			return static_cast<T *>(result);
		}

		IProjectGroupDelegate *getProjectGroupDelegate() const;
		ProjectGroup *getProjectGroup() const;
		OpenedFiles *getOpenedFiles() const;

		Q_INVOKABLE QObjectList findFiles(const QString &regExp);
		Q_INVOKABLE QObjectList getSearchDirectoryList() const;
		Q_INVOKABLE void clearSearchPaths();

		virtual const QMetaObject *getProjectFileMetaObject() const = 0;

		void saveAllFiles();

		virtual bool canBeCreated(const QMetaObject *metaObject) const = 0;

	signals:
		void changedSearchPaths();

	private slots:
		void onProjectFileModified();
		void onLinkDestroyed();

	protected:
		virtual void descendantChanged(QObject *descendant, DescendantState state) override;
		virtual AbstractProjectFile *createProjectFile();

		AbstractProjectFile *projectFile;

	private:
		bool isFileReplaceAllowed(const QString &absolutePath, Answer *rememberAnswer, const QMetaObject **outFileType);
		static void internalFindFiles(Directory *directory, const QRegExp &regExp, QObjectList &dest);

		bool validateFileSaveAsPath(Banana::AbstractFile *file,
									QString &newFilePath,
									const QString &selectedFilter) const;
		void saveAllFiles(Directory *directory, QList<AbstractFile *> &notSaved);
		void filePathError(Error error, const QString &filePath);
		void errorMessage(const QString &message) const;
		void fillSearchDirListFrom(const Banana::Directory *dir, QObjectList &list) const;
		void clearSearchPaths(Directory *dir);

		static QString stripExtension(const QString &fileName, const QString &extension);

		Answer nfRememberAnswer;
		Answer afRememberAnswer;
	};

}

Q_DECLARE_METATYPE(Banana::AbstractProjectDirectory *)
