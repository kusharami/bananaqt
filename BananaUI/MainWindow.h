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

#pragma once

#include <QMainWindow>

namespace Banana
{
struct MainWindowPrivate;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	virtual ~MainWindow();

	virtual void openFileOutside(const QString &filePath);
	virtual void customOutsideCommand(const QString &command,
									  const QString &params);

protected:
	void registerFileType(const QString &documentId,
						  const QString &fileTypeName,
						  const QString &fileExtension,
						  qint32 appIconIndex = 0);
	void registerCommand(const QString &command, const QString &documentId,
						 const QString cmdLineArg = QString::null,
						 const QString ddeCommand = QString::null);
	void enableOpenOutside();
	virtual bool nativeEvent(const QByteArray &eventType, void *message,
							 long *result) override;
	virtual bool eventFilter(QObject *watched, QEvent *event) override;

	friend struct MainWindowPrivate;

private:
	MainWindowPrivate *p;
};

}
