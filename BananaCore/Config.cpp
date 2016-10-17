/*
 * MIT License
 *
 * Copyright (c) 2016 Alexandra Cherdantseva
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
	Q_INIT_RESOURCE(BananaCore);
}

namespace Banana
{

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
			if (translator.load(locale, "BananaCore.qm", "", ":/Translations"))
			{
				QCoreApplication::installTranslator(&translator);
			}
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
