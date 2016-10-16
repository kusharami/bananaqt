#pragma once

#include "VariantMapFile.h"
#include "NamingPolicy.h"

#include "PropertyDef.h"

#include <set>

namespace Core
{
	class SearchPaths;
	class Directory;
	class OpenedFiles;
	class AbstractProjectFile : public VariantMapFile
	{
		Q_OBJECT

		Q_PROPERTY(Core::SearchPaths *mSearchPaths
				   READ getSearchPaths
				   RESET resetSearchPaths
				   DESIGNABLE true
				   STORED false)

	public:
		CUSTOM_PROPERTY(bool, HideIgnoredFiles)
		CUSTOM_REF_PROPERTY_EX(QString, IgnoredFilesPattern, STORED false)

		explicit AbstractProjectFile(const QString &name, const QString &extension);
		virtual ~AbstractProjectFile();

		virtual bool isWatched() const override;

		virtual void unwatchFile() override;
		virtual void watchFile() override;

		QStringList getIgnoredFilesList() const;
		void setIgnoredFilesList(const QStringList &value);

		SearchPaths *getSearchPaths();
		Q_INVOKABLE void resetSearchPaths();

		QString getAbsoluteFilePathFor(const QString &filepath) const;

	signals:
		void changedHideIgnoredFiles();
		void changedIgnoredFilesPattern();

	protected:
		virtual void saveData(QVariantMap &output) override;
		virtual bool loadData(const QVariantMap &input) override;

		virtual void doUpdateFilePath(bool check_oldpath) override;

		static const QString SEARCH_PATHS_KEY;
		static const QString IGNORED_FILES_KEY;
		static const QString FILES_KEY;
		static const QString TYPE_KEY;
		static const QString PATH_KEY;
		static const QString TARGET_KEY;
		static const QString TYPE_FILE;
		static const QString TYPE_DIR;
		static const QString TYPE_DIR_LINK;
		static const QString TYPE_FILE_LINK;

	private:
		enum class FileObjType
		{
			None,
			Directory,
			DirectoryLink,
			File,
			FileLink
		};

		static FileObjType getFileObjTypeFromString(const QString &str);

		void watch(bool yes);
		void saveProjectDirectory(Directory *root_dir,
								  Directory *dir,
								  QVariantList &output) const;

		OpenedFiles *openedFiles;
		SearchPaths *searchPaths;
	};

}

Q_DECLARE_METATYPE(Core::AbstractProjectFile *)
