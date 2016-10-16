#pragma once

#include "AbstractObjectUndoCommand.h"

#include <QVariant>
#include <QMetaProperty>

#include <vector>
#include <map>

namespace Core
{
	class Object;

	class ChangeValueCommand : public AbstractObjectUndoCommand
	{
		Q_OBJECT

	public:
		static QString getMultipleResetCommandTextFor(const QMetaObject *metaObject, const char *propertyName);
		static QString getMultipleResetCommandTextFor(const QMetaObject *metaObject, const QMetaProperty &metaProperty);
		static QString getResetCommandTextFor(Object *object, const char *propertyName);
		static QString getResetCommandTextFor(Object *object, const QMetaProperty &metaProperty);

		static QString getMultipleCommandTextFor(const QMetaObject *metaObject,
												 const char *propertyName);
		static QString getMultipleCommandTextFor(const QMetaObject *metaObject,
												 const QMetaProperty &metaProperty);

		static QString getCommandTextFor(Object *object, const char *propertyName);
		static QString getCommandTextFor(Object *object, const QMetaProperty &metaProperty);

		static QString resetCommandPattern();
		static QString changeValueCommandPattern();
		static QString setValueCommandPattern();
		static QString multipleObjectsStr();

	public:
		ChangeValueCommand(Object *object,
						   const QMetaProperty &metaProperty,
						   const QVariant &oldValue);
		ChangeValueCommand(Object *object, int propertyId, bool oldState);
		ChangeValueCommand(Object *object, quint64 oldStateBits);

		virtual int id() const override;
		virtual bool mergeWith(const QUndoCommand *other) override;

	protected:
		virtual void doUndo() override;
		virtual void doRedo() override;

	private:
		void applyValues(bool redo);
		void applyStateBits(quint64 bits);

		struct EntryData
		{
			QMetaProperty metaProperty;

			QVariant oldValue;
			QVariant newValue;

			size_t index;
		};

		typedef std::vector<const EntryData *> OrderedEntries;

		void pushEntry(const EntryData &entryData);
		void pushEntry(const QMetaProperty &metaProperty, const QVariant &oldValue);
		void prepareOrderedEntries();
		void prepareOrderedEntries(OrderedEntries &orderedEntries) const;
		static bool entryIndexLess(const EntryData *a, const EntryData *b);

		OrderedEntries orderedEntries;
		std::map<int, EntryData> entries;

		quint64 oldStateBits;
		quint64 newStateBits;
	};

}
