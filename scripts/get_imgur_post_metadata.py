'''
Copyright 2020 Adam Gray
This file is part of the tagem program.
The tagem program is free software: you can redistribute it and/or
modify it under the terms of the GNU General Public License as published by the
Free Software Foundation version 3 of the License.
The tagem program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
This copyright notice should be included in any copy or substantial copy of the tagem source code.
The absense of this copyright notices on some other files in this project does not indicate that those files do not also fall under this license, unless they have a different license written at the top of the file.
'''
#!/usr/bin/env python3


from datetime import datetime as dt
import re
import requests


headers = {
    'User-Agent': 'Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:78.0) Gecko/20100101 Firefox/78.0',
    'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8',
    'Accept-Language': 'en-GB,en;q=0.5',
    'Connection': 'keep-alive',
    'Upgrade-Insecure-Requests': '1',
    'DNT': '1',
    'TE': 'Trailers',
}


def get_page_metadata(id, cookies):
	r = requests.get(url if id is None else f"https://imgur.com/{id}", headers=headers, cookies=cookies)
	m1 = re.search(""","views":"([0-9]+)",""", r.text)
	m2 = re.search(""","datetime":"([^"]+)",""", r.text)
	
	return {
		"timestamp": int(dt.strptime(m2.group(1), "%Y-%m-%d %H:%M:%S").timestamp()),
		"view_count": m1.group(1),
		"uploader": None, # Use gallery interface to get this - and much more - info
	}
	'''
	r = requests.get(f"https://imgur.com/gallery/{id}/comment/best/hit.json", headers=headers, cookies=cookies)
	d = r.json()["data"]["image"]
	
	return {
		"likes_count": d["ups"],
		"dislikes_count": d["downs"],
		"description": d["description"],
		"datetime": d["timestamp"],
		"comment_count": d["comments"],
		"nsfw": d["nsfw"],
		"view_count": d["views"],
		"uploader": d["account_url"],
	}'''


if __name__ == "__main__":
	import sys
	import json
	
	cookies = json.loads(sys.argv[1])
	
	for url in sys.argv[2:]:
		m = re.search("^https?://(?:i[.])?imgur[.]com/([^/]+)", url)
		if m is None:
			raise ValueError(f"Not a recognised Imgur URL: {url}")
		json.dumps(get_page_metadata(id=m.group(1)), cookies)
