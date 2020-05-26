#include "get_from_network.hpp"
#include <QEventLoop>


QNetworkReply* get_from_network(const QString& url){
	QNetworkAccessManager* const network_manager = new QNetworkAccessManager(nullptr);
	QNetworkReply* const reply = network_manager->get(QNetworkRequest(url));
	QEventLoop event_loop;
	QObject::connect(reply,  &QNetworkReply::finished,  &event_loop,  &QEventLoop::quit);
	event_loop.exec();
	
	if (reply->error() != QNetworkReply::NoError){
		fprintf(stderr,  "Cannot load from network: %s\n",  url);
		return nullptr;
	}
	
	return reply;
}
