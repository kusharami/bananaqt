#pragma once

#include <QScriptValue>

namespace Banana
{

namespace Utils
{
	QScriptValue VariantToScriptValue(const QVariant &variant, QScriptEngine *engine);
}
}
