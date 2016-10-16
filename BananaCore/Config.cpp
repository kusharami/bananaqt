#include "Config.h"

#include "Core.h"
#include "SearchPaths.h"
#include "BinaryFile.h"
#include "Const.h"
#include "Directory.h"

#include <QLocale>
#include <QTranslator>
#include <QCoreApplication>

static void InitResources()
{
	Q_INIT_RESOURCE(CoreFramework);
}

namespace Core
{

	void Register()
	{
		(void) QT_TRANSLATE_NOOP("ClassName", "QObject");
		(void) QT_TRANSLATE_NOOP("QObject", "objectName");

		InitResources();

		qRegisterMetaType<QVariantMap>();
		qRegisterMetaType<SearchPaths *>();

		Directory::registerFileType(pNoExtension,
									&BinaryFile::staticMetaObject,
									&BinaryData::staticMetaObject);
	}

	void InstallTranslations(const QLocale &locale)
	{
		static QTranslator translator;
		if (translator.load(locale, "Core.qm", "", ":/Translations"))
		{
			QCoreApplication::installTranslator(&translator);
		}
	}

	OS getHostOS()
	{
#if defined(Q_OS_WIN)
		return OS_WINDOWS;
#elif defined(Q_OS_MAC)
		return OS_MAC;
#else
		return OS_UNKNOWN;
#endif
	}

}
