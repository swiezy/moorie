#ifndef _MAILBOXJS_H_
#define _MAILBOXJS_H_

#include <QtWebKit>

class QMailBoxJS : public QWebPage
{

	Q_OBJECT

	public:
		std::string doGetJS(std::string url);
		std::string doPostJS(std::string host, std::string vars);
		std::string executeJS(std::string js);
		QMailBoxJS();
		~QMailBoxJS();
	private slots:
		void request_End(bool err);
		void sslErrors(QNetworkReply *,const QList<QSslError>&);
	private:
		QSslSocket *socket;
		QNetworkAccessManager *manager;
		QNetworkReply *reply;
		QEventLoop eloop;
		bool finished;
};

#endif
