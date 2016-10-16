#pragma once

#include "BananaCore/BaseFileManager.h"

class QProgressDialog;

namespace Banana
{
	class AbstractFileSystemObject;

class PasteFileManager : public QObject, public Banana::BaseFileManager
{
	Q_OBJECT

public:
	PasteFileManager(QWidget *parent_widget = nullptr);

protected:
	virtual bool wasCanceled() const override;
	virtual int shouldReplaceFile(const QFileInfo &info) override;
	virtual int error(int action, int what, const QFileInfo &info) override;

	virtual void processStarted() override;
	virtual void processFinished() override;
	virtual void processAborted() override;
	virtual void processFileStarted(const QDir &pasteDir, const QFileInfo &file_src) override;
	virtual void processFileFinished(const QFileInfo &file_src, const QFileInfo &file_dst) override;
	virtual void processFileAborted(const QDir &pasteDir, const QFileInfo &file_src) override;

private:
	void endFileProcess(Banana::AbstractFileSystemObject *fsys);

	QWidget *parent_widget;
	QProgressDialog *dialog;
	int replace_button;
};
}
