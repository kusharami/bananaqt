#pragma once

#define CSTRKEY(key) #key
#define WSTRKEY(key) QT_UNICODE_LITERAL(CSTRKEY(key))
#define QSTRKEY(key) QStringLiteral(#key)
