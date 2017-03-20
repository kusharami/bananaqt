/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2016 Alexandra Cherdantseva

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/

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
static const char szShowInFinder[] =
	QT_TRANSLATE_NOOP("Config", "Show in Finder");
static const char szShowInFinderFmt[] =
	QT_TRANSLATE_NOOP("Config", "Show '%1' in Finder");
static const char szShowInFinderHint[] =
	QT_TRANSLATE_NOOP("Config", "Shows a selected file or directory in Finder");
static const char szShowInExplorer[] =
	QT_TRANSLATE_NOOP("Config", "Show in Explorer");
static const char szShowInExplorerFmt[] =
	QT_TRANSLATE_NOOP("Config", "Show '%1' in Explorer");
static const char szShowInExplorerHint[] =
	QT_TRANSLATE_NOOP(
		"Config", "Shows a selected file or directory in Explorer");

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

	return settings.value(
		szDefaultProjectLocationKey,
		QDir(
			QStandardPaths::writableLocation(
				QStandardPaths::DocumentsLocation))
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
	return QLocale(
		settings.value(
			szCurrentLanguageKey,
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
				return QCoreApplication::translate(
					szConfig,
					szShowInExplorerFmt).arg(what);

			case OS_MAC:
				return QCoreApplication::translate(
					szConfig,
					szShowInFinderFmt).arg(what);

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
