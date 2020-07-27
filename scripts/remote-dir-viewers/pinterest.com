#!/usr/bin/env bash


declare -i page="$1"
qry="$(echo "$2" | sed 's/^pinterest //g')"


echo "$qry" > /tmp/qry.txt


dir_id=388054

printf "%s" "\"$dir_id\",["



rawurlencode(){
	# Copyright Orwellophile 2012 under the CC BY-SA 3.0 license: https://creativecommons.org/licenses/by-sa/3.0/
	# src: https://stackoverflow.com/questions/296536/how-to-urlencode-data-for-curl-command
	# No substantial changes were made, aside from presentation (indentation) and selecting only one method of returning a result
	
	local string="${1}"
	local strlen=${#string}
	local encoded=""
	local pos c o

	for (( pos=0 ; pos<strlen ; pos++ )); do
		c=${string:$pos:1}
		case "$c" in
			[-_.~a-zA-Z0-9] ) o="${c}" ;;
			* )               printf -v o '%%%02x' "'$c"
		esac
		encoded+="${o}"
	done
	echo "${encoded}"
}


while read -r id; do
	read -r thumb_src
	read -r grid_title
	
	img_filename="$id"
	title="$grid_title"
	
	printf "%s" "[$thumb_src,0,$img_filename,$grid_title,0,0,0,0,0,0,0,0,0,0,\"\",\"\"],"
done < <(curl "https://www.pinterest.co.uk/resource/BaseSearchResource/get/?data=%7B%22options%22%3A%7B%22isPrefetch%22%3Afalse%2C%22query%22%3A%22$(rawurlencode "$qry")%22%2C%22scope%22%3A%22pins%22%7D%2C%22context%22%3A%7B%7D%7D" -H "User-Agent: ${USER_AGENT}" -H 'Accept: application/json, text/javascript, */*, q=0.01' -H 'Accept-Language: en-GB,en;q=0.5' --compressed -H 'X-Requested-With: XMLHttpRequest' -H 'X-APP-VERSION: 361966b' -H 'X-Pinterest-AppState: active' -H 'DNT: 1' -H 'Referer: https://www.pinterest.co.uk/' -H 'Connection: keep-alive' -H "Cookie: ${PINTEREST_COOKIES}" -H 'TE: Trailers' | jq '.resource_response.data.results[] | .id, .images["170x"].url, .grid_title')
