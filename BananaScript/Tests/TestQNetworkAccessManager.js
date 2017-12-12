importScript("TestQIODevice.js")
importScript("TestQNetworkRequest.js")
importScript("TestQNetworkReply.js")

var JSON_HEADER = "application/json";

function TestNetworkSignals()
{
	var manager = new QNetworkAccessManager();
	assert(manager instanceof QNetworkAccessManager);
	assert(manager.toString() === "QNetworkAccessManager");
	this.manager = manager;
	this.finished = false;
}

TestNetworkSignals.prototype.wait = function()
{
	while (!this.finished) {}
	this.finished = false;
}

TestNetworkSignals.prototype.connect = function()
{
	assert(this.manager.networkAccessible === QNetworkAccessManager.Accessible);
	this.manager.finished.connect(this, this.onFinished);
}

TestNetworkSignals.prototype.disconnect = function()
{
	this.manager.finished.disconnect(this, this.onFinished);
}

function onDownloadProgress(bytesReceived, bytesTotal)
{
	print("Downloaded %1 of %2", bytesReceived,
		  Math.max(bytesReceived, bytesTotal));
}

function onUploadProgress(bytesSent, bytesTotal)
{
	if (bytesTotal === 0)
		return;

	print("Uploaded %1 of %2", bytesSent,
		  Math.max(bytesSent, bytesTotal));
}

function onRedirected(url)
{
	print("Redirected to '%1'", url);
}

function connectNetworkReply(reply)
{
	reply.downloadProgress.connect(onDownloadProgress);
	reply.uploadProgress.connect(onUploadProgress);
	reply.redirected.connect(onRedirected);
}

TestNetworkSignals.prototype.onFinished = function()
{
	this.finished = true;
}

function resolveJsonReply(reply)
{
	assert(reply.header(QNetworkRequest.ContentTypeHeader) === JSON_HEADER);
	var data = JSON.parse(reply.readAll().toString("UTF-8"));
	assert(data instanceof Object);
	return data;
}

function testHeadRequest(signals)
{
	var manager = signals.manager;
	signals.connect();

	var reply = manager.head("https://httpbin.org/get");
	testReply(reply);
	connectNetworkReply(reply);

	signals.wait();

	testReadableDevice(reply);

	var rawHeaders = reply.rawHeaderList();

	for (var i in rawHeaders)
	{
		var header = rawHeaders[i];
		print("%1: %2",
			  header.toString("UTF-8"),
			  reply.rawHeader(header).toString("UTF-8"));
	}

	signals.disconnect();
}

function testGetRequest(signals)
{
	var manager = signals.manager;
	signals.connect();

	var request = new QNetworkRequest("https://github.io");
	request.setAttribute(QNetworkRequest.FollowRedirectsAttribute, true);
	var reply = manager.get(request);
	testReply(reply);
	connectNetworkReply(reply);

	signals.wait();

	testReadableDevice(reply);
	var size = reply.bytesAvailable;
	var file = new QTemporaryFile();
	assert(file.open());
	assert(file.write(reply.readAll()) === size);
	assert(reply.bytesAvailable === 0);

	signals.disconnect();
}

function testPostRequest(signals)
{
	var manager = signals.manager;
	signals.connect();

	var srcData = {
		a: 1,
		b: 2,
		c: [3,4,5]
	};
	var data = new QByteArray(JSON.stringify(srcData), "UTF-8");
	var request = new QNetworkRequest("http://httpbin.org/post");
	request.setHeader(QNetworkRequest.ContentTypeHeader, JSON_HEADER);
	var reply = manager.post(request, data);
	testReply(reply);
	connectNetworkReply(reply);

	signals.wait();

	testReadableDevice(reply);

	data = resolveJsonReply(reply).json;
	assert(data instanceof Object);
	assert(data.a === srcData.a);
	assert(data.b === srcData.b);
	assert(data.c.length === srcData.c.length);
	assert(data.c[0] === srcData.c[0]);
	assert(data.c[1] === srcData.c[1]);
	assert(data.c[2] === srcData.c[2]);

	signals.disconnect();
}

function testPutRequest(signals)
{
	var manager = signals.manager;
	signals.connect();

	var srcData = [1,2,3,4,5];
	var reply = manager.put("http://httpbin.org/put", srcData);
	testReply(reply);
	connectNetworkReply(reply);

	signals.wait();

	testReadableDevice(reply);

	var data = resolveJsonReply(reply);
	assert(new QByteArray(data.data, "latin1").equals(srcData));

	signals.disconnect();
}

function testDeleteRequest(signals)
{
	var manager = signals.manager;
	signals.connect();

	var deleteUrl = "http://httpbin.org/delete";
	var reply = manager.deleteResource(deleteUrl);
	testReply(reply);
	connectNetworkReply(reply);

	signals.wait();

	testReadableDevice(reply);

	var data = resolveJsonReply(reply);
	assert(data.url === deleteUrl);

	signals.disconnect();
}

function testQNetworkAccessManager()
{
	assert(QNetworkAccessManager.prototype.networkAccessible === 0);
	assert(QNetworkAccessManager.prototype.head(null) === null);
	assert(QNetworkAccessManager.prototype.get(null) === null);
	assert(QNetworkAccessManager.prototype.post(null, "") === null);
	assert(QNetworkAccessManager.prototype.put(null, "") === null);
	assert(QNetworkAccessManager.prototype.deleteResource(null) === null);
	QNetworkAccessManager.prototype.connectToHostEncrypted("https://ach.com");
	QNetworkAccessManager.prototype.connectToHost("https://ach.com");
	assert(QNetworkAccessManager.prototype.toString()
		   === "QNetworkAccessManager");

	assert(isFinite(QNetworkAccessManager.HeadOperation));
	assert(isFinite(QNetworkAccessManager.GetOperation));
	assert(isFinite(QNetworkAccessManager.PutOperation));
	assert(isFinite(QNetworkAccessManager.PostOperation));
	assert(isFinite(QNetworkAccessManager.DeleteOperation));
	assert(isFinite(QNetworkAccessManager.CustomOperation));
	assert(isFinite(QNetworkAccessManager.UnknownOperation));
	assert(isFinite(QNetworkAccessManager.UnknownAccessibility));
	assert(isFinite(QNetworkAccessManager.NotAccessible));
	assert(isFinite(QNetworkAccessManager.Accessible));

	var signals = new TestNetworkSignals();
	testHeadRequest(signals);
	testGetRequest(signals);
	testPostRequest(signals);
	testPutRequest(signals);
	testDeleteRequest(signals);

	print("QNetworkAccessManager OK");
}
