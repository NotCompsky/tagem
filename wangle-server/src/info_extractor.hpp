#pragma once

#include "db_info.hpp"
#include "thread_pool.hpp"
#include "str_parsing_macros.hpp"
#include "sprexer/element.hpp"
#include "sprexer/doc.hpp"
#include "sprexer/parser.hpp"
#include "sprexer/nullstr.hpp"


#define CONVERT_TO_USUAL_DATETIME_FROM \
	datetime = std::string_view(datetime.data(), std::char_traits<char>::length("2020-12-01T18:11:19"));


// TODO: Change ThreadPool so that it allows non-pointers (and therefore avoids dynamic memory allocaitons)
class HTMLParserPool : public ThreadPool<sprexer::Parser*, HTMLParserPool> {
  public:	
	void new_obj(sprexer::Parser*& parser){
		parser = new sprexer::Parser();
	};
	void kill_obj(sprexer::Parser* parser){
		delete parser;
	};
};

static
HTMLParserPool html_parser_pool;

extern
std::vector<DatabaseInfo> db_infos;



namespace info_extractor {


namespace _f {
	using namespace compsky::asciify::flag;
	constexpr Escape esc;
}


enum DomainID {
	NoDomain,
	
	DirectFile,
	
	BBCNews,
	Guardian,
	Telegraph,
	Indep,
	DailyMail,
	TheTimes,
	SkyNews,
	TheCourier,
	
	CNN,
	TheHill,
	Bloomberg,
	NYT,
	WSJ,
	BusinessInsider,
	Vice,
	Medium,
	WashPost,
	Reuters,
	Atlantic,
	NBCNews,
	CBSNews,
	APNews,
	Slate,
	Forbes,
	Politico,
	TheVerge,
	TheAmericanProspect,
	WBUR,
	GlobalNewsCA,
	ReasonCom,
	TheNation,
	Pajiba,
	
	SCMP,
	DieWelt,
	RussiaToday,
	ElectronicIntifada,
	CollegeFix,
	
	Wikipedia,
	
	Reddit,
	Twitter,
	Tumblr,
	TikTok,
	Digg,
	InstagramPost,
	
	Streamable,
	Gfycat,
	SoundCloud,
	LiveLeak,
	
	GitHub,
	
	n_domains
};


bool is_valid_protocol(const char*& str){
	--str;
	#include "auto-generated/auto__info_extractor-verify.hpp"
	return false;
}

DomainID get_domain_id(const char* str){
	if (not is_valid_protocol(str))
		return NoDomain;
	++str; // Skip trailing slash
	--str; // Prepare for my trie algorithm
	#include "auto-generated/auto__info_extractor-domainid.hpp"
	return NoDomain;
}

void set_to_string_literal_null_if_null(std::string_view& v){
	if (v == null_str_view)
		v = "NULL";
}

void set_to_string_literal_zero_if_null(std::string_view& v){
	if (v == null_str_view)
		v = "0";
}

std::string_view find_element_attr(const sprexer::Doc& doc,  char* const selector_path,  const char* const attr){
	sprexer::Element element(doc.get_element_from_class_selector_path(selector_path));
	if (element.is_null())
		return null_str_view;
	return element.get_value(attr);
}

template<typename FileIDType>
bool record_info(const FileIDType file_id,  const char* dest_dir,  char* resulting_fp_as_output,  char* const buf,  const char* url,  std::string_view& author,  const char*& author_title){
	const auto domain_id = get_domain_id(url);
	if (domain_id == NoDomain)
		return true;
	
	sprexer::Parser* parser = html_parser_pool.get();
	char* html_buf = buf;
	switch(domain_id){
		case Reddit:
			/*char new_url[200];
			// Strip HUMAN_TITLE/ out of https://www.reddit.com/r/SUBREDDIT/comments/POST_ID/HUMAN_TITLE/
			// equivalent to following the 7th slash with a null byte
			char* _itr = url;
			char* _itr2 = new_url;
			for(auto i = 0;  i < 7;  ){
				*_itr2 = *_itr;
				if (*_itr == '/')
					++i;
				++_itr;
			}
			*_itr = 0;*/
			char* _itr = buf;
			compsky::asciify::asciify(_itr, url, ".json", '\0');
			url = buf;
			break;
	}
	size_t html_sz = curl::dl_buf(url, html_buf);
	switch(domain_id){
		case DirectFile:
		case Reddit:
		case InstagramPost:
			// Avoid parsing the document
			html_sz = 0;
			break;
	}
	sprexer::Doc doc(*parser, html_buf, html_sz);
	html_parser_pool.free(parser);
	
	std::string_view title = "";
	std::string_view datetime = "";
	std::string_view description = "";
	const char* datetime_fmt = "%Y-%m-%dT%H:%i:%S";
	constexpr char* datetime_fmt_common_alt = "%Y-%m-%dT%H:%i:%SZ";
		/* 
		 * This default value is of the ISO datetime format
		 * e.g. 2020-12-14T14:04:50.000Z
		 * In UNIX format it would be "%Y-%m-%dT%H:%M:%S.%fZ";
		 */
	std::string_view timestamp = null_str_view;
	std::string_view likes = null_str_view;
	std::string_view views = null_str_view;
	std::string_view duration = null_str_view;
	std::string_view link_url = null_str_view; // The linked article or video
	
	author_title = "Uploader: ";
	switch(domain_id){
		case DirectFile:
			link_url = std::string_view(url, strlen(url));
			break;
		case BBCNews: {
			char _title[] = "*@h1#main-heading";
			title = find_element_attr(doc, _title, ".");
			char _dt[] = "*@h1#main-heading>^*@time";
			datetime = find_element_attr(doc, _dt, "datetime");
			CONVERT_TO_USUAL_DATETIME_FROM
			break;
		}
		case Guardian: {
			char _title[] = "*@h1";
			title = find_element_attr(doc, _title, ".");
			timestamp = STRING_VIEW_FROM_UP_TO(22, ",\"webPublicationDate\":")(html_buf, ',');
			author = STRING_VIEW_FROM_UP_TO(10, ",\"byline\":")(html_buf, '"');
			author_title = "Journalist: ";
			break;
		}
		case Twitter: {
			char _likes[200] = "*@a:href=";
			url += 18 + ((url[4] == 's') ? 1 : 0); // skip "httpS://twitter.com"
			auto i = 0;
			while(true){
				_likes[i] = url[i];
				if (unlikely(url[i] == 0))
					break;
				++i;
			}
			memcpy(_likes + i,  "/likes>>",  8);
			likes = find_element_attr(doc, _likes, ".");
			author_title = "twitter @";
			break;
		}
		case Reddit: {
			title = STRING_VIEW_FROM_UP_TO(12, ", \"title\": \"")(html_buf, '"');
			author = STRING_VIEW_FROM_UP_TO(13 , ", \"author\": \"")(html_buf, '"'); // NOTE: Multiple matches, but the first is selected
			link_url = STRING_VIEW_FROM_UP_TO(10, ", \"url\": \"")(html_buf, '"');
			likes = STRING_VIEW_FROM_UP_TO(11, ", \"score\": ")(html_buf, ',');
			timestamp = STRING_VIEW_FROM_UP_TO(17, ", \"created_utc\": ")(html_buf, '.');
			author_title = "/u/";
			break;
		}
		case TheAmericanProspect: {
			char _title[] = "*@div#title>@h1";
			title = find_element_attr(doc, _title, ".");
			char _datetime[] = "*@time:itemprop=datePublished";
			datetime = find_element_attr(doc, _datetime, "datetime");
			CONVERT_TO_USUAL_DATETIME_FROM
			char _author[] = "*@a:itemprop=author";
			author = find_element_attr(doc, _author, ".");
			char _descr[] = "*@div#title>@p.subtitle>@span";
			description = find_element_attr(doc, _descr, ".");
			author_title = "Journalist: ";
			break;
		}
		case NYT: {
			char _title[] = "*@meta:property=og:title";
			title = find_element_attr(doc, _title, "content");
			char _descr[] = "*@meta:property=og:description";
			description = find_element_attr(doc, _descr, "content");
			char _author[] = "*@span.last-byline";
			author = find_element_attr(doc, _author, ".");
			char _datetime[] = "*@time:datetime";
			datetime = find_element_attr(doc, _datetime, "datetime");
			CONVERT_TO_USUAL_DATETIME_FROM
			author_title = "Journalist: ";
			break;
		}
		case TheCourier: {
			char _title[] = "*@h1.title";
			title = find_element_attr(doc, _title, ".");
			char _descr[] = "*@meta:property=og:description";
			description = find_element_attr(doc, _descr, "content");
			char _author[] = "*@span.byline__name";
			author = find_element_attr(doc, _author, ".");
			datetime = STRING_VIEW_FROM_UP_TO(18,  ",\"datePublished\":\"")(html_buf, '"');
			CONVERT_TO_USUAL_DATETIME_FROM
			author_title = "Journalist: ";
			break;
		}
		case WBUR: {
			char _title[] = "*@h1.article-hdr";
			title = find_element_attr(doc, _title, ".");
			break;
		}
		case APNews: {
			char _title[] = "*@div.CardHeadline>*@h1";
			title = find_element_attr(doc, _title, ".");
			char _datetime[] = "*@div.CardHeadline>*@span.Timestamp";
			datetime = find_element_attr(doc, _datetime, "data-source");
			CONVERT_TO_USUAL_DATETIME_FROM
			break;
		}
		case GlobalNewsCA: {
			char _title[] = "*@h1.l-article__title";
			title = find_element_attr(doc, _title, ".");
			char _author[] = "*@a.c-byline__link";
			author = find_element_attr(doc, _author, ".");
			datetime = STRING_VIEW_FROM_UP_TO(17,  "\"datePublished\":\"")(html_buf, '"');
			CONVERT_TO_USUAL_DATETIME_FROM
			author_title = "Journalist: ";
			break;
		}
		case ReasonCom: {
			char _title[] = "*@h1.entry-title>@a";
			title = find_element_attr(doc, _title, ".");
			char _descr[] = "*@h2.entry-subtitle";
			description = find_element_attr(doc, _descr, ".");
			char _author[] = "*@span.author>@a";
			author = find_element_attr(doc, _author, ".");
			char _datetime[] = "*@meta:property=article:published_time";
			datetime = find_element_attr(doc, _datetime, "content");
			CONVERT_TO_USUAL_DATETIME_FROM
			author_title = "Journalist: ";
			break;
		}
		case Digg: {
			char _title[] = "*@h2.headline>@a";
			title = find_element_attr(doc, _title, ".");
			char _descr[] = "*@div.description";
			description = find_element_attr(doc, _descr, ".");
			char _author[] = "*@div.metadata>@a.author";
			author = find_element_attr(doc, _author, ".");
			char _datetime[] = "*@div.metadata>*@time:datetime";
			datetime = find_element_attr(doc, _datetime, "datetime");
			CONVERT_TO_USUAL_DATETIME_FROM
			break;
		}
		case TheNation: {
			char _title[] = "*@h1.title";
			title = find_element_attr(doc, _title, ".");
			char _descr[] = "*@h2.subtitle";
			description = find_element_attr(doc, _descr, ".");
			char _author[] = "*@h2.author_name>@a";
			author = find_element_attr(doc, _author, ".");
			char _datetime[] = "*@meta:property=article:published_time";
			datetime = find_element_attr(doc, _datetime, "content");
			CONVERT_TO_USUAL_DATETIME_FROM
			author_title = "Journalist: ";
			break;
		}
		case Pajiba: {
			char _title[] = "*@h1";
			title = find_element_attr(doc, _title, ".");
			char _descr[] = "*@h2.subtitle";
			description = find_element_attr(doc, _descr, ".");
			char _author[] = "*@div#ad-desktop>@p>@a";
			author = find_element_attr(doc, _author, ".");
			char _datetime[] = "*@meta:property=article:published_time";
			datetime = find_element_attr(doc, _datetime, "content");
			CONVERT_TO_USUAL_DATETIME_FROM
			author_title = "Journalist: ";
			break;
		}
		case InstagramPost: {
			title = STRING_VIEW_FROM_UP_TO(12, ",\"caption\":\"")(html_buf, '"');
			likes = STRING_VIEW_FROM_UP_TO(36, ",\"edge_media_preview_like\":{\"count\":")(html_buf, ',');
			author = STRING_VIEW_FROM_UP_TO(19, ",\"alternateName\":\"@")(html_buf, '"');
			timestamp = STRING_VIEW_FROM_UP_TO(22, ",\"taken_at_timestamp\":")(html_buf, ',');
			// link_url = STRING_VIEW_FROM_UP_TO(35,  "<meta property=\"og:image\" content=\"")(html_buf, '"');
			// The first image
			author_title = "insta @";
			break;
		}
		case Streamable: {
			char _title[] = "*@title";
			title = find_element_attr(doc, _title, ".");
			views = STRING_VIEW_FROM_UP_TO(17, "data-videoplays=\"")(html_buf, '"');
			char _metadata[] = "*@script:data-id=player-instream";
			duration = find_element_attr(doc, _metadata, "data-duration");
			char _link_url[] = "*@meta:property=og:video:secure_url";
			link_url = find_element_attr(doc, _link_url, "content");
			break;
		}
	}
	
	set_to_string_literal_zero_if_null(timestamp);
	set_to_string_literal_zero_if_null(likes);
	set_to_string_literal_zero_if_null(views);
	set_to_string_literal_zero_if_null(duration);
	
	db_infos[0].exec(
		html_buf + html_sz,
		"UPDATE file f "
		"SET "
			"f.title=IF(f.title IS NULL OR f.title=\"\",\"", _f::esc, '"', title, "\", f.title),"
			"f.description=IF(f.description IS NULL OR f.description=\"\",\"", _f::esc, '"', description, "\", f.description),"
			"f.t_origin=IF(f.t_origin,f.t_origin,IFNULL(UNIX_TIMESTAMP(STR_TO_DATE(\"", datetime, "\",\"", datetime_fmt, "\")),", timestamp, ")),"
			"f.likes=GREATEST(IFNULL(f.likes,0),", likes, "),"
			"f.views=GREATEST(IFNULL(f.views,0),", views, "),"
			"f.duration=GREATEST(IFNULL(f.duration,0),", duration, ")"
		"WHERE f.id=", file_id
	);
	
	if ((dest_dir != nullptr) and (link_url != null_str_view)){
		// Download the linked file or web page
		// NOTE: Overwrites html_buf
		char* _itr = resulting_fp_as_output;
		compsky::asciify::asciify(_itr, dest_dir, file_id, '\0');
		char* mimetype;
		curl::dl_file(nullptr, link_url, resulting_fp_as_output, false, mimetype);
	} else {
		resulting_fp_as_output[0] = 0;
	}
}


} // namespace info_extractor
