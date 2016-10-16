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

