#pragma once

#include "AbstractFileSystemObject.h"
#include "NamingPolicy.h"
#include "Object.h"

#include <QMetaMethod>

class QIODevice;

namespace Banana
{
	class Directory;
	class AbstractDirectory;
	class AbstractFileRegistrator;
	class OpenedFiles;
	class AbstractFile : public Object, public AbstractFileSystemObject, public FileNamingPolicy
	{
		Q_OBJECT

		Q_PROPERTY(QString fileName READ getFileName WRITE setFileName SCRIPTABLE true STORED false DESIGNABLE false)
		Q_PROPERTY(QString filePath READ getFilePath SCRIPTABLE true STORED false DESIGNABLE false)
		Q_PROPERTY(QString shortFilePath READ getFilePathShort SCRIPTABLE true STORED false DESIGNABLE false)
		Q_PROPERTY(QString canonicalFilePath READ getCanonicalFilePath SCRIPTABLE true STORED false DESIGNABLE false)

	public:
		explicit AbstractFile(const QString &extension);
		virtual ~AbstractFile();

		virtual const QString &getFileExtension() const override;

		Directory *getTopDirectory() const;
		Directory *getParentDirectory() const;

		Q_INVOKABLE QObject *getData(bool open = true);

		template <typename CLASS>
		CLASS *getDataAs(bool open = true)
		{
			auto result = getData(open);

			if (result != nullptr)
				Q_ASSERT(nullptr != dynamic_cast<CLASS *>(result));

			return static_cast<CLASS *>(result);
		}

		Q_INVOKABLE bool isOpen() const;
		Q_INVOKABLE bool canClose();
		Q_INVOKABLE void bind();
		Q_INVOKABLE void unbind(bool stay_open);
		Q_INVOKABLE bool isBound() const;
		Q_INVOKABLE bool save();
		Q_INVOKABLE bool create(bool open = false);
		Q_INVOKABLE bool open();
		Q_INVOKABLE bool reload();
		Q_INVOKABLE void close();
		Q_INVOKABLE virtual bool rename(const QString &new_name) override;

		Q_INVOKABLE bool isWritable() const;

		bool saveTo(QIODevice *device);
		bool loadFrom(QIODevice *device);

		virtual QString getCanonicalFilePath() const override;

		inline unsigned getBindCount() const;

		virtual bool isSymLink() const override;
		virtual QString getSymLinkTarget() const override;

		virtual bool isWatched() const = 0;

		virtual void watchFile() = 0;
		virtual void unwatchFile() = 0;

		void updateFilePath(bool check_oldpath = true);

		void setLoadError(bool value);

		QString getFilePathShort(Directory *topDirectory = nullptr) const;
		Directory *getSearchedDirectory(Directory *topDirectory = nullptr) const;

	signals:
		void flagsChanged();
		void dataChanged();
		void fileOpened();
		void fileClosed();
		void fileReloaded();
		void pathChanged();
		void updateFilePathError(const QString &path, const QString &failed_path);

	private slots:
		void onNameChanged();
		void onDataDestroyed();

	protected:
		friend class AbstractFileRegistrator;
		friend class AbstractDirectory;
		friend class Directory;

		void connectData();
		void disconnectData();

		virtual void changeFilePath(const QString &new_path);
		virtual bool tryChangeFilePath(const QString &new_path);
		virtual void executeUpdateFilePathError(const QString &path, const QString &failed_path) override;
		virtual bool updateFileExtension(const QString &filename, QString *out_ext) override;
		virtual void doUpdateFilePath(bool check_oldpath);
		virtual void doParentChange() override;
		virtual QObject *doGetData();
		virtual void createData(bool *reused = nullptr);
		virtual void destroyData();
		virtual bool doSave(QIODevice *device) = 0;
		virtual bool doLoad(QIODevice *device) = 0;
		virtual void onSave();
		virtual void doFlagsChanged() override;
		virtual void opened();

		QString extension;
		QString canonical_path;
		QString symlink_target;
		unsigned bind_count;
		bool load_error;
		bool symlink;

	private:
		bool saveInternal();
		void disconnectData(QObject *data);
		void internalClose();
		bool is_open;
		bool signals_connected;

		QObject *old_parent;
		QString saved_path;
		QObject *connected_data;
	};

	unsigned AbstractFile::getBindCount() const
	{
		return bind_count;
	}
}

Q_DECLARE_METATYPE(Banana::AbstractFile *)
