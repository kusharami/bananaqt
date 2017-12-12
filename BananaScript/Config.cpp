/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2017 Alexandra Cherdantseva

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
#include "ScriptManager.h"

#include "BananaCore/BinaryFile.h"
#include "BananaCore/Directory.h"

#include <QLocale>
#include <QTranslator>
#include <QCoreApplication>

static void InitResources()
{
	Q_INIT_RESOURCE(BananaScript);
}

namespace Banana
{
namespace Script
{
void Register()
{
	InitResources();

	qRegisterMetaType<ScriptManager *>();

	ScriptManager::registerMetaObject(&Object::staticMetaObject);
	ScriptManager::registerMetaObject(&AbstractFile::staticMetaObject);
	ScriptManager::registerMetaObject(&BinaryFile::staticMetaObject);
	ScriptManager::registerMetaObject(&Directory::staticMetaObject);
}

void InstallTranslations(const QLocale &locale)
{
	static QTranslator translator;
	if (translator.load(locale, "BananaScript.qm", "", ":/Translations"))
	{
		static bool installOnce = false;
		if (not installOnce)
		{
			installOnce = true;
			QCoreApplication::installTranslator(&translator);
		}
	}
}
}
}
