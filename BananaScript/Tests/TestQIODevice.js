function shouldThrow(cls)
{
	try
	{
		var d = cls();
	} catch (e)
	{
		return true;
	}

	return false;
}

function testQIODevice()
{
	assert(QIODevice.prototype.bytesAvailable === -1);
	assert(QIODevice.prototype.bytesToWrite === -1);
	assert(QIODevice.prototype.errorString === "");
	assert(QIODevice.prototype.isTextModeEnabled === false);
	assert(QIODevice.prototype.isOpen === false);
	assert(QIODevice.prototype.sequential === false);
	assert(QIODevice.prototype.readable === false);
	assert(QIODevice.prototype.writable === false);
	assert(QIODevice.prototype.atEnd === false);
	assert(QIODevice.prototype.isTransactionStarted === false);
	assert(QIODevice.prototype.pos === -1);
	assert(QIODevice.prototype.openMode === QIODevice.NotOpen);
	assert(!QIODevice.prototype.open(0xFFFFFFFF));
	assert(!QIODevice.prototype.seek(0));
	assert(!QIODevice.prototype.reset());
	assert(!QIODevice.prototype.canReadLine());
	assert(QIODevice.prototype.peek(1).equals(""));
	assert(QIODevice.prototype.read(1).equals(""));
	assert(QIODevice.prototype.readAll().equals(""));
	assert(QIODevice.prototype.readLine(1).equals(""));
	assert(QIODevice.prototype.write("b") === -1);
	assert(QIODevice.prototype.toString() === "QIODevice");
	QIODevice.prototype.close();
	QIODevice.prototype.startTransaction();
	QIODevice.prototype.rollbackTransaction();
	QIODevice.prototype.commitTransaction();

	assert(shouldThrow(function() { return new QIODevice(); }));
	assert(isFinite(QIODevice.NotOpen));
	assert(isFinite(QIODevice.ReadOnly));
	assert(isFinite(QIODevice.WriteOnly));
	assert(isFinite(QIODevice.ReadWrite));
	assert(isFinite(QIODevice.Append));
	assert(isFinite(QIODevice.Truncate));
	assert(isFinite(QIODevice.Text));
	assert(isFinite(QIODevice.Unbuffered));

	print("QIODevice OK");
}

function testWritableDevice(device)
{
	assert(device instanceof QIODevice);

	assert(device.isOpen || device.open(QIODevice.WriteOnly));
	assert(device.isOpen);
	assert(device.writable);

	assert(0 !== (device.openMode & QIODevice.WriteOnly));
	assert(device.pos === 0);
	assert(device.size === 0);
	assert(device.sequential || device.atEnd);
	assert(!device.isTransactionStarted);

	var bytes = new QByteArray([0, 1, 2, 3, 4, 5, 6]);
	assert(device.write(bytes) === bytes.length);
	assert(device.seqential || device.size === bytes.length);
	assert(device.seqential || device.pos === device.size);
	assert(device.atEnd);
	assert(device.seek(0) === !device.sequential);
	assert(device.sequential || device.pos === 0);
	assert(!device.atEnd);
	assert(device.seek(bytes.length));
	assert(device.atEnd);
	assert(device.reset());
	assert(!device.atEnd);
	assert(shouldThrow(function() { return device.pos = -1; }));
}

function testReadableDevice(device, size)
{
	assert(device instanceof QIODevice);

	assert(device.isOpen || device.open(QIODevice.ReadOnly));
	assert(device.isOpen);
	assert(device.readable);

	assert(0 !== (device.openMode & QIODevice.ReadOnly));
	assert(device.sequential || device.size === 0  || !device.atEnd);
	assert(device.pos === 0);

	assert(!device.isTransactionStarted);
	device.startTransaction();
	assert(device.isTransactionStarted);
	assert(device.seek(1) === !device.sequential);
	assert(device.sequential || device.pos === 1);
	if (device.sequential)
		assert(shouldThrow(function() { return device.pos = 0; }));
	else
		device.pos = 0;
	assert(device.readAll().length === device.size);
	assert(device.sequential || device.pos === device.size);
	device.rollbackTransaction();
	assert(!device.isTransactionStarted);
	assert(device.pos === 0);
	assert(shouldThrow(function() { return device.pos = -1; }));
}
