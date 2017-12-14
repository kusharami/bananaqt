importScript("TestQFileDevice.js")

function testQTemporaryFile()
{
	assert(QTemporaryFile.prototype instanceof QFile);
	assert(QTemporaryFile.prototype instanceof QFileDevice);
	assert(QTemporaryFile.prototype instanceof QIODevice);
	assert(QTemporaryFile.prototype.autoRemove === false);
	assert(QTemporaryFile.prototype.fileTemplate === "");
	assert(!QTemporaryFile.prototype.open());
	assert(QTemporaryFile.prototype.toString() === "QTemporaryFile");

	var file = testFile(QTemporaryFile, QIODevice.ReadWrite);
	assert(file instanceof QTemporaryFile);
	assert(file instanceof QFile);
	print(file.fileTemplate);
	assert(file.autoRemove);

	assert(!file.exists());

	var baseName = "ScriptTemp";
	file = new QTemporaryFile(QDir.temp().filePath(baseName));
	assert(file.fileTemplate.indexOf(baseName) >= 0);
	file.autoRemove = false;
	assert(file.open());
	print(file.filePath);
	assert(file.filePath.indexOf(baseName) >= 0);
	assert(!file.autoRemove);
	file.close();
	testFileError(file);

	assert(file.exists());
	assert(QFile.exists(file.filePath));
	assert(file.remove());

	print("QTemporaryFile OK");
}
