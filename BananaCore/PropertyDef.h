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

#define PROP(name) #name
#define MPROP(Name) "m" #Name

#define PUSH_UNDO_COMMAND(Name, oldValue) \
	Banana::Object::pushUndoCommand(MPROP(Name), oldValue)

#define PDEF_NONE
#define PDEF_DESIGNABLE(bool) DESIGNABLE bool

#define PDEF_NO_IMPL ;

#define PDEF_GET_IMPL(name) { return m##name; }
#define PDEF_SET_IMPL(name) { if (isLoading() || m##name != value) { doSet##name(value); } }
#define PDEF_SIMPLE_SET_IMPL(name) { if (m##name != value) { doSet##name(value); } }
#define PDEF_DO_SET_PUSH_IMPL(name) { \
	auto oldValue = m##name; \
	m##name = value; \
	PUSH_UNDO_COMMAND(name, oldValue); \
	emit changed##name(); \
	setModified(true); }
#define PDEF_DO_SET_IMPL(name) { \
	m##name = value; \
	emit changed##name(); \
	setModified(true); }

#define PDEF_GET(FULL_TYPE, name, GET_IMPL) \
	FULL_TYPE get##name() const GET_IMPL

#define PDEF_SET(FULL_TYPE, name, SET_IMPL) \
	void set##name(FULL_TYPE value) SET_IMPL

#define PDEF_DO_SET(FULL_TYPE, name, SET_IMPL) \
	virtual void doSet##name(FULL_TYPE value) SET_IMPL

#define PDEF_EX_GETSET(FULL_TYPE, name) \
protected: \
	PDEF_DO_SET(FULL_TYPE, name, PDEF_NO_IMPL)  \
public: \
	PDEF_GET(inline FULL_TYPE, name, PDEF_GET_IMPL(name)) \
	PDEF_SET(FULL_TYPE, name, PDEF_SET_IMPL(name))

#define PDEF_DEFAULT_GETSET(FULL_TYPE, name) \
protected: \
	PDEF_DO_SET(FULL_TYPE, name, PDEF_DO_SET_PUSH_IMPL(name)) \
public: \
	PDEF_GET(inline FULL_TYPE, name, PDEF_GET_IMPL(name)) \
	PDEF_SET(FULL_TYPE, name, PDEF_SET_IMPL(name))

#define PDEF_FLD_EX_GETSET(FULL_TYPE, name) \
protected: \
	PDEF_DO_SET(FULL_TYPE, name, PDEF_NO_IMPL)  \
public: \
	PDEF_GET(inline FULL_TYPE, name, PDEF_GET_IMPL(name)) \
	PDEF_SET(FULL_TYPE, name, PDEF_SIMPLE_SET_IMPL(name))

#define PDEF_FLD_DEFAULT_GETSET(FULL_TYPE, name) \
protected: \
	PDEF_DO_SET(FULL_TYPE, name, PDEF_DO_SET_IMPL(name)) \
public: \
	PDEF_GET(inline FULL_TYPE, name, PDEF_GET_IMPL(name)) \
	PDEF_SET(FULL_TYPE, name, PDEF_SIMPLE_SET_IMPL(name))

#define PDEF_EX_GETSETRESET(FULL_TYPE, name) \
	PDEF_EX_GETSET(FULL_TYPE, name) \
	Q_INVOKABLE virtual void reset##name();

#define PDEF_DEFAULT_GETSETRESET(FULL_TYPE, name) \
	PDEF_DEFAULT_GETSET(FULL_TYPE, name) \
	Q_INVOKABLE virtual void reset##name();

#define SIMPLE_FIELD(Type, name) \
	CUSTOM_FIELD_DEF(Type, name, PDEF_FLD_DEFAULT_GETSET(Type, name))
#define REF_FIELD(Type, name) \
	CUSTOM_FIELD_DEF(Type, name, PDEF_FLD_DEFAULT_GETSET(const Type &, name))
#define CUSTOM_FIELD(Type, name) \
	CUSTOM_FIELD_DEF(Type, name, PDEF_FLD_EX_GETSET(Type, name))
#define CUSTOM_REF_FIELD(Type, name) \
	CUSTOM_FIELD_DEF(Type, name, PDEF_FLD_EX_GETSET(const Type &, name))

#define SIMPLE_PROPERTY(Type, name) \
	SIMPLE_PROPERTY_EX(Type, name, PDEF_NONE)
#define REF_PROPERTY(Type, name) \
	REF_PROPERTY_EX(Type, name, PDEF_NONE)
#define CUSTOM_PROPERTY(Type, name) \
	CUSTOM_PROPERTY_EX(Type, name, PDEF_NONE)
#define CUSTOM_REF_PROPERTY(Type, name) \
	CUSTOM_REF_PROPERTY_EX(Type, name, PDEF_NONE)

#define SIMPLE_PROPERTY_EX(Type, name, EXTRA) \
	CUSTOM_PROPERTY_DEF(Type, name, EXTRA, PDEF_DEFAULT_GETSETRESET(Type, name))
#define REF_PROPERTY_EX(Type, name, EXTRA) \
	CUSTOM_PROPERTY_DEF(Type, name, EXTRA, PDEF_DEFAULT_GETSETRESET(const Type &, name))
#define CUSTOM_PROPERTY_EX(Type, name, EXTRA) \
	CUSTOM_PROPERTY_DEF(Type, name, EXTRA, PDEF_EX_GETSETRESET(Type, name))
#define CUSTOM_REF_PROPERTY_EX(Type, name, EXTRA) \
	CUSTOM_PROPERTY_DEF(Type, name, EXTRA, PDEF_EX_GETSETRESET(const Type &, name))


#define CUSTOM_PROPERTY_DEF(Type, name, EXTRA, GET_SET) \
public: \
	Q_PROPERTY(Type m##name READ get##name WRITE set##name NOTIFY changed##name RESET reset##name EXTRA) \
	CUSTOM_FIELD_DEF(Type, name, GET_SET)

#define CUSTOM_FIELD_DEF(Type, name, GET_SET) \
protected: \
	Type m##name; \
	GET_SET

#define NEW_PROPERTY_VALUE_NO_COMMAND(Class, Name) \
{ \
	auto prototype = dynamic_cast<Class *>(this->getPrototype()); \
	if (nullptr == prototype || m##Name != prototype->get##Name()) \
		Banana::Object::setPropertyModified(New##Name, true); \
}

#define NEW_PROPERTY_VALUE(Class, Name, oldValue) \
{ \
	PUSH_UNDO_COMMAND(Name, oldValue); \
	NEW_PROPERTY_VALUE_NO_COMMAND(Class, Name); \
}

#define DO_SET_PROPERTY_VALUE_IMPL(Class, Type, Name) \
	void Class::doSet##Name(Type value) \
	{ \
		auto oldValue = m##Name; \
		m##Name = value; \
		NEW_PROPERTY_VALUE(Class, Name, oldValue); \
		emit changed##Name(); \
		setModified(true); \
	}

#define DO_RESET_PROPERTY_VALUE_IMPL(Class, Name) \
	void Class::reset##Name() \
	{ \
		DO_RESET_PROPERTY_VALUE_BODY(Class, Name); \
	}

#define PROP_VALUES_EQUAL(a, b) a == b
#define PROP_VALUES_CASE_INSENSITIVE_EQUAL(a, b) (0 == QString::compare(a, b, Qt::CaseInsensitive))

#define DO_RESET_PROPERTY_VALUE_BODY(Class, Name) \
	DO_RESET_PROPERTY_VALUE_BODY_EX(Class, Name, this->set##Name(DEFAULT_VALUE(Name)), PROP_VALUES_EQUAL)

#define DO_RESET_PROPERTY_STR_VALUE_BODY(Class, Name) \
	DO_RESET_PROPERTY_VALUE_BODY_EX(Class, Name, this->set##Name(DEFAULT_VALUE(Name)), PROP_VALUES_CASE_INSENSITIVE_EQUAL)

#define DO_RESET_PROPERTY_VALUE_BODY_EX(Class, Name, DEFAULT_RESET, EQUAL) \
	{ \
		auto prototype = dynamic_cast<Class *>(this->getPrototype()); \
		if (nullptr != prototype) \
		{ \
			this->set##Name(prototype->m##Name); \
			if (EQUAL(this->m##Name, prototype->m##Name)) \
			{ \
				if (Banana::Object::setPropertyModified(Class::New##Name, false)) \
					emit this->changed##Name(); \
			} \
		} else \
		{ \
			DEFAULT_RESET; \
			if (Banana::Object::setPropertyModified(Class::New##Name, false)) \
				emit this->changed##Name(); \
		} \
	}

#define ASSIGN_PROP_DUMMY(Prop)

#define ASSIGN_PROP_NO_RESET(Prop, source, EXTRA) \
{ \
	if (this->canAssignPropertyFrom(source, New##Prop)) \
	{ \
		this->set##Prop(source->m##Prop); \
		if (source == this->prototype) \
			Banana::Object::setPropertyModified(New##Prop, false); \
	} \
	EXTRA(Prop) \
}

#define ASSIGN_PROP_WITH_RESET(Prop, source, EXTRA) \
{ \
	if (nullptr != source) \
	{ \
		ASSIGN_PROP_NO_RESET(Prop, source, EXTRA); \
	} else \
	if (this->isLoading() || !Banana::Object::isPropertyModified(New##Prop)) \
		this->reset##Prop(); \
}

#define ASSIGN_PROP(Prop, source) ASSIGN_PROP_WITH_RESET(Prop, source, ASSIGN_PROP_DUMMY)

#define ASSIGN_PROP_OLD(Prop) \
	else \
		std::swap(this->m##Prop, old##Prop);

#define CONNECT_PROTO_PROP(Class, proto, Prop) \
	QObject::connect(proto, &Class::changed##Prop, \
				 this, &Class::onPrototype##Prop##Changed);

#define DISCONNECT_PROTO_PROP(Class, proto, Prop) \
	QObject::disconnect(proto, &Class::changed##Prop, \
				 this, &Class::onPrototype##Prop##Changed);

#define PROTO_PROP_CHANGED_IMPL(Class, Prop) \
void Class::onPrototype##Prop##Changed() \
{ \
	if (!Banana::Object::isPropertyModified(New##Prop)) \
	{ \
		this->reset##Prop(); \
	} \
}

#define PROTOTYPE_CONNECTION_BODY(ACTION, Class, FilePath) \
{ \
	auto proto = dynamic_cast<Class *>(prototype); \
	if (nullptr != proto) \
	{ \
		ACTION##_PROTO_PROP(Class, proto, FilePath); \
	} \
}

#define PROTOTYPE_CONNECTION_IMPL(Class, FilePath) \
void Class::doConnectPrototype() \
{ \
	Inherited::doConnectPrototype(); \
	PROTOTYPE_CONNECTION_BODY(CONNECT, Class, FilePath) \
} \
void Class::doDisconnectPrototype() \
{ \
	PROTOTYPE_CONNECTION_BODY(DISCONNECT, Class, FilePath) \
	Inherited::doDisconnectPrototype(); \
}

#define EXPANDED_PROPERTIES_BEGIN() \
	static Banana::ExpandedSet initExpandedProperties() \
	{ \
		Banana::ExpandedSet result

#define INIT_EXPANDED_PROPERTY(Name) \
	result.insert(PROPERTY_INDEX(Name))

#define EXPANDED_PROPERTIES_END() \
	return result; \
}	 \
static const Banana::ExpandedSet sExpandedProperties = initExpandedProperties()

#define MAP_PROP_FLAGS_BEGIN() \
	static Banana::PropertyFlagsMap initPropertyFlagsMap() \
	{ \
		Banana::PropertyFlagsMap result

#define MAP_PROP_FLAGS_CUSTOM(Name, flags) \
	result[PROPERTY_INDEX(Name)] = flags

#define MAP_PROP_FLAGS(Class, Name) \
	MAP_PROP_FLAGS_CUSTOM(Name, MOD_FLAG(Class, Name))

#define MAP_PROP_FLAGS_END() \
	return result; \
} \
static const Banana::PropertyFlagsMap sPropertyFlagsMap = initPropertyFlagsMap()

#define PROPERTY_INDEX(Name) \
	s##Name##Index

#define DEFAULT_VALUE(Name) \
	s##Name##Default

#define CONSTRUCT_PROPERTY(Name) \
	m##Name(s##Name##Default)

#define DECLARE_PROPERTY_INDEX(Name) \
	static const int PROPERTY_INDEX(Name)

#define DECLARE_DEFAULT_VALUE(Name, Type) \
	static const Type DEFAULT_VALUE(Name)

#define GET_PROPERTY_INDEX(Class, Name) \
	Class::staticMetaObject.indexOfProperty(MPROP(Name))

#define INIT_PROPERTY_INDEX_CL(Class, Name) \
	const int Class::PROPERTY_INDEX(Name) = GET_PROPERTY_INDEX(Class, Name)

#define INIT_PROPERTY_INDEX(Class, Name) \
	DECLARE_PROPERTY_INDEX(Name) = GET_PROPERTY_INDEX(Class, Name)

#define INIT_DEFAULT_VALUE_CL(Class, Name, Type, ...) \
const Type Class::DEFAULT_VALUE(Name)(__VA_ARGS__)

#define INIT_DEFAULT_VALUE(Name, Type, ...) \
	DECLARE_DEFAULT_VALUE(Name, Type)(__VA_ARGS__)

#define INIT_PROPERTY(Class, Name, Type, ...) \
	INIT_PROPERTY_INDEX(Class, Name); \
	INIT_DEFAULT_VALUE(Name, Type, __VA_ARGS__)

#define INIT_PROPERTY_CL(Class, Name, Type, ...) \
	INIT_PROPERTY_INDEX_CL(Class, Name); \
	INIT_DEFAULT_VALUE_CL(Name, Type, __VA_ARGS__)

#define DECLARE_PROPERTY(Name, Type) \
	DECLARE_PROPERTY_INDEX(Name); \
	DECLARE_DEFAULT_VALUE(Name, Type)

#define MOD_FLAG(Class, Name) \
	(1ULL << Class::New##Name)

#define IMPL_CHECK_MODIFIED_FLAGS(metaProperty) \
if (metaProperty.isResettable()) \
{ \
	auto flags = Banana::findPropertyFlags(metaProperty, sPropertyFlagsMap); \
	if (0 != flags) \
	{ \
		if (0 != (Banana::Object::getPropertyModifiedBits() & flags)) \
			result |= QtnPropertyStateModifiedValue; \
	} \
} \
do {} while (false)


#include <QVariant>

#include <set>
#include <map>

class QMetaProperty;

namespace Banana
{

	typedef std::set<int> ExpandedSet;

	typedef std::map<int, quint64> PropertyFlagsMap;

	quint64 findPropertyFlags(const QMetaProperty &metaProperty, const PropertyFlagsMap &in);
}
