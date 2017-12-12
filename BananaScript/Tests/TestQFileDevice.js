importScript("TestQIODevice.js")

function testQFileDevice()
{
	assert(QFileDevice.prototype instanceof QIODevice);
	assert(QFileDevice.prototype.error === QFileDevice.UnspecifiedError);
	assert(QFileDevice.prototype.permissions === 0);
	assert(QFileDevice.prototype.size === -1);
	assert(QFileDevice.prototype.toString() === "QFileDevice");
	QFileDevice.prototype.resize(1);
	QFileDevice.prototype.flush();
	QFileDevice.prototype.unsetError();

	assert(shouldThrow(function() { return new QFileDevice(); }));
	assert(isFinite(QFileDevice.ReadOwner));
	assert(isFinite(QFileDevice.ReadUser));
	assert(isFinite(QFileDevice.ReadGroup));
	assert(isFinite(QFileDevice.ReadOther));
	assert(isFinite(QFileDevice.WriteOwner));
	assert(isFinite(QFileDevice.WriteUser));
	assert(isFinite(QFileDevice.WriteGroup));
	assert(isFinite(QFileDevice.WriteOther));
	assert(isFinite(QFileDevice.ExeOwner));
	assert(isFinite(QFileDevice.ExeUser));
	assert(isFinite(QFileDevice.ExeGroup));
	assert(isFinite(QFileDevice.ExeOther));

	assert(isFinite(QFileDevice.NoError));
	assert(isFinite(QFileDevice.ReadError));
	assert(isFinite(QFileDevice.WriteError));
	assert(isFinite(QFileDevice.FatalError));
	assert(isFinite(QFileDevice.ResourceError));
	assert(isFinite(QFileDevice.OpenError));
	assert(isFinite(QFileDevice.AbortError));
	assert(isFinite(QFileDevice.TimeOutError));
	assert(isFinite(QFileDevice.UnspecifiedError));
	assert(isFinite(QFileDevice.RemoveError));
	assert(isFinite(QFileDevice.RenameError));
	assert(isFinite(QFileDevice.PositionError));
	assert(isFinite(QFileDevice.ResizeError));
	assert(isFinite(QFileDevice.PermissionsError));
	assert(isFinite(QFileDevice.CopyError));

	print("QFileDevice OK");
}

function testFileError(file)
{
	if (file.error !== QFileDevice.NoError)
		print(file.errorString);
	assert(file.error === QFileDevice.NoError);
}

function testFile(cls, mode, filePath)
{
	function File(filePath)
	{
		if (filePath === undefined)
			cls.call(this);
		else
			cls.call(this, filePath);

		print("%1 constructed", cls.prototype);
	}

	File.prototype = cls.prototype;
	var file = new File(filePath);
	assert(file.open(mode));
	assert(file.toString() === file.filePath);
	assert(file instanceof File);
	assert(file instanceof QFileDevice);
	if (0 !== (mode & QIODevice.WriteOnly))
		testWritableDevice(file);
	if (0 !== (mode & QIODevice.ReadOnly))
		testReadableDevice(file);
	print(file.permissions);
	file.permissions =
			QFileDevice.ReadOwner | QFileDevice.ReadUser |
			QFileDevice.WriteOwner | QFileDevice.WriteUser;
	if (!(file instanceof QSaveFile))
	{
		assert(file.exists());
	}

	var catcher = { closed: false };
	var onAboutToClose = function()
	{
		print("%1 is about to close", cls.prototype);
		this.closed = true;
	};

	file.aboutToClose.connect(catcher, onAboutToClose);

	assert(file.flush());
	testFileError(file);

	if (!(file instanceof QSaveFile))
	{
		file.close();
		testFileError(file);
		assert(!file.isOpen);
		assert(catcher.closed);
	}

	return file;
}
