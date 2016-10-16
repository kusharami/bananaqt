#pragma once

#include <QString>

class QDir;
class QFileInfo;

namespace Core
{

	struct AbstractNamingPolicy
	{
		virtual ~AbstractNamingPolicy();

		virtual QString getFixedName(const QString &source) const = 0;
		virtual QString getNumberSeparator() const = 0;

		QString getNameWithoutNumber(const QString &name, quint32 *number_ptr = nullptr) const;
	};

	class IdentifierNamingPolicy : public AbstractNamingPolicy
	{
	public:
		virtual QString getFixedName(const QString &source) const override;
		virtual QString getNumberSeparator() const override;
	};

	class FileNamingPolicy : public AbstractNamingPolicy
	{
	public:
		virtual QString getFixedName(const QString &source) const override;
		virtual QString getNumberSeparator() const override;

		QString getUniqueFilePath(const QDir &parent_dir, const QString &base_name, QString suffix) const;

		static QString uniqueFilePath(const QString &parent_path, const QString &base_name, const QString &suffix = QString());
		static QString uniqueFilePath(const QString &parent_path, const QFileInfo &info);
		static QString uniqueFilePath(const QDir &parent_dir, const QString &base_name, const QString &suffix = QString());
		static QString uniqueFilePath(const QDir &parent_dir, const QFileInfo &info);
	};
}
