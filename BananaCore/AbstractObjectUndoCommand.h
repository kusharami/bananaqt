#pragma once

#include <QObject>
#include <QUndoCommand>

namespace Core
{
	class AbstractObjectUndoCommand : public QObject, public QUndoCommand
	{
	public:
		AbstractObjectUndoCommand(QObject *object);

	protected:
		virtual void undo() override;
		virtual void redo() override;

		virtual void doUndo() = 0;
		virtual void doRedo() = 0;

		QObject *getObject() const;
		void fetchObject();

	private slots:
		void onObjectDestroyed();

	private:
		void connectObject();
		void disconnectObject();

		QObject *objectParent;
		QObject *object;
		QStringList objectPath;
		QString objectName;

		bool skipRedoOnPush;
	};



}
