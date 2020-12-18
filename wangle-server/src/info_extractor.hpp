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
bool record_info(const FileIDType file_id,  const char* dest_dir,  char* const buf,  const char* url){
	const auto domain_id = get_domain_id(url);
	if (domain_id == NoDomain)
		return true;
	
	sprexer::Parser* parser = html_parser_pool.get();
	char* html_buf = buf;
	const size_t html_sz = curl::dl_buf(url, html_buf);
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
	std::string_view likes = null_str_view;
	std::string_view views = null_str_view;
	std::string_view duration = null_str_view;
	std::string_view video_url = null_str_view;
	
	switch(domain_id){
		case BBCNews: {
			char _title[] = "*@h1#main-heading";
			title = find_element_attr(doc, _title, ".");
			char _dt[] = "*@h1#main-heading>^*@time";
			datetime = find_element_attr(doc, _dt, "datetime");
			break;
		}
		case Guardian: {
			char _title[] = "*@h1";
			title = find_element_attr(doc, _title, ".");
			timestamp = STRING_VIEW_FROM_UP_TO(22, ",\"webPublicationDate\":")(html_buf, ',');
			author = STRING_VIEW_FROM_UP_TO(10, ",\"byline\":")(html_buf, '"');
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
		}
		case Streamable: {
			char _views[] = "*@script:data-id=player-instream";
			views = find_element_attr(doc, _views, "data-videoplays");
			duration = find_element_attr(doc, _views, "data-duration");
			char _video_url[] = "*@meta:property=og:video";
			video_url = find_element_attr(doc, _video_url, "content");
		}
	}
	
	set_to_string_literal_null_if_null(title);
	set_to_string_literal_null_if_null(datetime);
	set_to_string_literal_null_if_null(timestamp);
	set_to_string_literal_null_if_null(author);
	set_to_string_literal_zero_if_null(likes);
	set_to_string_literal_zero_if_null(views);
	set_to_string_literal_zero_if_null(duration);
	
	if ((dest_dir != nullptr) and (video_url != null_str_view)){
		
	}
	
	db_infos[0].exec(
		html_buf + html_sz,
		"UPDATE file f "
		"SET "
			"f.title=IFNULL(file.title,", title, "),"
			"f.t_origin=IFNULL(file.t_origin,IFNULL(", timestamp, ",UNIX_TIMESTAMP(STR_TO_DATE(", datetime, ',', datetime_fmt, ")))),"
			"f.n_likes=GREATEST(IFNULL(file.n_likes,0),", likes, "),"
			"f.n_views=GREATEST(IFNULL(file.n_views,0),", views, "),"
			"f.duration=GREATEST(IFNULL(file.duration,0),", duration, ")"
		"WHERE f.id=", file_id
	);
}


} // namespace info_extractor
