/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2017 Alexandra Cherdantseva

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

#include "BananaCore/AbstractProjectFile.h"

namespace Banana
{
class ScriptManager;
class ScriptableProjectFile : public AbstractProjectFile
{
	Q_OBJECT

	Q_PROPERTY(Banana::ScriptManager *mScriptManager READ getScriptManager RESET
			resetScriptManager DESIGNABLE true STORED false)

	ScriptManager *mScriptManager;

protected:
	static const QString SCRIPTS_KEY;
	static const QString CAPTION_KEY;
	static const QString SHORTCUT_KEY;
	static const QString OBJECT_TYPE_KEY;

public:
	explicit ScriptableProjectFile(
		const QString &name, const QString &extension);
	virtual ~ScriptableProjectFile() override;

	inline ScriptManager *getScriptManager() const;
	Q_INVOKABLE void resetScriptManager();

protected:
	virtual void saveData(QVariantMap &output) override;
	virtual bool loadData(const QVariantMap &input) override;

private:
	bool loadScriptEntries(const QVariantMap &input);
	void saveScriptEntries(QVariantMap &output);
};

ScriptManager *ScriptableProjectFile::getScriptManager() const
{
	return mScriptManager;
}
}
