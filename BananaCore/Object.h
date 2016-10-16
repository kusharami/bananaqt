#pragma once

#include <QObject>
#include <QVariantMap>

#include <bitset>
#include <vector>

class QMimeData;

namespace Banana
{
	class UndoStack;

	extern const char szOBJECT_NAME_KEY[];
	extern const char szCLASS_NAME_KEY[];
	extern const char szCHILDREN_KEY[];

	typedef std::vector<const char *> PropertyNames;
	typedef std::vector<QMetaProperty> QMetaPropertyVec;

	class Object : public QObject
	{
		Q_OBJECT

	public:
		explicit Object();
		virtual ~Object();

		template <typename T, typename... ARG_T>
		static T *create(QObject *parent, ARG_T... args);

		template <typename CLASS>
		CLASS *getPrototypeAs() const;

		Q_INVOKABLE inline QObject *getPrototype() const;
		void setPrototype(Object *prototype);

		static bool loadContents(const QVariantMap &source, QObject *destination, bool skipObjectName);
		static void saveContents(const QObject *source, QVariantMap &destination, QObject *prototype = nullptr);

		enum SaveMode
		{
			SavePrototyped,
			SaveStandaloneInheritedChild,
			SaveStandalone,
		};

		virtual bool loadContents(const QVariantMap &source, bool skipObjectName);
		virtual void saveContents(QVariantMap &destination, SaveMode saveMode = SavePrototyped);

		Q_INVOKABLE void applyContents(const QVariantMap &source);

		inline UndoStack *getUndoStack() const;
		void setUndoStack(UndoStack *undoStack, bool own = false);
		Q_INVOKABLE void beginMacro(const QString &text);
		Q_INVOKABLE void endMacro();
		Q_INVOKABLE bool macroIsRecording() const;
		Q_INVOKABLE bool undoStackIsUpdating() const;
		template <typename T>
		inline void pushUndoCommand(const char *propertyName, const T &oldValue);
		bool canPushUndoCommand() const;

		Q_INVOKABLE void addChildCommand(QObject *child);
		Q_INVOKABLE void moveChildCommand(QObject *child, QObject *oldParent);
		Q_INVOKABLE void deleteChildCommand(QObject *child);

		Q_INVOKABLE void assign(QObject *source);

		Q_INVOKABLE void removeAllChildren();

		Q_INVOKABLE bool isAncestorOf(const QObject *object) const;
		Q_INVOKABLE bool isDescendantOf(const QObject *object) const;

		Q_INVOKABLE inline bool isLoading() const;

		Q_INVOKABLE void beginLoad();
		Q_INVOKABLE void endLoad();

		Q_INVOKABLE inline bool isInheritedChild() const;
		Q_INVOKABLE inline bool isPrototypedRoot() const;
		Q_INVOKABLE inline bool isPrototyped() const;
		virtual bool canBeUsedAsPrototype(Object *object) const;

		inline bool isModified() const;
		void setModified(bool value);
		static void modifyObject(QObject *object, bool modified, bool signalize, bool children);

		inline bool isDeleted() const;

		void beginReload();
		void endReload();
		void beginUndoStackUpdate();
		void endUndoStackUpdate();

		static QObject *getTopAncestor(QObject *object);
		QStringList getNamesChain(const QObject *topAncestor = nullptr) const;
		static QStringList getNamesChain(const QObject *topAncestor, const QObject *bottomDescendant);

		static QObject *loadQObjectPointer(const QMetaObject *metaObject, const QMimeData *data);
		static void saveQObjectPointer(const QObject *object, QMimeData *data);

		template <typename T>
		static inline T *loadObjectPointer(const QMimeData *data);

		inline bool isPropertyModified(int propertyId) const;
		bool setPropertyModified(int propertyId, bool modified);

		inline quint64 getPropertyModifiedBits() const;
		void setPropertyModifiedBits(quint64 propertyIdBits);

		QMetaPropertyVec getPropertyListAffectedBy(const QMetaProperty &metaProperty) const;

		void deprototype();

	signals:
		void modifiedSetChanged();
		void modifiedFlagChanged(bool modified);
		void parentChanged();
		void reloadStarted();
		void reloadFinished();
		void childAdded(QObject *child);
		void childRemoved(QObject *child);
		void beforeDestroy(QObject *object);

	private slots:
		void onPrototypeChildAdded(QObject *protoChild);
		void onPrototypeChildRemoved(QObject *protoChild);
		void onPrototypeDestroyed(QObject *object);
		void onPrototypeReloadStarted();
		void onPrototypeReloadFinished();
		void onLinkedObjectNameChanged(const QString &name);
		void onObjectNameChanged(const QString &newName);

	private:
		void pushUndoCommandInternal(const char *propertyName, const QVariant &oldValue);
		Object *getMainPrototype() const;
		void internalAssign(QObject *source, bool fresh, bool top);
		void internalSetPrototype(Object *prototype, bool child, bool fresh);
		void connectChildPrototypeDestroy();
		void disconnectChildPrototypeDestroy();
		void connectPrototypeChildLifeCycle();
		void disconnectPrototypeChildLifeCycle();
		void connectPrototype();
		void disconnectPrototype();
		void connectChildPrototype();
		void disconnectChildPrototype();
		bool checkPrototypeCycling(const Object *object) const;
		static bool isDescendantOf(const QObject *ancestor, const QObject *object);
		bool assignChild(QObject *sourceChild, bool is_prototype = true);
		void beforePrototypeReloadStarted();
		void beforeChildPrototypeReloadStarted();
		void beforePrototypeChange();
		void afterPrototypeChange();

	protected:
		virtual int internalPropertyCount() const;

		bool shouldSwapModifiedFieldsFor(QObject *source) const;
		virtual bool canAssignPropertyFrom(QObject *source, int propertyId) const;
		virtual void doConnectPrototype();
		virtual void doDisconnectPrototype();
		virtual bool assignBegin(QObject *source, bool top);
		virtual void assignEnd(QObject *source, bool top);
		virtual void assignChildren(QObject *source);
		virtual void assignProperties(QObject *source);
		void newChildFrom(QObject *source);
		virtual const QMetaObject *getStorableChildType(const QString &className) const;
		virtual bool isStorableChildType(const QMetaObject *type) const;
		virtual void childEvent(QChildEvent *event) override;
		virtual void doParentChange();
		virtual void doFlagsChanged();
		virtual void doAddChild(QObject *object);
		virtual void doRemoveChild(QObject *object);

		QString oldName;
		Object *prototype;
		Object *childPrototype;
		unsigned reloadCounter;
		unsigned loadCounter;
		unsigned macroCounter;
		unsigned undoStackUpdate;
		bool modified;
		bool deleted;

	private:
		UndoStack *undoStack;
		bool ownUndoStack;

		typedef std::bitset<64> ModifiedSet;
		ModifiedSet modifiedSet;
	};

	QObject *Object::getPrototype() const
	{
		return prototype;
	}

	UndoStack *Object::getUndoStack() const
	{
		return undoStack;
	}

	template <typename T>
	void Object::pushUndoCommand(const char *propertyName, const T &oldValue)
	{
		if (canPushUndoCommand())
			pushUndoCommandInternal(propertyName, QVariant::fromValue<T>(oldValue));
	}

	bool Object::isLoading() const
	{
		return loadCounter > 0;
	}

	bool Object::isInheritedChild() const
	{
		return (nullptr != childPrototype);
	}

	bool Object::isPrototypedRoot() const
	{
		return (nullptr != prototype && childPrototype != prototype);
	}

	bool Object::isPrototyped() const
	{
		return (nullptr != prototype);
	}

	bool Object::isModified() const
	{
		return modified;
	}

	bool Object::isDeleted() const
	{
		return deleted;
	}

	bool Object::isPropertyModified(int propertyId) const
	{
		return modifiedSet.test(propertyId);
	}

	quint64 Object::getPropertyModifiedBits() const
	{
		return modifiedSet.to_ullong();
	}

	template <typename T>
	T *Object::loadObjectPointer(const QMimeData *data)
	{
		return static_cast<T *>(loadQObjectPointer(&T::staticMetaObject, data));
	}

	template <typename T, typename... ARG_T>
	T *Object::create(QObject *parent, ARG_T... args)
	{
		auto result = new T(args...);

		result->setParent(parent);

		return result;
	}

	template <typename CLASS>
	CLASS *Object::getPrototypeAs() const
	{
		if (prototype != nullptr)
			Q_ASSERT(nullptr != dynamic_cast<CLASS *>(prototype));

		return static_cast<CLASS *>(prototype);
	}

}
