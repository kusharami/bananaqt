#pragma once

#include <QScriptValue>

namespace Utils
{
	QScriptValue VariantToScriptValue(const QVariant &variant, QScriptEngine *engine);
}
