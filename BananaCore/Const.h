#pragma once

class QFont;
class QString;

namespace Core
{
	extern const char pX[];
	extern const char pY[];
	extern const char pWidth[];
	extern const char pHeight[];
	extern const char pNoExtension[];
	extern const char pUntitledFileName[];
	extern const char pFalse[];
	extern const char pTrue[];

	QFont getDefaultFont();
	QString getBoolString(bool value);

	enum
	{
		CHANGE_VALUE_COMMAND,
		CHANGE_CONTENTS_COMMAND,
		CHILD_ACTION_COMMAND,

		USER_COMMAND
	};
}
