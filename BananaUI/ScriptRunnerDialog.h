/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2016-2017 Alexandra Cherdantseva

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

#include "BananaScript/IScriptRunner.h"

#include <QDialog>

#include <set>

namespace Ui
{
class ScriptRunnerDialog;
}

class QTimer;

namespace Banana
{
class ProjectGroup;
class ScriptRunner;
struct IAbortDelegate;
struct ScriptSubDialogHandler;
class ScriptRunnerDialog
	: public QDialog
	, public IScriptRunner
{
	Q_OBJECT

	Ui::ScriptRunnerDialog *ui;

	QString lastFilePath;
	ProjectGroup *group;
	ScriptRunner *runner;
	IAbortDelegate *abortDelegate;
	qint64 lastRespondTime;
	int timerId;
	bool running;
	bool checkRespond;
	bool waiting;
	bool stopShow;
	static bool cursorIsSet;

public:
	enum
	{
		CHECK_RESPOND_SECONDS = 25
	};

	explicit ScriptRunnerDialog(QWidget *parent = nullptr);
	virtual ~ScriptRunnerDialog() override;

	inline void setProjectGroup(ProjectGroup *group);
	void showModal(ScriptRunner *runner, ProjectGroup *group,
		const QString &scriptFilePath);
	virtual void beforeScriptExecution(const QString &filePath) override;
	virtual void afterScriptExecution(bool ok, const QString &message) override;
	virtual void log(const QString &text) override;
	bool abort();

	inline void setAbortDelegate(IAbortDelegate *d);

	static void restoreCursor();

protected:
	virtual void initializeEngine(QScriptEngine *engine) override;
	virtual void closeEvent(QCloseEvent *event) override;
	virtual void timerEvent(QTimerEvent *event) override;

private slots:
	void on_buttonBrowse_clicked();

	void on_btnInsertFilePath_clicked();

	void on_btnInsertDirectoryPath_clicked();

	void on_btnInsertProjectItem_clicked();

	void on_closeButton_clicked();

	void on_runButton_clicked();

signals:
	void registerFilePath(const QString &filepath);
	void shouldDisableParentWindow();
	void shouldEnableParentWindow();

private:
	friend struct ScriptSubDialogHandler;

	static QString getRunText();
	static QString getAbortText();

	void showMe();
	static void setWaitCursor();
	void beginWait();
	void endWait();
	void showProgressBar();
	void hideProgressBar();
	void scriptRuntimeError(const QString &message);
	void connectLog();
	void disconnectLog();
	void insertList(const QStringList &value);
	void runScript();
	void setScriptFilePath(const QString &filepath, bool reg = true);
	void registerScript();
};

void ScriptRunnerDialog::setProjectGroup(ProjectGroup *group)
{
	this->group = group;
}

void ScriptRunnerDialog::setAbortDelegate(IAbortDelegate *d)
{
	abortDelegate = d;
}
}
