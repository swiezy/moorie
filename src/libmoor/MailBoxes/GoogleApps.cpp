/*
 * This file is a part of Moorie.
 *
 * Copyright (C) 2007 Krzysztof Konieczny <silent@aol.pl>
 *
 * Moorie is free software; you can redestribute it and/or modify it
 * under terms of GNU General Public License by Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY. See GPL for more details.
 */

#include "GoogleApps.h"
#include "../Log.h"
#include "../MailboxFactory.h"

#include <sstream>
#include <boost/regex.hpp>
#include <boost/format.hpp>

namespace {
  CMailBox* Create(const std::string& username,
                   const std::string& password)
  {
    return new GoogleAppsMailbox(username, password);
  }
}

const std::string c_names[] = {
	"gala.net",
	"gazeta.pl",
	"mailbox.hu",
	"vivapolska.tv",
	"gde.ru",
	"klikni.cz",
	"livedoor.com",
	"oneindia.in",
	"bigmir.net",
	std::string()
};
const bool registered = MailboxFactory::Instance().Register(c_names, Create);

GoogleAppsMailbox::GoogleAppsMailbox(const std::string &usr, const std::string
&passwd)
  : CMailBox(usr, passwd),
  totalEmails( 0 )
{
}

int GoogleAppsMailbox::loginRequest()
{
    //    LOG_ENTER( "GoogleAppsMailbox::loginRequest " + domain );
    domain = "gazeta.pl";
    boost::smatch match2;

	
    page = doGet("https://www.google.com/a/"+domain);
    boost::regex re3("name=\"GALX\"[\n].*?value=\"([a-zA-Z0-9_*[-]*]*)\"");
    boost::regex_search(page, match2, re3);
    LOG(Log::Info, boost::format("match: %1% ") %match2[1]);
    const std::string vars = std::string("ltmpl=default&ltmplcache=2&continue=")
                + escape("https://mail.google.com/a/"+domain+"/")
                +"&service=mail&GALX="
                + escape(match2[1])
                +"&rm=false&hl=pl&Email="+escape(getUser())
                +"&Passwd="+escape(getPassword())
                +"&rmShown=1";
    page = doPost("https://www.google.com/a/"+domain+"/LoginAction2?service=mail",vars,true);
    
    //LOG(Log::Info,"page :"+ page);
		std::string username = getUser();
        boost::regex re(username);
        boost::regex re2("&amp;");
        boost::regex authre("auth=([\\w\\d_-]+)");
        boost::smatch match;
		
        if (boost::regex_search(page, match, re))
        {
                std::string url = match[1];
                if(url.find("answer=40695") != std::string::npos)
                {
                    LOG(Log::Info, "Niestety, konto zostało wyłączone.");
                    return 1;
                }
                boost::regex_replace(url, re2, "&");
                url = unescape(url);
                if (boost::regex_search(url, match, authre))
                        auth = match[1];
//		LOG(Log::Debug, "auth=" + auth);
                return 0;
        }
        else
        {
                return 1;
        }
}

void GoogleAppsMailbox::logoutRequest()
{
//	setState(Mailbox::Disconnected); //TODO
}

void GoogleAppsMailbox::getHeadersRequest()
{
//	LOG_ENTER( "GoogleAppsMailbox::getHeadersRequest " + domain );
	std::string url("https://mail.google.com/a/"+domain+"/?AuthEventSource=SSO&husr="+escape(getUser()+"@"+domain)+"&ui=html&auth=");
//	setState(Mailbox::ReadHeadersIP); // request headers
	totalEmails = 0;
	page = doGet(url + auth);

	int msgcnt = 0; // number of message headers for current page

	boost::match_results<std::string::const_iterator> match;
	boost::regex mheadre("type=\"checkbox\".+?value=\"(.+?)\".+?<a.+?(?:</font>)+\\s*(?:<b>)*(.+?)(?:<)");

	while(regex_search(page,match,mheadre)) {
	//	const string page = getPage();
		std::string::const_iterator pbegin = page.begin();
		std::string::const_iterator pend = page.end();
		msgcnt = 0;
		while (boost::regex_search(pbegin, pend, match, mheadre, boost::match_default))
		{
			EmailHeader hdr(match[1], match[2]);
// 			LOG(Log::Debug, "Found header: " + hdr.subject);
 			addHeader(hdr);
			addHeaderLink(match[1]);
			pbegin = match[2].second;
			++msgcnt;
		}
		if (msgcnt == 0)
		{
//			LOG(Log::Debug, format("Total headers = %d") % totalEmails);
// 			setState(Mailbox::ReadHeadersDone);
		}
		else
		{
			std::stringstream numstr;
			totalEmails += msgcnt;
			numstr << totalEmails;
			std::string url = "https://mail.google.com/a/"+domain+"/?ui=html&st=" + numstr.str();
			page = doGet(url);
		}
	}
//	setState(Mailbox::ReadHeadersDone);
}

int GoogleAppsMailbox::downloadRequest(int seg)
{
        std::string mylink = getLink(seg);
//	page = doGet();
        std::string link = "https://mail.google.com/a/"+domain+"/?realattid=file0&attid=0.1&disp=attd&view=att&th=" + mylink;
		LOG(Log::Debug, link);
        downloadSeg();
        doGet(link);
        if (downloadSegDone() == 0) return 0;
        else return 1;
}

GoogleAppsMailbox::~GoogleAppsMailbox()
{
}

void GoogleAppsMailbox::parseResponse()
{
//	const Mailbox::State state = getState();
}
