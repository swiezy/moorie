#ifndef _MAILBOXJS_H_
#define _MAILBOXJS_H_

#include <QtWebKit>
#include <QtCore>

class QMailBoxJS : public QWebPage
{

	Q_OBJECT

	public:
		std::string doGetJS(std::string url);
		std::string doPostJS(std::string host, std::string vars);
		std::string executeJS(std::string js); //js name of function to execute
		QMailBoxJS();
		~QMailBoxJS();
	private slots:
		void sslErrors(QNetworkReply *,const QList<QSslError>&); //only ignore ssl errors
	private:
		QNetworkAccessManager *manager;
		QEventLoop eloop; //Used to stop main loop
};

#endif
