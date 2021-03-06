/*
 * This file is a part of Moorie
 *
 * Copyright (C) 2007-2010 Moorie Team - http://moorie.pl/
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
   
#include "MyNet.h"
#include "../Log.h"
#include "../MailboxFactory.h"

// #include "Downloader.h"
// #include "MoorieException.h"
#include <sstream>
#include <boost/regex.hpp>

namespace {
        CMailBox* Create(const std::string& name,
                         const std::string& username,
					 const std::string& password)
	{
                return new MyNetMailbox(name, username, password);
	}

	const bool registered = MailboxFactory::Instance().
			Register("mynet.com", Create);
}

MyNetMailbox::MyNetMailbox(const std::string &name, const std::string &usr, const std::string &passwd)
    : CMailBox(name, usr, passwd),
      totalEmails( 0 )
{
}

int MyNetMailbox::loginRequest()
{
// 	LOG(Log::Debug,"MyNetMailbox::loginRequest");
// 	LOG(Log::Debug,"MyNetMailbox::State");
        std::string page,url,purl;
	url="http://eposta.mynet.com/index/mymail.html";
	page=doGet(url);
	purl="http://uyeler.mynet.com/login/login.asp?loginRequestingURL=http%3A%2F%2Feposta.mynet.com%2Findex%2Fmymail.html&formname=eposta";
                const std::string vars = std::string("faultCount=&faultyUser=&loginRequestingURL=")
		+ escape(url)
		+"&nameofservice=eposta&pageURL="
		+ escape(purl)
		+"&password="
		+ escape(getPassword())
		+"&rememberstate=2&rememberstatep=2&username="
		+ escape(getUser())
		+"&x=0&y=0";
// 	setState(Mailbox::LoginIP);
	doPost("https://uyeler.mynet.com/index/uyegiris.html",vars);
	page=doGet(url,true);
        if(page.find("Merhaba "+getUser())==std::string::npos)
	{
// 		setState(Mailbox::LoginError); //login failure
// 		throw MoorieException("Login failed");
		return 1;
	}
// 	setState(Mailbox::LoginDone);
        boost::smatch match;
	//regex re("Location: (http[^\r\n]*)");
	//regex_search(page,match,re);
	//page=doGet(match[1]);
        boost::regex re("<a href=\"([^\"]*mynet.com/web2/src/showFolder.php\\?adu[^\"&]*)");
        boost::regex_search(page,match,re);
	auth=match[1];
// 	setState(Mailbox::Connected);
	return 0;
}

void MyNetMailbox::logoutRequest()
{
	doGet("http://uyeler.mynet.com/logout.aspx");
//	setState(Mailbox::Disconnected); //TODO
}

void MyNetMailbox::getHeadersRequest()
{
// 	LOG_ENTER("MyNetMailbox::getHeadersRequest");
        std::string page,url;
        boost::smatch match;
        boost::regex re_inbox("<a href=\"([^\"]*)\">.leri</a>");
        boost::regex re("<a class=\"subject\" href=\"rbody.php[^\"]*passed_id=([0-9]*)[^\"]*\">([^<>]*)</a>");
//	setState(Mailbox::ReadHeadersIP); // request headers
        boost::replace_first(auth,"web2","webmail");
	mserv=auth.substr(0,auth.find("showFolder.php"));
	page=doGet(auth+"&mailbox=INBOX.Posta_kutusu");
	while(1)
	{
                std::string::const_iterator pbegin = page.begin();
                std::string::const_iterator pend = page.end();
                while (regex_search(pbegin, pend, match, re, boost::match_default))
		{
			EmailHeader hdr(match[1], match[2]);
// 			LOG(Log::Debug,"Subject: "+match[2]);
// 			LOG(Log::Debug, "Found header: " + hdr.subject);
			addHeader(hdr);
			addHeaderLink(match[1]);
			pbegin = match[2].second;
		}
		if(!regex_search(page,match,re_inbox))
			break;
		url=match[1];
                boost::erase_all(url,"amp;");
		page=doGet(mserv+url);
	}
// 	setState(Mailbox::ReadHeadersDone);
}

int MyNetMailbox::downloadRequest(int seg)
{
// 	LOG_ENTER("MyNetMailbox::downloadRequest");
        std::string page,dlink;

        std::string id = getLink(seg);
	page=doGet(mserv+"rbody2.php?mailbox=INBOX.Posta_kutusu&passed_id="+id);
	dlink=mserv+"download.php?absolute_dl=true&passed_id="+id+"&mailbox=INBOX.Posta_kutusu&ent_id=2";

	LOG(Log::Debug, dlink);
	downloadSeg();
	doGet(dlink);
// 	setSegment(s);

// 	dlink="download.php?absolute_dl=true&passed_id="+hdr.uniqueId+"&mailbox=INBOX.Posta_kutusu&ent_id=2";
// 	setState(Mailbox::DownloadIP);
// 	doGet(mserv+dlink);
	if (downloadSegDone() == 0)
		return 0;
	else
		return 1;
}

int MyNetMailbox::uploadRequest(std::string filename, std::vector<std::string> to, int seg)
{
}


MyNetMailbox::~MyNetMailbox()
{
}

void MyNetMailbox::parseResponse()
{
// 	const Mailbox::State state = getState();
}
