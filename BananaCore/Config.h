#pragma once

#include <qsystemdetection.h>

#ifdef Q_OS_MAC
#	define FILE_DIALOG_FLAGS QFileDialog::DontUseNativeDialog
#else
#	define FILE_DIALOG_FLAGS (QFileDialog::Option) 0
#endif

class QLocale;

namespace Core
{
	void Register();
	void InstallTranslations(const QLocale &locale);

	enum OS
	{
		OS_UNKNOWN,
		OS_WINDOWS,
		OS_MAC
	};

	OS getHostOS();
}
