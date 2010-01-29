/*
 * This file is a part of Moorie.
 *
 * Copyright (C) 2007 Pawel Stolowski <pawel.stolowski@wp.pl>
 *
 * Moorie is free software; you can redestribute it and/or modify it
 * under terms of GNU General Public License by Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY. See GPL for more details.
 */

#include "MoorhuntHashDecoder.h"

// #include "Account.h"
//#include "Util.h"
#include "Log.h"
#include <stdlib.h>
#include <mcrypt.h>
#include <mhash.h>
#include <boost/scoped_array.hpp>
#include <boost/format.hpp>
#include <boost/regex.hpp>
#include <sstream>
#include <iomanip>

#include "MoorhuntHash.h"
#include "HashUtils.h"
#include "Decoder.h"
#include "MailboxFactory.h"

namespace {
	// TODO: should this mapping be moved to some out-of-source
	// configuration file ?
	const char* getMailboxName(int id) {
//  		LOG(Log::Debug, boost::format( "ID: %1%. " ) %id);
		switch (id) {
			case 2	:		return "yahoo.com";
			case 3	:		return "gmail.com";
			case 4	:		return "o2.pl";
			case 5  :		return "lajt.hu";
			case 9	:		return "wp.pl";
			case 72	:		return "oi.com.br";
			case 22	:		return "mynet.com";
			case -27:		return "aol.pl";
			case -12:		return "aol.de";
			case -93:		return "gazeta.pl";
			case -31:		return "gala.net";
			case 53	:		return "yandex.ru";
			case 24	:		return "mail.ru";
			case -29:		return "bigmir.net";
			case -56:		return "azet.sk";
			case -86:		return "poczta.onet.pl";
			case 71	:		return "onet.eu";
			case 23	:		return "rock.com";
			case 87 :		return "mailbox.hu";
			case -28:		return "hotmail.com";
			case -124:		return "interfree.it";
			case -100:		return "tiscali.it";
			case -11:		return "vivapolska.tv";
			case 125:		return "wippies.com";
			case -9 :		return "gde.ru";
			case -65:		return "klikni.cz";
			case -30:		return "livedoor.com";
			case 55 :		return "oneindia.in";
			case -69:		return "libero.it";

			default	:		return 0;
		}
	}

	unsigned char* getVerByte(const unsigned char data[][34],
                            unsigned char vermaj,
                            unsigned char vermin)
	{
		for (int i = 0; data[i][0]; ++i)
			if ((data[i][0] == vermaj) && (data[i][1] == vermin))
				return const_cast<unsigned char*>(&data[i][2]);

		return 0;
	}
}

const unsigned char keys[][34] = {
	{'a', 'h', 0x1e, 0x1e, 0x1e, 0x1e, 0x1f, 0x1f, 0x1f, 0x1e, 0x1e, 0x1f, 0x1e, 0x1e, 0x1e, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1e, 0x1e, 0x1e, 0x1f, 0x1e, 0x1e, 0x1f, 0x1f, 0x1f, 0x1f, 0x1e, 0x1f, 0x1e},
	{'a', 'g', 0xfb, 0xfb, 0xbb, 0xbb, 0xbb, 0xbb, 0xfb, 0xfb, 0xfb, 0xfb, 0xfb, 0xfb, 0xfb, 0xbb, 0xfb, 0xbb, 0xbb, 0xbb, 0xfb, 0xfb, 0xbb, 0xfb, 0xbb, 0xfb, 0xbb, 0xfb, 0xfb, 0xfb, 0xfb, 0xfb, 0xfb, 0xbb},
	{'a', 'f', 0xd2, 0x32, 0x52, 0x12, 0x32, 0x72, 0xf6, 0x32, 0xd2, 0x72, 0xb2, 0xf6, 0xd6, 0xb2, 0x76, 0x76, 0x36, 0x36, 0xd6, 0x72, 0x92, 0xd6, 0x16, 0xf2, 0x16, 0x76, 0xf2, 0x56, 0x72, 0xb6, 0xf2, 0x52},
	{'a', 'e', 0x34, 0x12, 0x65, 0x90, 0xae, 0xdf, 0x89, 0x56, 0x90, 0x12, 0x34, 0x89, 0xbb, 0xaa, 0x01, 0x67, 0x46, 0xf0, 0x90, 0x78, 0xac, 0x27, 0xe9, 0x89, 0x5f, 0x6c, 0x0f, 0xa1, 0xc3, 0x8f, 0x92, 0x23},
	{'a', 'd', 0xFF, 0x22, 0x35, 0x82, 0x9F, 0xAE, 0x76, 0x7C, 0x59, 0x71, 0x8E, 0x77, 0x7F, 0x0A, 0x74, 0x01, 0x61, 0xD6, 0x45, 0x08, 0x36, 0xFB, 0xFE, 0x63, 0x40, 0x13, 0xE6, 0xEE, 0xB0, 0x33, 0x69, 0x69},
	{'a', 'a', 0x50, 0x6F, 0x64, 0x7A, 0x69, 0xEA, 0x6B, 0x6F, 0x77, 0x61, 0x6E, 0x69, 0x61, 0x20, 0x64, 0x6C, 0x61, 0x20, 0x4D, 0x79, 0x73, 0x74, 0x68, 0x65, 0x61, 0x27, 0x69, 0x20, 0x3A, 0x29, 0x0, 0xFF},
	{'a', 'b', 0x40, 0x3F, 0x65, 0x73, 0x79, 0xE9, 0x61, 0x2F, 0x47, 0x65, 0x3E, 0x65, 0x62, 0x10, 0x64, 0x45, 0x51, 0x56, 0x45, 0x0, 0x56, 0x75, 0x68, 0x35, 0x81, 0x28, 0x61, 0x20, 0x3A, 0x49, 0x35, 0x48},
	{'a', 'c', 0x53, 0x42, 0x75, 0x82, 0x8F, 0xFE, 0x76, 0x3C, 0x53, 0x71, 0x4E, 0x77, 0x7F, 0x8A, 0x74, 0x5C, 0x61, 0xC6, 0x45, 0x8, 0x36, 0xFB, 0xFE, 0x43, 0x40, 0x13, 0xF6, 0xEE, 0xB0, 0x6B, 0xC5, 0x18},
	{'d', 'a', 0x00, 0x00, 0x41, 0x55, 0x34, 0xE4, 0x31, 0x24, 0x43, 0x55, 0x7E, 0xA5, 0x72, 0x11, 0xFF, 0x0, 0x67, 0x58, 0x96, 0x0, 0x0, 0x0, 0x0, 0x11, 0x13, 0x32, 0x63, 0x24, 0x31, 0x79, 0x65, 0x78},
	{'d', 'b', 0x10, 0x01, 0x00, 0x51, 0x31, 0xEA, 0xC1, 0x2D, 0x63, 0x55, 0x9E, 0xA2, 0x71, 0x31, 0x5F, 0x0, 0x37, 0x54, 0x95, 0x6, 0x32, 0x32, 0x32, 0x12, 0x14, 0x12, 0x64, 0x64, 0x51, 0xA9, 0xC5, 0xFF},
	{'d', 'c', 0x00, 0x31, 0x40, 0xE1, 0xAE, 0x34, 0x67, 0x22, 0x33, 0x55, 0x7E, 0xBC, 0x21, 0x35, 0x51, 0x00, 0x00, 0x00, 0x93, 0x16, 0x33, 0x34, 0x22, 0x42, 0x16, 0x45, 0x81, 0x28, 0x36, 0xB4, 0xCA, 0xEE},
	{0, 0}
};

const unsigned char ivec[][34] = {
	{'a', 'h', 0x59, 0xdd, 0x59, 0x19, 0x5b, 0xdb, 0x99, 0x9b, 0xd7, 0xd9, 0x57, 0x9d, 0x1b, 0x9b, 0x59, 0x17, 0x19, 0x97, 0xdd, 0x5d, 0x9b, 0x5d, 0x97, 0xd9, 0x19, 0x97, 0x57, 0x99, 0x1d, 0x59, 0x19, 0x5d},
	{'a', 'g', 0xc1, 0xd5, 0x15, 0xd5, 0x01, 0x11, 0xd5, 0xd4, 0x04, 0x15, 0x05, 0xc1, 0xc0, 0xd1, 0x14, 0xc5, 0xd5, 0x05, 0x01, 0x11, 0xd0, 0x10, 0xc5, 0x01, 0xc1, 0xd5, 0x15, 0xc5, 0xc0, 0x15, 0xd5, 0x15},
	{'a', 'f', 0xad, 0xf5, 0xf4, 0x31, 0xc9, 0xf1, 0xec, 0x14, 0xf1, 0x90, 0x15, 0x6d, 0x50, 0xec, 0xb0, 0x4d, 0xf5, 0x28, 0x95, 0x14, 0xad, 0xf4, 0xd0, 0x0d, 0xd1, 0x8c, 0x14, 0x31, 0xf1, 0xC8, 0xf4, 0x15},
	{'a', 'e', 0x78, 50, 0x31, 0xf6, 0x4e, 0x8e, 0x29, 0x11, 0x34, 0x97, 0x12, 170, 0xc5, 0x3b, 0x67, 0xca, 0x20, 0xfd, 0x90, 0x01, 120, 0x23, 0x55, 0x08, 0x44, 0x99, 0x13, 0xe4, 0x67, 0x4f, 0x21, 0x45},
	{'a', 'd', 0x88, 0x38, 0x61, 230, 0x2e, 0x9e, 0x09, 0x0b, 0x55, 0x18, 0x42, 0x84, 0xa7, 0xee, 0xff, 0xeb, 0x88, 0x52, 0x36, 0xcd, 100, 0xa5, 0x54, 0xb2, 0x12, 0xef, 0, 0xff, 0x99, 0x66, 0x33, 0xcc},
	{'a', 'a', 0x3f, 0xec, 0xd8, 0x99, 0x09, 0x18, 0x1c, 0xdd, 0xbb, 0x98, 0xb0, 0x57, 0x28, 0x56, 0x42, 0x1b, 0x8b, 0x46, 0x18, 0x22, 0x28, 0x21, 0xd8, 0xce, 0xc9, 0x8a, 0x95, 0x87, 0x70, 0x18, 0x06, 0xdd},
	{'a', 'b', 0x76, 0x48, 0x20, 0xc0, 30, 0xae, 0x40, 0xeb, 0x39, 0xa8, 50, 0x74, 0xf7, 0x66, 0x29, 250, 0xe0, 0xb2, 0xbc, 12, 0x67, 5, 0x72, 190, 0xfe, 0x42, 0x2a, 0x70, 0xdd, 0, 0xc2, 160},
	{'a', 'c', 0x36, 0x38, 0x60, 0xd0, 0x2e, 0x3e, 0x90, 0x0b, 0x69, 0x18, 0x42, 0x84, 0xa7, 0x86, 0x49, 0x6b, 0x55, 0x52, 0x36, 0xcd, 100, 0xa1, 0x54, 0xb2, 0x12, 0xef, 0x13, 0x6f, 0x00, 0x30, 0xc9, 0x10},
	{'d', 'a', 0x71, 0x08, 0x21, 0x10, 0x3e, 0x4e, 0x45, 230, 0x79, 0xa8, 0x72, 0x84, 0x27, 0x63, 0x49, 0xf5, 0xe1, 0x34, 0xbc, 0x53, 0x34, 0x35, 50, 0x5e, 0xac, 190, 0xef, 250, 0x00, 0x00, 0x11, 100},
	{'d', 'b', 0x10, 0x01, 0x01, 0x11, 0x31, 0x14, 0x15, 0xe1, 0x79, 0xa7, 0x92, 0x14, 0x2a, 0x63, 0x56, 0x85, 0xe4, 50, 180, 170, 0xf4, 0x3a, 0x36, 0x2e, 0x1c, 0xb3, 0xe4, 0x3a, 0x00, 0x56, 0x15, 0x77},
	{'d', 'c', 0x11, 0x12, 0x13, 0x11, 0x23, 0xf3, 0x73, 0x21, 0x74, 0x67, 0x97, 0x13, 0xfe, 0x00, 0x00, 0x2f, 0x14, 0x12, 0x34, 0x12, 0x23, 0xb6, 0x4f, 0x2e, 0x4f, 0xde, 0x00, 0xff, 0x00, 0xf4, 0xfc, 0x11},
	{0, 0}
};

const char *prefixes[] = {"<<", "mh://", NULL};

Hash* MoorhuntHashDecoder::decode(const std::string& hashcode)
{
	int declen;
	int i;
	int hpos;
	HashInfo result;

	MCRYPT td;
	result.valid = false;
	size_t hashinSize = hashcode.size() + hashcode.size() / 64 + 1;
	boost::scoped_array<char> hashin(new char[hashinSize]);

	//
	// reformat hash code - add new lines, remove or replace some
	// characters
	i = 0;
	for (hpos = 0; hashcode[hpos]; ++hpos)
	{
		if ((hashcode[hpos] == '>') ||
				(hashcode[hpos] == ' ') ||
				(hashcode[hpos] == '\t'))
			continue;

		if (hashcode[hpos] == '-')
			hashin[i++] = '+';
		else
			hashin[i++] = hashcode[hpos];
		if ((i % 64) == 0)
			hashin[i++] = '\n';
	}
	if (i == 0)
		return new MoorhuntHash(result); // empty hash code


	hashin[i++] = '\n';
	hashin[i] = 0;

	result.hashString = std::string(&hashin[0]);
	// strip off moorhunt prefixes
	hpos = 0;
	for (int j=0; prefixes[j]; j++)
	{
		if (strncmp(&hashin[0], prefixes[j], strlen(prefixes[j])) == 0)
		{
			hpos += strlen(prefixes[j]);
			break;
		}
//		std::cout << j << std::endl;
	}

	result.verMaj = hashin[hpos++]; // determine moorhunt version
	result.verMin = hashin[hpos++];

//	unsigned char *in = base64Decode(&hashin[hpos], i, &declen);
	unsigned char *in = unbase64(&hashin[hpos], strlen(&hashin[hpos]));
	declen = strlen(&hashin[hpos]);

	//
	// AES decryption
	do
	{
		if (declen == 0)
			break;

		td = mcrypt_module_open((char*)"rijndael-256", NULL, (char*)"cbc", NULL);
		if (td == MCRYPT_FAILED)
			break;

		unsigned char* key = getVerByte(keys, result.verMaj, result.verMin);
		unsigned char* iv = getVerByte(ivec, result.verMaj, result.verMin);

		if (key == NULL || iv == NULL)
			break;

		if (mcrypt_generic_init(td, key, 32, iv) < 0)
		{
			mcrypt_module_close(td);
			break;
		}

		if (mdecrypt_generic(td, in, declen) != 0)
 		{
 			mcrypt_module_close(td);
 			break;
 		}

//		LOG_BUFFER(Log::Debug, reinterpret_cast<char *>(in), declen);
		mcrypt_generic_deinit(td);
		mcrypt_module_close(td);

		result.valid = true;

	} while (false); //just once

	if (result.valid)
	{
		std::string tmp;
		char *src = reinterpret_cast<char *>(in);
// 		std::cout << src << std::endl;
//		char *ptr;
		std::vector<int> v = split(src, declen);
		std::cout << v.size() << std::endl;
		if (v.size() < 39)
		{
			result.valid = false;
//			LOG(Log::Warn, "decode: less than expected number of tokens");
		}
		else
		{
			int numOfMirrors;
			result.fileName = std::string(src, v[1]);
			result.crc = (
					((unsigned char)src[v[2]]) << 24 |
					((unsigned char)src[v[2] + 1]) << 16 |
					((unsigned char)src[v[2] + 2]) << 8 |
					((unsigned char)src[v[2] + 3])
			);
			std::stringstream asd;
// 			int dupa =((unsigned char)src[v[2]]) << 24;
// 			asd << dupa << " " << std::hex << result.crc;
// 			std::cout << " CRC: " <<  asd.str() << " " << dupa << std::endl;

			tmp = std::string(src + v[4], v[5]);
			result.fileSize = atol(tmp.c_str());
			tmp = std::string(src + v[6], v[7]); // bool 1
			tmp = std::string(src + v[8], v[9]); // bool 2
			tmp = std::string(src + v[10], v[11]); // num of segments
			result.numOfSegments = atoi(tmp.c_str());
			tmp = std::string(src + v[12], v[13]); // int
			result.segmentSize = atoi(tmp.c_str());
			tmp = std::string(src + v[14], v[15]); // int
			result.accessPasswd = std::string(src + v[16], v[17]);
			tmp = std::string(src + v[18], v[19]); // num of mirrors
			numOfMirrors = atoi(tmp.c_str());
			int offset = 0;
			for (int i = 0; i < numOfMirrors; ++i) {
				offset = 6 * i;
				const char* id = getMailboxName(src[v[20 + offset]]);
				// Filtering out all mailboxes program is unable to handle!
				if (id && MailboxFactory::Instance().Registered(id)) {
					HashInfo::MboxAccount account;
					account.name = id;
					account.login = std::string(src + v[22 + offset], v[23 + offset]);
					account.password = std::string(src + v[24 + offset], v[25 + offset]);
					result.accounts.push_back(account);
				}
			}
			result.coverURL = std::string(src + v[26 + offset], v[27 + offset]);
 			result.editPasswd = std::string(src + v[28 + offset], v[29 + offset]);
 			result.forWhom = std::string(src + v[30 + offset], v[31 + offset]);
 			result.descURL = std::string(src + v[32 + offset], v[33 + offset]);
 			result.fullTitle = std::string(src + v[34 + offset], v[35 + offset]);
 			result.uploader = std::string(src + v[36 + offset], v[37 + offset]);
 			result.comment = std::string(src + v[38 + offset], v[39 + offset]);

			if (result.fileSize <=0 || numOfMirrors <= 0 || result.numOfSegments <=0)
			{
				result.valid = false;
//				LOG(Log::Warn, boost::format("fileSize or numOfMirrors or numOfMirrors <= 0: %d %d %d") % fileSize % numOfMirrors % numOfSegments);
			}
		}
	}

	delete [] in;
	return new MoorhuntHash(result);
}

std::string MoorhuntHashEncoder::encode(std::string data) {

	MCRYPT td;
	int length = strlen(data.c_str());
	char *in;
		// AES decryption

	do
	{
		td = mcrypt_module_open((char*)"rijndael-256", NULL, (char*)"cbc", NULL);
		if (td == MCRYPT_FAILED)
			break;

		unsigned char* key = getVerByte(keys, 'a', 'h');
		unsigned char* iv = getVerByte(ivec, 'a', 'h');

		if (key == NULL || iv == NULL)
			break;

		if (mcrypt_generic_init(td, key, 32, iv) < 0)
		{
			mcrypt_module_close(td);
			break;
		}

		in = (char*)data.c_str();
		if (mcrypt_generic(td, in, length) != 0)
		{
			mcrypt_module_close(td);
			break;
		}

//		LOG_BUFFER(Log::Debug, reinterpret_cast<char *>(in), declen);
		mcrypt_generic_deinit(td);
		mcrypt_module_close(td);

// 		result.valid = true;

	} while (false); //just once

	unsigned char *myhash = base64(in, length);

	std::stringstream ss;
	ss << myhash;
	return ss.str();
}


// bool MoorhuntHash::usesMD5Passwords() const
// {
// 	return ( verMaj == 'a' && ( verMin == 'g' || verMin == 'h' ) );
// }

// bool MoorhuntHash::isAccessPasswordProtected() const
// {
// 	return accessPasswd.size() > 0;
// }

// bool MoorhuntHash::checkAccessPassword( const std::string & pass ) const
// {
// 	if ( !isAccessPasswordProtected() ) {
// 		return true;
// 	}
// 	if ( usesMD5Passwords() ) {
// 		return getMD5( pass ) == accessPasswd;
// 	}
// 	return pass == accessPasswd;
// }

// bool MoorhuntHash::checkEditPassword( const std::string & pass ) const
// {
// 	if ( usesMD5Passwords() ) {
// 		return getMD5( pass ) == editPasswd;
// 	}
// 	return pass == editPasswd;
// }

// bool MoorhuntHash::read(std::ifstream &f)
// {
//         const boost::regex re("^\\s*#");
// 	std::string hashStr;
// 	while (f.good())
// 	{
// 		std::string line;
// 		getline(f, line);
// 		if (boost::regex_search(line, re))
// 			continue;
// 		hashStr += line;
// 		if (line.find(">>") != std::string::npos)
// 		{
// //			LOG(Log::Debug, "Found Moorhunt hash");
// 			decode(hashStr);
// 			hashStr = "";
// 			return true;
// 		}
// 	}
// 	return false;
// }

// std::list<MoorhuntHash> MoorhuntHash::fromFile(std::ifstream &f)
// {
// 	std::list<MoorhuntHash> mh;
// 	while (f.good())
// 	{
// 		MoorhuntHash h;
// 		if (h.read(f))
// 			mh.push_back(h);
// 	}
// 	return mh;
// }
