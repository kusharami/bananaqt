#include "Config.h"

#include "QtnPropertySearchPaths.h"

#include "BananaCore/Const.h"
#include "BananaCore/Config.h"

#include <QSettings>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QTranslator>

static void InitResources()
{
	Q_INIT_RESOURCE(BananaUI);
}

namespace Banana
{

static const char szDefaultProjectLocationKey[] = "DefaultProjectLocation";
static const char szLastNewFileExtensionKey[] = "LastNewFileExtension";
static const char szCurrentLanguageKey[] = "CurrentLanguage";
static const char szConfig[] = "Config";
static const char szShowInFinder[] = QT_TRANSLATE_NOOP("Config", "Show in Finder");
static const char szShowInFinderFmt[] = QT_TRANSLATE_NOOP("Config", "Show '%1' in Finder");
static const char szShowInFinderHint[] = QT_TRANSLATE_NOOP("Config", "Shows a selected file or directory in Finder");
static const char szShowInExplorer[] = QT_TRANSLATE_NOOP("Config", "Show in Explorer");
static const char szShowInExplorerFmt[] = QT_TRANSLATE_NOOP("Config", "Show '%1' in Explorer");
static const char szShowInExplorerHint[] = QT_TRANSLATE_NOOP("Config", "Shows a selected file or directory in Explorer");

namespace UI
{
	void Register()
	{
		InitResources();

		QtnPropertySearchPaths::Register();
	}
	
	void InstallTranslations(const QLocale &locale)
	{
		static QTranslator translator;
		if (translator.load(locale, "BananaUI.qm", "", ":/Translations"))
			QCoreApplication::installTranslator(&translator);
	}
}

QString getDefaultProjectLocation()
{
	QSettings settings;

	return settings.value(szDefaultProjectLocationKey,
	  QDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation))
	  .absoluteFilePath(QCoreApplication::applicationName())).toString();
}

void setDefaultProjectLocation(const QString &path)
{
	QSettings settings;
	settings.setValue(szDefaultProjectLocationKey, path);
	settings.sync();
}

QString getLastNewFileExtension()
{
	QSettings settings;
	return settings.value(szLastNewFileExtensionKey).toString();
}

void setLastNewFileExtension(const QString &ext)
{
	QSettings settings;
	settings.setValue(szLastNewFileExtensionKey, ext);
	settings.sync();
}

QLocale::Language getCurrentLanguage()
{
	auto defaultLang = QLocale::system().language();
	if (defaultLang != QLocale::Russian)
		defaultLang = QLocale::English;

	QSettings settings;
	return QLocale(settings.value(szCurrentLanguageKey,
								  QLocale(defaultLang).name())
				   .toString()).language();
}

void setCurrentLanguage(QLocale::Language language)
{
	QSettings settings;
	settings.setValue(szCurrentLanguageKey, QLocale(language).name());
	settings.sync();
}

QString getShowInGraphicalShellCommandText(const QString &what)
{
	if (what.isEmpty())
	{
		switch (Banana::getHostOS())
		{
			case OS_WINDOWS:
				return QCoreApplication::translate(szConfig, szShowInExplorer);

			case OS_MAC:
				return QCoreApplication::translate(szConfig, szShowInFinder);

			case OS_UNKNOWN:
				break;
		}
	} else
	{
		switch (Banana::getHostOS())
		{
			case OS_WINDOWS:
				return QCoreApplication::translate(szConfig, szShowInExplorerFmt).arg(what);

			case OS_MAC:
				return QCoreApplication::translate(szConfig, szShowInFinderFmt).arg(what);

			case OS_UNKNOWN:
				break;
		}
	}

	return QString();
}

QString getShowInGraphicalShellHintText()
{
	switch (Banana::getHostOS())
	{
		case OS_WINDOWS:
			return QCoreApplication::translate(szConfig, szShowInExplorerHint);

		case OS_MAC:
			return QCoreApplication::translate(szConfig, szShowInFinderHint);

		case OS_UNKNOWN:
			break;
	}

	return QString();
}

}
