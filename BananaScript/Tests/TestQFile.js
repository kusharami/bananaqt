importScript("TestQFileDevice.js");

function testQFile()
{
	assert(QFile.prototype instanceof QFileDevice);
	assert(QFile.prototype instanceof QIODevice);
	assert(QFile.prototype.filePath === "");
	assert(QFile.prototype.symLinkTarget === "");
	assert(!QFile.prototype.exists());
	assert(!QFile.prototype.remove());
	assert(!QFile.prototype.rename(""));
	assert(!QFile.prototype.copy(""));
	assert(!QFile.prototype.link(""));
	assert(QFile.prototype.toString() === "QFile");

	var filePath = "QFile.test";
	QFile.remove(filePath);
	var file = testFile(QFile, QIODevice.WriteOnly | QIODevice.Truncate,
						filePath);
	print(file.filePath);
	assert(file instanceof QFile);
	assert(file.exists());
	assert(QFile.exists(file.filePath));
	assert(file.remove());

	var filePathRen = file.filePath + "_ren";
	var filePathLink = file.filePath + "_link";
	QFile.remove(filePathRen);
	QFile.remove(filePathLink);

	file = new QFile();
	assert(!file.sequential);
	assert(!file.isOpen);
	assert(!file.readable);
	assert(!file.writable);
	assert(!file.isTextModeEnabled);
	assert(file.atEnd);
	assert(!file.isTransactionStarted);

	var openMode = QIODevice.WriteOnly;

	file.filePath = filePath;
	assert(file.open(openMode));
	assert(file.openMode === openMode);
	assert(file.writable);
	assert(!file.readable);
	assert(!file.isTextModeEnabled);
	assert(file.size === 0);
	assert(file.atEnd);

	var bytes = new QByteArray([0, 1, 2, 3, 4, 5, 6]);
	assert(file.write(bytes) === bytes.length);
	assert(file.exists());
	assert(!QFile.remove(file.filePath));
	assert(file.copy(filePathRen));
	assert(!file.rename(filePathRen));
	assert(file.error === QFileDevice.RenameError);
	print(file.errorString);
	file.unsetError();
	assert(QFile.remove(filePathRen));
	assert(file.flush());
	file.close();
	testFileError(file);

	assert(file.rename(filePathRen));
	assert(!QFile.exists(filePath));
	assert(file.exists());
	assert(QFile.exists(filePathRen));
	assert(file.filePath === filePathRen);
	assert(file.link(filePathLink));
	var file2 = new QFile(filePathLink);
	assert(file2.symLinkTarget === filePathRen);
	assert(file2.exists());
	assert(QFile.exists(filePathLink));
	assert(QFile.remove(filePathLink));
	assert(!file2.exists());

	openMode = QIODevice.ReadOnly;
	assert(file.open(openMode));
	assert(file.openMode === openMode);
	testReadableDevice(file);
	assert(!file.writable);
	assert(!file.isTextModeEnabled);
	assert(file.size === bytes.length);
	assert(file.pos === 0);
	assert(!file.atEnd);
	assert(file.peek(1)[0] === bytes[0]);
	assert(file.pos === 0);
	assert(file.readAll().equals(bytes));
	assert(file.atEnd);
	assert(file.pos === bytes.length);
	assert(file.seek(0));
	assert(file.pos === 0);
	assert(file.read(2)[1] === bytes[1]);
	assert(!file.atEnd);
	assert(file.pos === 2);
	assert(file.reset());
	assert(file.pos === 0);
	print("File contents: %1", file.readLine().toHex().toStringLatin());
	assert(!file.readLine().length);

	file.close();
	assert(!file.isOpen);
	assert(file.remove());
	assert(!file.exists());
	assert(!QFile.exists(filePathRen));

	var textFilePath = "QFile.txt";

	file.filePath = textFilePath;
	var text = "Text to write\n";
	var textBytes = new QByteArray(text, UTF8_CODEC);

	openMode = QIODevice.WriteOnly | QIODevice.Truncate | QIODevice.Text;
	assert(file.open(openMode));
	assert(file.openMode === openMode);
	assert(file.writable);
	assert(!file.readable);
	assert(file.isTextModeEnabled);
	assert(file.size === 0);
	assert(file.pos === 0);
	assert(file.atEnd);
	assert(file.write(textBytes) >= textBytes.length);
	assert(file.atEnd);
	assert(file.pos >= textBytes.length);
	assert(file.size >= textBytes.length);
	assert(file.seek(0));
	assert(!file.atEnd);

	file.close();
	assert(!file.isOpen);

	openMode = QIODevice.WriteOnly | QIODevice.Append | QIODevice.Text;
	assert(file.open(openMode));
	assert(file.openMode === openMode);
	assert(file.writable);
	assert(!file.readable);
	assert(file.isTextModeEnabled);
	assert(file.size >= textBytes.length);
	assert(file.pos === file.size);
	assert(file.atEnd);
	assert(file.write(textBytes) >= textBytes.length);
	assert(file.atEnd);
	assert(file.pos >= textBytes.length * 2);
	assert(file.size >= textBytes.length * 2);
	assert(file.seek(0));
	assert(!file.atEnd);
	file.close();
	assert(!file.isOpen);

	openMode = QIODevice.ReadWrite | QIODevice.Text;
	assert(file.open(openMode));
	assert(file.openMode === openMode);
	assert(file.writable);
	assert(file.readable);
	assert(file.isTextModeEnabled);
	assert(file.size >= textBytes.length * 2);
	assert(file.pos === 0);
	assert(!file.atEnd);
	assert(file.readLine().toString(UTF8_CODEC) === text);
	assert(file.readLine().toString(UTF8_CODEC) === text);
	assert(!file.readLine().length);
	assert(file.write(textBytes) >= textBytes.length);
	assert(file.atEnd);
	assert(file.pos >= textBytes.length * 3);
	assert(file.size >= textBytes.length * 3);
	assert(file.seek(0));
	assert(!file.atEnd);
	file.close();
	assert(!file.isOpen);

	var error = false;
	try
	{
		file.size = -1;
	} catch(e)
	{
		print(e);
		error = true;
	}

	assert(error);

	assert(file.remove());

	print("QFile OK");
}
