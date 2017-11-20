#pragma once

#include "BananaCore/IUndoStack.h"

#include <QUndoStack>

namespace Banana
{
class UndoStack
	: public QUndoStack
	, public IUndoStack
{
	Q_OBJECT

public:
	UndoStack(QObject *parent = nullptr);

	virtual UndoStack *qundoStack() override;

	virtual void beginUpdate() override;
	virtual void endUpdate() override;
	virtual void beginMacro(const QString &text) override;
	virtual void endMacro() override;
	virtual void blockMacro() override;
	virtual void unblockMacro() override;

	virtual void clear() override;
	virtual void setClean() override;

	virtual bool isUpdating() const override;
	virtual bool canPushForMacro() const override;

	virtual void connectCleanChanged(Object *object, CleanChanged cb) override;
	virtual void disconnectCleanChanged(
		Object *object, CleanChanged cb) override;

	virtual void pushCommand(IUndoCommand *command) override;
	virtual void pushChangeName(Object *object, const QString &oldName,
		const QString &newName) override;
	virtual void pushAddChild(Object *child) override;
	virtual void pushMoveChild(
		Object *child, Object *oldParent, const QString &oldName) override;
	virtual void pushDeleteChild(Object *child) override;
	virtual void pushValueChange(Object *object,
		const QMetaProperty &metaProperty, const QVariant &oldValue) override;
	virtual void pushPropertyChange(
		Object *object, int propertyId, bool oldState) override;
	virtual void pushMultiPropertyChange(
		Object *object, quint64 oldStateBits) override;
	virtual void pushContentsChange(
		Object *object, const QVariantMap &oldContents) override;

	static QString getDragAndDropCommandText(Qt::DropAction action);

signals:
	void macroStarted();
	void macroFinished();

private:
	unsigned blockCounter;
	unsigned macroCounter;
	unsigned updateCounter;
};
}
