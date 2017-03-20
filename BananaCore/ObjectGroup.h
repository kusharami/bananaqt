/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2016 Alexandra Cherdantseva

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

#include "UniqueNameScope.h"
#include "Object.h"
#include "AbstractObjectGroup.h"

#include <map>
#include <memory>

namespace Banana
{
struct IChildFilter;
enum class ConnectionState;
class ObjectGroup : public Object, public AbstractObjectGroup
{
	Q_OBJECT

public:
	explicit ObjectGroup();

	void registerChildType(
		const QMetaObject *meta_object, Qt::CaseSensitivity sensitivity =
			Qt::CaseInsensitive);
	void unregisterChildType(const QMetaObject *meta_object,
							 bool children = true);
	void unregisterAllChildTypes(bool children = true);

	bool isSupportedChildType(const QMetaObject *meta_object) const;

	virtual const QObjectList &getChildren() override;
	virtual void resetChildren() override;

signals:
	void childObjectConnectionChanged(QObject *object, ConnectionState state);

protected:
	virtual UniqueNameScope *createNameScope(const QMetaObject *meta_object,
											 Qt::CaseSensitivity sensitivity)
	const;
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
