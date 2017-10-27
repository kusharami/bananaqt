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

#include "MainWindow.h"

#if defined(Q_OS_WIN)
#include <windows.h>
#elif defined(Q_OS_MAC)
#include <QFileOpenEvent>
#endif

#include <QDebug>
#include <QMessageBox>
#include <QApplication>
#include <QRegExp>
#include <QDir>

namespace Banana
{
struct MainWindowPrivate
{
	MainWindow *parent;

#ifdef Q_OS_WIN

	// implementation of the WM_DDE_INITIATE windows message
	bool ddeInitiate(MSG *message, long *result);

	// implementation of the WM_DDE_EXECUTE windows message
	bool ddeExecute(MSG *message, long *result);

	// implementation of the WM_DDE_TERMINATE windows message
	bool ddeTerminate(MSG *message, long *result);

	// Sets specified value in the registry under HKCU\Software\Classes, which is mapped to HKCR then.
	bool SetHkcrUserRegKey(QString key, const QString &value,
		const QString &valueName = QString::null);

	// ----- members -------------------------------------------------------------------------------
	QString
		appAtomName; /**< the name of the application, without file extension */
	QString
		systemTopicAtomName; /**< the name of the system topic atom, typically "System" */
	ATOM appAtom; /**< The windows atom needed for DDE communication */
	ATOM
		systemTopicAtom; /**< The windows system topic atom needed for DDE communication */
#endif

	void enableOpenOutside();
	void executeOutsideCommand(const QString &command, const QString &params);

	MainWindowPrivate(MainWindow *parent);
	~MainWindowPrivate();
};

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, p(new MainWindowPrivate(this))
{
}

MainWindow::~MainWindow()
{
	delete p;
}

void MainWindow::registerFileType(const QString &documentId,
	const QString &fileTypeName, const QString &fileExtension,
	qint32 appIconIndex)
{
#ifdef Q_OS_WIN
	// first register the type ID of our server
	if (!p->SetHkcrUserRegKey(documentId, fileTypeName))
		return;
	if (!p->SetHkcrUserRegKey(QString("%1\\DefaultIcon").arg(documentId),
			QString("\"%1\",%2")
				.arg(QDir::toNativeSeparators(
					QApplication::applicationFilePath()))
				.arg(appIconIndex)))
		return;

	registerCommand("Open", documentId, " %1", "[open(\"%1\")]");

#ifdef UNICODE
	WCHAR szExtension[_MAX_PATH];
	szExtension[fileExtension.toWCharArray(szExtension)] = 0;
#else
	QByteArray sExtension = fileExtension.toLocal8Bit();
	auto szExtension = sExtension.constData();
#endif

	LONG lSize = _MAX_PATH * 2;
	TCHAR szTempBuffer[_MAX_PATH * 2];
	LONG lResult =
		::RegQueryValue(HKEY_CLASSES_ROOT, szExtension, szTempBuffer, &lSize);
#ifdef UNICODE
	QString temp = QString::fromWCharArray(szTempBuffer, lSize);
#else
	QString temp = QString::fromLocal8Bit(szTempBuffer, lSize);
#endif
	if (lResult != ERROR_SUCCESS || temp.isEmpty() || temp == documentId)
	{
		// no association for that suffix
		if (!p->SetHkcrUserRegKey(fileExtension, documentId))
			return;
		p->SetHkcrUserRegKey(
			QString("%1\\ShellNew").arg(fileExtension), QString(), "NullFile");
	}
#else
	Q_UNUSED(documentId);
	Q_UNUSED(fileTypeName);
	Q_UNUSED(fileExtension);
	Q_UNUSED(appIconIndex);
#endif
}

void MainWindow::registerCommand(const QString &command,
	const QString &documentId, const QString cmdLineArg,
	const QString ddeCommand)
{
#ifdef Q_OS_WIN
	QString commandLine =
		QDir::toNativeSeparators(QApplication::applicationFilePath());
	commandLine.prepend(QLatin1String("\""));
	commandLine.append(QLatin1String("\""));
	if (!cmdLineArg.isEmpty())
	{
		commandLine.append(QChar(' '));
		commandLine.append(cmdLineArg);
	}
	if (!p->SetHkcrUserRegKey(
			QString("%1\\shell\\%2\\command").arg(documentId).arg(command),
			commandLine))
		return; // just skip it
	if (!ddeCommand.isEmpty())
	{
		if (!p->SetHkcrUserRegKey(
				QString("%1\\shell\\%2\\ddeexec").arg(documentId).arg(command),
				ddeCommand))
			return;
		if (!p->SetHkcrUserRegKey(QString("%1\\shell\\%2\\ddeexec\\application")
									  .arg(documentId)
									  .arg(command),
				p->appAtomName))
			return;
		if (!p->SetHkcrUserRegKey(QString("%1\\shell\\%2\\ddeexec\\topic")
									  .arg(documentId)
									  .arg(command),
				p->systemTopicAtomName))
			return;
	}
#else
	Q_UNUSED(command);
	Q_UNUSED(documentId);
	Q_UNUSED(cmdLineArg);
	Q_UNUSED(ddeCommand);
#endif
}

void MainWindow::enableOpenOutside()
{
	p->enableOpenOutside();
}

bool MainWindow::nativeEvent(
	const QByteArray &eventType, void *message, long *result)
{
#ifdef Q_OS_WIN
	auto msg = reinterpret_cast<MSG *>(message);
	switch (msg->message)
	{
		case WM_DDE_INITIATE:
			return p->ddeInitiate(msg, result);

		case WM_DDE_EXECUTE:
			return p->ddeExecute(msg, result);

		case WM_DDE_TERMINATE:
			return p->ddeTerminate(msg, result);
	}
#endif

	return QMainWindow::nativeEvent(eventType, message, result);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
#ifdef Q_OS_MAC
	if (watched == QApplication::instance())
	{
		if (event->type() == QEvent::FileOpen)
		{
			auto fileOpenEvent = static_cast<QFileOpenEvent *>(event);
			openFileOutside(fileOpenEvent->file());
		}
	}
#endif
	return QMainWindow::eventFilter(watched, event);
}

void MainWindow::openFileOutside(const QString &) {}

void MainWindow::customOutsideCommand(const QString &, const QString &) {}

#ifdef Q_OS_WIN
bool MainWindowPrivate::ddeInitiate(MSG *message, long *result)
{
	if (0 != LOWORD(message->lParam) && 0 != HIWORD(message->lParam) &&
		LOWORD(message->lParam) == appAtom &&
		HIWORD(message->lParam) == systemTopicAtom)
	{
		// make duplicates of the incoming atoms (really adding a reference)
		TCHAR atomName[_MAX_PATH];

		auto atomNameHandler =
			::GlobalGetAtomName(appAtom, atomName, _MAX_PATH - 1);
		Q_ASSERT(atomNameHandler != 0);
		Q_UNUSED(atomNameHandler);

		auto atomHandler = ::GlobalAddAtom(atomName);
		Q_ASSERT(atomHandler == appAtom);
		Q_UNUSED(atomHandler);

		auto topicAtomNameHandler =
			::GlobalGetAtomName(systemTopicAtom, atomName, _MAX_PATH - 1);
		Q_ASSERT(topicAtomNameHandler != 0);
		Q_UNUSED(topicAtomNameHandler);

		auto topicAtomHandler = ::GlobalAddAtom(atomName);
		Q_ASSERT(topicAtomHandler == systemTopicAtom);
		Q_UNUSED(topicAtomHandler);

		// send the WM_DDE_ACK (caller will delete duplicate atoms)
		::SendMessage(reinterpret_cast<HWND>(message->wParam), WM_DDE_ACK,
			static_cast<WPARAM>(parent->winId()),
			MAKELPARAM(appAtom, systemTopicAtom));
	}
	*result = 0;
	return true;
}

bool MainWindowPrivate::ddeExecute(MSG *message, long *result)
{
	// unpack the DDE message
	UINT_PTR unused;
	HGLOBAL hData;
	//IA64: Assume DDE LPARAMs are still 32-bit
	bool unpackDdeParam = ::UnpackDDElParam(WM_DDE_EXECUTE, message->lParam,
		&unused, reinterpret_cast<UINT_PTR *>(&hData));
	Q_ASSERT(unpackDdeParam);
	QString command = QString::fromWCharArray((LPCWSTR)::GlobalLock(hData));
	::GlobalUnlock(hData);
	// acknowledge now - before attempting to execute
	::PostMessage(reinterpret_cast<HWND>(message->wParam), WM_DDE_ACK,
		static_cast<WPARAM>(parent->winId()),
		//IA64: Assume DDE LPARAMs are still 32-bit
		ReuseDDElParam(message->lParam, WM_DDE_EXECUTE, WM_DDE_ACK, 0x8000,
			reinterpret_cast<UINT_PTR>(hData)));
	// don't execute the command when the window is disabled
	if (parent->isEnabled())
	{
		QRegExp regCommand("^\\[(\\w+)\\((.*)\\)\\]$");
		if (regCommand.exactMatch(command))
		{
			executeOutsideCommand(regCommand.cap(1), regCommand.cap(2));
		}
	}
	*result = 0;
	return true;
}

bool MainWindowPrivate::ddeTerminate(MSG *message, long *result)
{
	::PostMessage(reinterpret_cast<HWND>(message->wParam), WM_DDE_TERMINATE,
		static_cast<WPARAM>(parent->winId()), message->lParam);
	*result = 0;
	return true;
}

bool MainWindowPrivate::SetHkcrUserRegKey(
	QString key, const QString &value, const QString &valueName)
{
	HKEY hKey;
	key.prepend("Software\\Classes\\");

#ifdef UNICODE
	WCHAR szKey[1024];
	szKey[key.toWCharArray(szKey)] = 0;
#else
	QByteArray sKey = key.toLocal8Bit();
	auto szKey = sKey.constData();
#endif
	LONG lRetVal = RegCreateKey(HKEY_CURRENT_USER, szKey, &hKey);
	if (ERROR_SUCCESS == lRetVal)
	{
#ifdef UNICODE
		WCHAR szValue[1024];
		szValue[valueName.toWCharArray(szValue)] = 0;
#else
		QByteArray sValue = key.toLocal8Bit();
		auto szValue = sValue.constData();
#endif
		LONG lResult = ::RegSetValueEx(hKey, szValue[0] ? nullptr : szValue, 0,
			REG_SZ, (CONST BYTE *) value.utf16(),
			(value.length() + 1) * sizeof(quint16));
		if (::RegCloseKey(hKey) == ERROR_SUCCESS && lResult == ERROR_SUCCESS)
			return true;
		QMessageBox::warning(QApplication::activeWindow(),
			QApplication::applicationName(),
			MainWindow::tr("Error in setting Registry values.\n"
						   "Registration database update failed for key '%1'.")
				.arg(key));
	} else
	{
		TCHAR buffer[4096];
		auto size = ::FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM, 0, lRetVal, 0, buffer, 4096, 0);
#ifdef UNICODE
		auto errorMessage = QString::fromWCharArray(buffer, size);
#else
		auto szText = QString::fromLocal8Bit(buffer, size);
#endif
		QMessageBox::warning(QApplication::activeWindow(),
			QApplication::applicationName(),
			MainWindow::tr("Error in setting Registry values.\n%1"),
			errorMessage);
	}
	return false;
}

#endif

void MainWindowPrivate::enableOpenOutside()
{
#if defined(Q_OS_WIN)
	if (0 != appAtom || 0 != systemTopicAtom)
		return;

#ifdef UNICODE
	WCHAR name[1024];
	name[appAtomName.toWCharArray(name)] = 0;
#else
	auto sName = appAtomName.toLocal8Bit();
	auto name = sName.constData();
#endif
	appAtom = ::GlobalAddAtom(name);

#ifdef UNICODE
	name[systemTopicAtomName.toWCharArray(name)] = 0;
#else
	sName = systemTopicAtomName.toLocal8Bit();
	name = sName.constData();
#endif
	systemTopicAtom = ::GlobalAddAtom(name);
#elif defined(Q_OS_MAC)
	QApplication::instance()->installEventFilter(parent);
#endif
}

void MainWindowPrivate::executeOutsideCommand(
	const QString &command, const QString &params)
{
	QRegExp regCommand("^\"(.*)\"$");
	bool singleCommand = regCommand.exactMatch(params);
	if (0 == command.compare("open", Qt::CaseInsensitive) && singleCommand)
	{
		parent->openFileOutside(regCommand.cap(1));
		parent->raise();
	} else
	{
		parent->customOutsideCommand(command, params);
	}
}

MainWindowPrivate::MainWindowPrivate(MainWindow *parent)
	: parent(parent)
#ifdef Q_OS_WIN
	, appAtomName(QFileInfo(QApplication::applicationFilePath()).baseName())
	, systemTopicAtomName("system")
	, appAtom(0)
	, systemTopicAtom(0)
#endif
{
}

MainWindowPrivate::~MainWindowPrivate()
{
#ifdef Q_OS_WIN
	if (0 != appAtom)
	{
		::GlobalDeleteAtom(appAtom);
	}
	if (0 != systemTopicAtom)
	{
		::GlobalDeleteAtom(systemTopicAtom);
	}
#endif
}
}
