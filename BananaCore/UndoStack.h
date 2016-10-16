#pragma once

#include <QUndoStack>

namespace Banana
{

	class UndoStack : public QUndoStack
	{
		Q_OBJECT

	public:
		UndoStack(QObject *parent = nullptr);

		void beginUpdate();
		void endUpdate();
		void beginMacro(const QString &text);
		void endMacro();

		void clear(bool force = false);

		inline bool isUpdating() const;
		inline bool macroIsRecording() const;

		static QString getDragAndDropCommandText(Qt::DropAction action);

	private:
		unsigned macroCounter;
		unsigned updateCounter;
	};

	bool UndoStack::isUpdating() const
	{
		return updateCounter > 0;
	}

	bool UndoStack::macroIsRecording() const
	{
		return macroCounter > 0;
	}

}
