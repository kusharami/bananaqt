#pragma once

#include <QTabBar>
#include <QMetaMethod>

#include <vector>
#include <functional>

namespace Banana
{
	class UndoStack;
	class AbstractFile;

class FileTabBar : public QTabBar
{
	Q_OBJECT

public:
	explicit FileTabBar(QWidget *parent = nullptr);

	typedef std::function<bool (Banana::AbstractFile *)> CanOpenFunc;
	typedef std::function<bool (Banana::AbstractFile *, bool multiple)> CanCloseFunc;

	void setCanOpen(const CanOpenFunc &canOpen);
	void setCanClose(const CanCloseFunc &canClose);

	Banana::AbstractFile *getFileAtIndex(int index) const;
	Banana::AbstractFile *getCurrentFile() const;
	Banana::UndoStack *getCurrentUndoStack() const;

	bool fileOpen(Banana::AbstractFile *file);
	void fileClose(Banana::AbstractFile *file, bool multiple = false);

	bool isFileOpen(Banana::AbstractFile *file) const;

signals:
	void fileOpenError(Banana::AbstractFile *file);
	void fileOpened(Banana::AbstractFile *file);
	void fileFlagsChanged(Banana::AbstractFile *file);
	void fileClosed(Banana::AbstractFile *file);

private slots:
	void onTabCloseRequested(int index);
	void onTabMoved(int from, int to);
	void onFileClosed();
	void onFileDestroyed(QObject *object);
	void onFileFlagsChanged();
	void updateTabText();

private:
	static QString getFileIsModifiedFmt();

	void closeTab(int index, bool multiple);
	void doFileClose(Banana::AbstractFile *file);
	static QString getTitleForFile(Banana::AbstractFile *file);
	static QString getToolTipForFile(Banana::AbstractFile *file);

	void connectFile(Banana::AbstractFile *file);
	void disconnectFile(Banana::AbstractFile *file);

	typedef std::vector<Banana::AbstractFile *> FileVector;

	int getFileIndex(QObject *fileObject, bool valid = false) const;

	CanOpenFunc canOpen;
	CanCloseFunc canClose;
	FileVector openedFiles;
};

}
