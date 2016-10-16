#include "Const.h"

#include <qglobal.h>
#include <QFont>
#include <QCoreApplication>

namespace Core
{
	const char pX[] = "x";
	const char pY[] = "y";
	const char pWidth[] = "width";
	const char pHeight[] = "height";
	const char pNoExtension[] = "";
	const char pUntitledFileName[] = QT_TRANSLATE_NOOP("FileNamingPolicy", "Untitled");
	const char pFalse[] = QT_TRANSLATE_NOOP("Boolean", "False");
	const char pTrue[] = QT_TRANSLATE_NOOP("Boolean", "True");

	QFont getDefaultFont()
	{
		QFont result;
		result.setStyleStrategy(QFont::PreferAntialias);
		result.setPixelSize(result.pointSize());
		return result;
	}

	QString getBoolString(bool value)
	{
		if (value)
			return QCoreApplication::translate("Boolean", pTrue);

		return QCoreApplication::translate("Boolean", pFalse);
	}

}
