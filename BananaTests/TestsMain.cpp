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

#include "TestsMain.h"

#include <QCoreApplication>
#include <QDebug>

#include <vector>

static std::vector<TestCreator> testCreators;

size_t registerTestCreator(const TestCreator &create)
{
	auto result = testCreators.size();
	testCreators.push_back(create);
	return result;
}

static int executeTests(int argc, char **argv)
{
	int status = 0;
	for (auto &create : testCreators)
	{
		auto testObject = create();
		status |= QTest::qExec(testObject, argc, argv);
		delete testObject;
	}
	return status;
}

int main(int argc, char *argv[])
{
	qInfo() << "Starting tests...";
	QCoreApplication app(argc, argv);
	Q_UNUSED(app);
	QTEST_SET_MAIN_SOURCE_PATH
	return executeTests(argc, argv);
}

