function testQDir()
{
	assert(typeof QDir.listSeparator === "string");
	assert(typeof QDir.separator === "string");
	assert(typeof QDir.currentPath === "string");
	assert(typeof QDir.homePath === "string");
	assert(typeof QDir.rootPath === "string");
	assert(typeof QDir.tempPath === "string");
	assert(QDir.prototype.path === "");
	assert(QDir.prototype.absolutePath === "");
	assert(QDir.prototype.canonicalPath === "");
	assert(QDir.prototype.dirName === "");
	assert(QDir.prototype.readable === false);
	assert(QDir.prototype.relative === true);
	assert(QDir.prototype.absolute === false);
	assert(QDir.prototype.isRoot === false);
	assert(!QDir.prototype.exists());
	assert(!QDir.prototype.exists(""));
	assert(!QDir.prototype.mkdir(""));
	assert(!QDir.prototype.rmdir(""));
	assert(!QDir.prototype.mkpath(""));
	assert(!QDir.prototype.rmpath(""));
	assert(!QDir.prototype.removeRecursively());
	assert(!QDir.prototype.makeAbsolute());
	assert(!QDir.prototype.cd(""));
	assert(!QDir.prototype.cdUp());
	assert(!QDir.prototype.remove(""));
	assert(!QDir.prototype.rename("", ""));
	assert(!QDir.prototype.equals(""));
	assert(QDir.prototype.entryList(
			   [], QDir.NoFilter, QDir.NoSort) === undefined);
	assert(QDir.prototype.filePath("") === "");
	assert(QDir.prototype.absoluteFilePath("") === "");
	assert(QDir.prototype.relativeFilePath("") === "");
	assert(QDir.prototype.toString() === "QDir");
	QDir.prototype.refresh();

	var currentPath = QDir.currentPath;
	var nativeCurrentPath = QDir.toNativeSeparators(currentPath);
	assert(QDir.separator === '/' || currentPath !== nativeCurrentPath);
	assert(currentPath.indexOf('/') >= 0);
	assert(nativeCurrentPath.indexOf(QDir.separator) >= 0);
	assert(QDir.fromNativeSeparators(nativeCurrentPath) === currentPath);
	print("List separator: '%1'", QDir.listSeparator);
	print("Native separator: '%1'", QDir.separator);
	print("Root path: %1", QDir.toNativeSeparators(QDir.rootPath));
	print("Current path: %1", nativeCurrentPath);
	assert(QDir.current() instanceof QDir);
	assert(QDir.home() instanceof QDir);
	assert(QDir.root() instanceof QDir);
	assert(QDir.temp() instanceof QDir);

	var dir = new QDir();
	assert(dir instanceof QDir);
	assert(dir.equals(QDir.current()));
	assert(dir.readable);
	assert(dir.relative);
	assert(!dir.absolute);
	assert(dir.path === '.');
	assert(dir.absolutePath.indexOf(currentPath) === 0);
	assert(dir.canonicalPath !== '.');
	var temp = false;
	if (new QFileInfo(dir.absolutePath).isRoot)
	{
		dir = QDir.temp();
		currentPath = dir.path;
		temp = true;
	}
	print("Absolute current path: %1", QDir.toNativeSeparators(dir.absolutePath));
	print("Canonical current path: %1", QDir.toNativeSeparators(dir.canonicalPath));
	print("Home path: %1", QDir.toNativeSeparators(QDir.homePath));
	print("Temp path: %1", QDir.toNativeSeparators(QDir.tempPath));
	assert(dir.toString() === dir.path);
	assert(!dir.isRoot);
	assert(dir.cdUp());
	assert(dir.absolute);
	assert(dir.makeAbsolute());
	assert(dir.path === dir.absolutePath);
	assert(!dir.relative);
	assert(dir.dirName !== dir.path);
	var dir2 = new QDir();
	assert(dir2.makeAbsolute());
	assert(dir2.isRoot || dir2.path.indexOf(dir.path) === 0);
	dir2.path = [dir2.path, "..", dir2.dirName].join(QDir.separator);
	assert(dir2.absolute);
	assert(dir2.path !== dir2.absolutePath);
	assert(temp || QDir.cleanPath(dir2.path) === currentPath);

	var tempDirName = "ScriptTempDir";
	var tempFileName = "ScriptTempFile";
	dir = QDir.temp();
	assert(dir.toString() === dir.absolutePath);
	dir.equals(dir.absolutePath);
	assert(!dir.exists(tempFileName) || dir.remove(tempFileName));
	dir2.path = dir.filePath(tempDirName);
	assert(dir2.removeRecursively());
	assert(dir.mkdir(tempDirName));
	assert(dir.exists(tempDirName));
	assert(QDir.exists(dir2.path));
	assert(!dir.remove(tempDirName));
	assert(dir.rmdir(tempDirName));
	assert(!dir.exists(tempDirName));
	assert(!QDir.exists(dir2.path));

	var file = new QFile(dir.filePath(tempFileName));
	assert(file.open(QIODevice.WriteOnly));
	assert(file.flush());
	file.close();

	var dir2Name = "dir2";
	var tempPath = [tempDirName, dir2Name].join(QDir.separator);
	assert(QDir.toNativeSeparators(
			   dir.relativeFilePath(dir.filePath(tempPath))) === tempPath);
	assert(dir.mkpath(tempPath));
	dir2.refresh();
	assert(dir2.exists());
	assert(QDir.exists(dir2.filePath(dir2Name)));
	assert(!dir.rmdir(tempDirName));
	assert(dir2.removeRecursively());
	assert(!dir2.exists());
	assert(!QDir.exists(dir2.filePath(dir2Name)));
	assert(dir.mkpath(tempPath));
	var dir2Ren = dir2Name + "_ren";
	assert(dir2.rename(dir2Name, dir2Ren));
	assert(!dir2.exists(dir2Name));
	assert(dir2.exists(dir2Ren));
	var filePath2 = dir2.filePath(tempFileName);
	assert(file.copy(filePath2));
	assert(QFile.exists(filePath2));
	tempPath = [tempDirName, dir2Ren].join(QDir.separator);
	assert(dir2.exists(filePath2));
	assert(dir2.remove(filePath2));
	assert(dir.rmpath(tempPath));
	assert(!dir2.exists());
	assert(!QDir.exists(dir2.filePath(dir2Ren)));
	assert(dir.mkpath(tempPath));
	assert(QDir.remove(dir2.path));
	assert(!dir2.exists());
	assert(dir.mkpath(tempPath));
	assert(QDir.rename(dir2.absoluteFilePath(dir2Ren),
					   dir2.absoluteFilePath(dir2Name)));
	assert(dir2.exists(dir2Name));
	assert(!dir2.exists(dir2Ren));
	assert(dir2.removeRecursively());
	assert(!dir2.exists());

	assert(file.remove());

	print("QDir OK");
}
