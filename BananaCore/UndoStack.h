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
		void blockMacro();
		void unblockMacro();

		void clear(bool force = false);
		void setClean();

		inline bool isUpdating() const;
		bool canPushForMacro() const;

		static QString getDragAndDropCommandText(Qt::DropAction action);

	signals:
		void macroStarted();
		void macroFinished();

	protected:
		virtual void customEvent(QEvent *event) override;

	private slots:
		void onCleanChanged(bool clean);

	private:
		unsigned blockCounter;
		unsigned macroCounter;
		unsigned updateCounter;
		bool firstClean;
	};

	bool UndoStack::isUpdating() const
	{
		return updateCounter > 0;
	}



}
