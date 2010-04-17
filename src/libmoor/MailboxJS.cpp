#include "MailboxJS.h"

QMailBoxJS::QMailBoxJS()
{
	connect(this, SIGNAL(loadFinished(bool)),&eloop,SLOT(quit()));
}

std::string QMailBoxJS::doGetJS(std::string url)
{
	manager = new QNetworkAccessManager;
	this->setNetworkAccessManager(manager);
	QNetworkRequest request;
	QString urlStr;
	 request.setUrl(QUrl(urlStr.fromStdString(url)));
    request.setRawHeader("User-Agent", "User-Agent: Mozilla/5.0 (compatible;)");
    connect(manager, SIGNAL( sslErrors (QNetworkReply *,const QList<QSslError>&)),
         this, SLOT(sslErrors(QNetworkReply *,const QList<QSslError>&)));
	this->mainFrame()->load(request,manager->GetOperation);
	eloop.exec();
	return this->mainFrame()->toHtml().toStdString();
}

std::string QMailBoxJS::doPostJS(std::string url, std::string vars)
{
	QByteArray data;
	QString a;
	data=a.fromStdString(vars).toAscii();
	manager = new QNetworkAccessManager;
	this->setNetworkAccessManager(manager);
	QNetworkRequest request;
	QString urlStr;
	request.setUrl(QUrl(urlStr.fromStdString(url)));
	request.setRawHeader("User-Agent", "User-Agent: Mozilla/5.0 (compatible;)");
	connect(manager, SIGNAL( sslErrors (QNetworkReply *,const QList<QSslError>&)),
	this, SLOT(sslErrors(QNetworkReply *,const QList<QSslError>&)));
	this->mainFrame()->load(request,manager->PostOperation,data);
	eloop.exec();
	return this->mainFrame()->toHtml().toStdString();
}

std::string QMailBoxJS::executeJS(std::string js)
{
	QString a;
	this->mainFrame()->evaluateJavaScript(a.fromStdString(js));
	return this->mainFrame()->toHtml().toStdString();
}

void QMailBoxJS::sslErrors(QNetworkReply *reply,const QList<QSslError> &errors)
{
        reply->ignoreSslErrors();
        QString err;
        Q_FOREACH(QSslError e, errors)
        {
                err += e.errorString() + "\n";
        }
}

void QMailBoxJS::request_End(bool err)
{
	finished=true;
}

QMailBoxJS::~QMailBoxJS()
{
}
