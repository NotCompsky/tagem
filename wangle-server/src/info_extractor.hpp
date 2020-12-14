#pragma once

#include "db_info.hpp"
#include "thread_pool.hpp"
#include "sprexer/element.hpp"
#include "sprexer/doc.hpp"
#include "sprexer/parser.hpp"
#include "sprexer/nullstr.hpp"


#define SKIP_TO_AFTER_SUBSTR__NONULLTERMINATE(length,name) \
	/* Returns a pointer to the character immediately AFTER the substr */ \
	[](const char* str)->const char* { \
		while(true){ \
			if (likely(not IS_STR_EQL(str,length,name))) continue; \
			return str; \
		} \
	}
#define STRING_VIEW_FROM_UP_TO(length,name) \
	[](const char* str,  const char c)->const std::string_view { \
		str = SKIP_TO_AFTER_SUBSTR__NONULLTERMINATE(length,name)(str); \
		const char* const begin = str; \
		while(*str != c) \
			++str; \
		return std::string_view(begin,  (uintptr_t)str - (uintptr_t)begin); \
	}


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


enum DomainID {
	NoDomain,
	
	BBCNews,
	Guardian,
	Telegraph,
	Indep,
	DailyMail,
	TheTimes,
	SkyNews,
	
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

std::string_view find_element_attr(const sprexer::Doc& doc,  char* const selector_path,  const char* const attr){
	sprexer::Element element(doc.get_element_from_class_selector_path(selector_path));
	if (element.is_null())
		return null_str_view;
	return element.get_value(attr);
}

template<typename FileIDType>
bool record_info(const FileIDType file_id,  const char* user_headers,  char* const html_buf,  const char* url){
	const auto domain_id = get_domain_id(url);
	if (domain_id == NoDomain)
		return true;
	
	sprexer::Parser* parser = html_parser_pool.get();
	const size_t html_sz = curl::dl_buf(user_headers, url, html_buf);
	sprexer::Doc doc(*parser, html_buf, html_sz);
	html_parser_pool.free(parser);
	
	std::string_view title = null_str_view;
	std::string_view datetime = null_str_view;
	const char* datetime_fmt = "%Y-%m-%dT%H:%i:%S.%fZ";
		/* 
		 * This default value is of the ISO datetime format
		 * e.g. 2020-12-14T14:04:50.000Z
		 * In UNIX format it would be "%Y-%m-%dT%H:%M:%S.%fZ";
		 */
	std::string_view timestamp = null_str_view;
	std::string_view author = null_str_view;
	
	printf(">>>>>\n%.*s\n<<<<<", (int)html_sz, html_buf); fflush(stdout);
	
	switch(domain_id){
		case BBCNews: {
			printf("BBCNews\n"); fflush(stdout);
			char _title[] = "*@h1#main-heading";
			title = find_element_attr(doc, _title, ".");
			char _dt[] = "*@h1#main-heading>^*@time";
			datetime = find_element_attr(doc, _dt, "datetime");
			break;
		}
		case Guardian: {
			printf("Guardian\n"); fflush(stdout);
			char _title[] = "*@h1";
			title = find_element_attr(doc, _title, ".");
			timestamp = STRING_VIEW_FROM_UP_TO(22, ",\"webPublicationDate\":")(html_buf, ',');
			author = STRING_VIEW_FROM_UP_TO(10, ",\"byline\":")(html_buf, '"');
			break;
		}
	}
	
	set_to_string_literal_null_if_null(title);
	set_to_string_literal_null_if_null(datetime);
	set_to_string_literal_null_if_null(timestamp);
	set_to_string_literal_null_if_null(author);
	
	db_infos[0].exec(
		html_buf + html_sz,
		"UPDATE file f "
		"SET "
			"f.title=IFNULL(file.title,", title, "),"
			"f.t_origin=IFNULL(file.t_origin,IFNULL(", timestamp, ",UNIX_TIMESTAMP(STR_TO_DATE(", datetime, ',', datetime_fmt, ")))),"
		"WHERE f.id=", file_id
	);
}


} // namespace info_extractor
