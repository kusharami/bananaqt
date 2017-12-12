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

	file = new QTemporaryFile("ScriptTemp");
	assert(file.fileTemplate === "ScriptTemp");
	file.autoRemove = false;
	assert(file.open());
	print(file.filePath);
	assert(file.filePath.indexOf("ScriptTemp") >= 0);
	assert(!file.autoRemove);
	file.close();
	testFileError(file);

	assert(file.exists());
	assert(QFile.exists(file.filePath));
	assert(file.remove());

	print("QTemporaryFile OK");
}
