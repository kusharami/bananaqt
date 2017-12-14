importScript("TestQIODevice.js")
importScript("TestQNetworkRequest.js")

function testQNetworkReply()
{
	assert(QNetworkReply.prototype instanceof QIODevice);
	assert(QNetworkReply.prototype.readBufferSize === -1);
	assert(QNetworkReply.prototype.operation ===
		   QNetworkAccessManager.UnknownOperation);
	assert(QNetworkReply.prototype.request instanceof QNetworkRequest);
	assert(QNetworkReply.prototype.url instanceof QUrl);
	assert(QNetworkReply.prototype.resultCode === QNetworkReply.NoError);
	assert(QNetworkReply.prototype.isFinished === false);
	assert(QNetworkReply.prototype.running === false);
	assert(QNetworkReply.prototype.manager === null);
	assert(QNetworkReply.prototype.attribute(
			   QNetworkRequest.User) === undefined);
	assert(QNetworkReply.prototype.header(
			   QNetworkRequest.ContentTypeHeader) === undefined);
	assert(QNetworkReply.prototype.hasRawHeader(CONTENT_TYPE) === false);
	assert(QNetworkReply.prototype.rawHeader(
			   CONTENT_TYPE) instanceof QByteArray);
	assert(QNetworkReply.prototype.rawHeaderList() === undefined);
	assert(QNetworkReply.prototype.toString() === "QNetworkReply");

	assert(shouldThrow(function() { return new QNetworkReply(); }));
	assert(isFinite(QNetworkReply.NoError));
	assert(isFinite(QNetworkReply.ConnectionRefusedError));
	assert(isFinite(QNetworkReply.RemoteHostClosedError));
	assert(isFinite(QNetworkReply.HostNotFoundError));
	assert(isFinite(QNetworkReply.TimeoutError));
	assert(isFinite(QNetworkReply.OperationCanceledError));
	assert(isFinite(QNetworkReply.SslHandshakeFailedError));
	assert(isFinite(QNetworkReply.TemporaryNetworkFailureError));
	assert(isFinite(QNetworkReply.NetworkSessionFailedError));
	assert(isFinite(QNetworkReply.BackgroundRequestNotAllowedError));
	assert(isFinite(QNetworkReply.TooManyRedirectsError));
	assert(isFinite(QNetworkReply.InsecureRedirectError));
	assert(isFinite(QNetworkReply.UnknownNetworkError));
	assert(isFinite(QNetworkReply.ProxyConnectionRefusedError));
	assert(isFinite(QNetworkReply.ProxyConnectionClosedError));
	assert(isFinite(QNetworkReply.ProxyNotFoundError));
	assert(isFinite(QNetworkReply.ProxyTimeoutError));
	assert(isFinite(QNetworkReply.ProxyAuthenticationRequiredError));
	assert(isFinite(QNetworkReply.UnknownProxyError));
	assert(isFinite(QNetworkReply.ContentAccessDenied));
	assert(isFinite(QNetworkReply.ContentOperationNotPermittedError));
	assert(isFinite(QNetworkReply.ContentNotFoundError));
	assert(isFinite(QNetworkReply.AuthenticationRequiredError));
	assert(isFinite(QNetworkReply.ContentReSendError));
	assert(isFinite(QNetworkReply.ContentConflictError));
	assert(isFinite(QNetworkReply.ContentGoneError));
	assert(isFinite(QNetworkReply.UnknownContentError));
	assert(isFinite(QNetworkReply.ProtocolUnknownError));
	assert(isFinite(QNetworkReply.ProtocolInvalidOperationError));
	assert(isFinite(QNetworkReply.ProtocolFailure));
	assert(isFinite(QNetworkReply.InternalServerError));
	assert(isFinite(QNetworkReply.OperationNotImplementedError));
	assert(isFinite(QNetworkReply.ServiceUnavailableError));
	assert(isFinite(QNetworkReply.UnknownServerError));

	print("QNetworkReply OK");
}

function testReply(reply)
{
	assert(reply instanceof QNetworkReply);
	assert(!reply.isFinished);
	assert(reply.running);
	print(reply);
	assert(reply.sequential);
	assert(reply.isOpen);
	assert(!reply.writable);
	assert(!reply.isTransactionStarted);
	assert(reply.operation !== QNetworkAccessManager.UnknownOperation);
	assert(reply.manager instanceof QNetworkAccessManager);
}
