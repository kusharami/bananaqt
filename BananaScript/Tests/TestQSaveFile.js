importScript("TestQFileDevice.js");

function testQSaveFile()
{
	assert(QSaveFile.prototype instanceof QFileDevice);
	assert(QSaveFile.prototype instanceof QIODevice);
	assert(QSaveFile.prototype.filePath === "");
	assert(QSaveFile.prototype.directWriteFallback === false);
	assert(!QSaveFile.prototype.commit());
	assert(QSaveFile.prototype.toString() === "QSaveFile");
	QSaveFile.prototype.cancelWriting();

	var workDir = QDir.current();
	if (workDir.isRoot)
		workDir = QDir.temp();
	var filePath = workDir.filePath("QSaveFile.test");
	QFile.remove(filePath);
	var file = testFile(QSaveFile, QIODevice.WriteOnly,	filePath);
	assert(file instanceof QSaveFile);
	assert(file instanceof QFileDevice);
	print(file.filePath);
	assert(!file.directWriteFallback);
	assert(!QFile.exists(file.filePath));
	assert(file.commit());
	assert(QFile.exists(file.filePath));
	assert(QFile.remove(file.filePath));

	file = new QSaveFile(filePath);
	file.directWriteFallback = true;
	assert(file.open(QIODevice.WriteOnly));
	assert(file.commit());
	assert(QFile.exists(file.filePath));
	assert(QFile.remove(file.filePath));

	file = new QSaveFile(filePath);
	assert(file.open(QIODevice.WriteOnly));
	file.cancelWriting();
	assert(file.error !== QFileDevice.NoError);
	print(file.errorString);
	assert(!file.commit());
	assert(!QFile.exists(file.filePath));
}
