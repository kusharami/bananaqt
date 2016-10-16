#pragma once

#include "UniqueNameScope.h"
#include "Object.h"
#include "AbstractObjectGroup.h"

#include <map>
#include <memory>

namespace Core
{
	struct IChildFilter;
	enum class ConnectionState;
	class ObjectGroup : public Object, public AbstractObjectGroup
	{
		Q_OBJECT
	public:
		explicit ObjectGroup();

		void registerChildType(const QMetaObject *meta_object,
							   Qt::CaseSensitivity sensitivity = Qt::CaseInsensitive);
		void unregisterChildType(const QMetaObject *meta_object, bool children = true);
		void unregisterAllChildTypes(bool children = true);

		bool isSupportedChildType(const QMetaObject *meta_object) const;

		virtual const QObjectList &getChildren() override;

		virtual void resetChildren() override;

	signals:
		void childObjectConnectionChanged(QObject *object, ConnectionState state);

	protected:
		virtual UniqueNameScope *createNameScope(const QMetaObject *meta_object,
												 Qt::CaseSensitivity sensitivity) const;
		virtual void doAddChild(QObject *object) override;
		virtual void doRemoveChild(QObject *object) override;
		virtual void doConnectChildObject(QObject *object);
		virtual void doDisconnectChildObject(QObject *object);

	private slots:
		void onChildObjectDestroyed(QObject *object);
		void childrenNeedRearrange();

	private:
		void reconnectChildren(UniqueNameScope *new_scope);
		void connectChildObject(QObject *object, UniqueNameScope *scope);
		void disconnectChildObject(QObject *object, UniqueNameScope *scope);

	protected:
		void makeChildList();

		typedef std::unique_ptr<UniqueNameScope> NameScopePtr;
		typedef std::map<const QMetaObject *, NameScopePtr> ChildTypesMap;

		QObjectList m_children;
		ChildTypesMap child_types;
	};
}
