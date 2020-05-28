#pragma once

namespace protocol {
	enum {
		NONE,
		local_filesystem,
		http,
		https,
		youtube_dl,
		COUNT
	};
	constexpr static
	const char* const strings[COUNT] = {
		"NONE",
		"file://",
		"http://",
		"https://",
		"youtube-dl"
	};
}
