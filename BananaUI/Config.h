#pragma once

#include <QString>
#include <QLocale>

namespace Banana
{

	namespace UI
	{
		void Register();
		void InstallTranslations(const QLocale &locale);
	}

	QString getDefaultProjectLocation();
	void setDefaultProjectLocation(const QString &path);

	QString getLastNewFileExtension();
	void setLastNewFileExtension(const QString &ext);

	QLocale::Language getCurrentLanguage();
	void setCurrentLanguage(QLocale::Language language);

	QString getShowInGraphicalShellCommandText(const QString &what = QString());
	QString getShowInGraphicalShellHintText();

}

