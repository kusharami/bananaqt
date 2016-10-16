#include "NamingPolicy.h"

#include "Utils.h"
#include "Const.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>

namespace Core
{

	AbstractNamingPolicy::~AbstractNamingPolicy()
	{

	}

	QString AbstractNamingPolicy::getNameWithoutNumber(const QString &name, quint32 *number_ptr) const
	{
		QString result(name);

		quint32 num = 0;
		int len = result.length();
		if (len > 0 && result.at(len - 1).isDigit())
		{
			int new_len = len;
			for (int j = len - 1; j >= 0; j--)
			{
				bool ok;
				num = QString::fromRawData(&result.constData()[j], len - j).toUInt(&ok);

				if (ok)
					new_len--;
				else
					break;
			}

			if (new_len > 0)
			{
				auto number_separator = getNumberSeparator();
				auto sep_len = number_separator.length();
				auto check_index = new_len - sep_len;
				if (sep_len > 0
				&&	check_index >= 0
				&&	number_separator == QString::fromRawData(&result.constData()[check_index], sep_len))
				{
					new_len = check_index;
				}
			}

			result = QString(result.constData(), new_len);
		}

		if (nullptr != number_ptr)
			*number_ptr = num;

		return result;
	}

	QString IdentifierNamingPolicy::getFixedName(const QString &source) const
	{
		return Utils::ConvertToIdentifierName(source);
	}

	QString IdentifierNamingPolicy::getNumberSeparator() const
	{
		return QString();
	}

	QString FileNamingPolicy::getFixedName(const QString &source) const
	{
		QString result(source);

		if (result.isEmpty())
			return QString(QCoreApplication::translate("FileNamingPolicy", pUntitledFileName));

		return Utils::ConvertToFileName(result);
	}

	QString FileNamingPolicy::getNumberSeparator() const
	{
		return QString(" ");
	}

	QString FileNamingPolicy::getUniqueFilePath(const QDir &parent_dir, const QString &base_name, QString suffix) const
	{
		if (!suffix.isEmpty() && suffix.at(0) != '.')
		{
			suffix = "." + suffix;
		}

		auto current_path = parent_dir.filePath(getFixedName(base_name));

		quint32 number = 0;
		auto without_number = getNameWithoutNumber(current_path, &number);

		QFileInfo info;

		while (true)
		{
			current_path += suffix;

			info.setFile(current_path);

			if (!info.exists() && !info.isSymLink())
				break;

			if (number > 0)
			{
				current_path = without_number + getNumberSeparator() + QString::number(number);
			} else
				current_path = without_number;

			number++;
		}

		return current_path;
	}

	QString FileNamingPolicy::uniqueFilePath(const QString &parent_path, const QString &base_name, const QString &suffix)
	{
		return uniqueFilePath(QDir(parent_path), base_name, suffix);
	}

	QString FileNamingPolicy::uniqueFilePath(const QString &parent_path, const QFileInfo &info)
	{
		return uniqueFilePath(QDir(parent_path), info.baseName(), info.completeSuffix());
	}

	QString FileNamingPolicy::uniqueFilePath(const QDir &parent_dir, const QString &base_name, const QString &suffix)
	{
		FileNamingPolicy policy;

		return policy.getUniqueFilePath(parent_dir, base_name, suffix);
	}

	QString FileNamingPolicy::uniqueFilePath(const QDir &parent_dir, const QFileInfo &info)
	{
		return uniqueFilePath(parent_dir, info.baseName(), info.completeSuffix());
	}
}
