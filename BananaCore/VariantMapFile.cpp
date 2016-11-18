/*
 * Banana Qt Libraries
 *
 * Copyright (c) 2016 Alexandra Cherdantseva
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "VariantMapFile.h"

#include "Utils.h"
#include "Object.h"

namespace Banana
{

	VariantMapFile::VariantMapFile(const QString &extension)
		: AbstractFile(extension)
	{

	}

	bool VariantMapFile::doSave(QIODevice *device)
	{
		QVariantMap data;
		saveData(data);

		return Utils::SaveVariantMapToIODevice(data, device);
	}

	bool VariantMapFile::doLoad(QIODevice *device)
	{
		QVariantMap data;
		if (Utils::LoadVariantMapFromIODevice(data, device))
			return loadData(data);

		return false;
	}

	void VariantMapFile::saveData(QVariantMap &output)
	{
		auto data = doGetData();
		if (nullptr != data)
		{
			auto obj = dynamic_cast<Object *>(data);
			if (nullptr != obj)
				obj->saveContents(output);
			else
				Object::saveContents(data, output);
		}
	}

	bool VariantMapFile::loadData(const QVariantMap &input)
	{
		beginLoad();
		bool ok = false;
		auto data = doGetData();
		if (nullptr != data)
		{
			auto obj = dynamic_cast<Object *>(data);
			if (nullptr != obj)
				ok = obj->loadContents(input, true);
			else
				ok = Object::loadContents(input, data, true);
		}

		endLoad();
		return ok;
	}

}
