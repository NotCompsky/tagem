"function add_files_dialog(){"
	"hide_all_except(['dirselect-container','tagselect-files-container','add-files-dialog']);"
"}"

"function add_file(){"
	"const queue = document.getElementById('add-files-queue');"
	"const urls = Array.from(queue.getElementsByTagName('ul')).map(x => x.textContent);"
	"if(urls.length===0)"
		"return;"
	"const tagselect = $('#tagselect-files');"
	"const tag_ids = tagselect.val();"
	"if(tag_ids.length === 0)"
		"return;"
	"const dir = document.getElementById(\"dirselect-container\").value;"
	"const _dir_name = d[parseInt(dir)][0];"
	"if(dir === \"\"){"
		"alert(\"Please set the directory - likely a common prefix of all the URLs\");"
		"return;"
	"}"
	"for(const url of urls){"
		"if(!url.startsWith(_dir_name)){"
			"const b = confirm(\"The directory '\" + _dir_name + \"' is not a common prefix of all the URLs. Still proceed?\");"
			"if(!b)"
				"return;"
		"}"
	"}"
	"$.ajax({"
		"type:\"POST\","
		"url:\"http://localhost:1999/add-f/\" + tag_ids.join(\",\") + \"/\" + dir,"
		"data:urls.join(\"\\n\"),"
		"success:function(){"
			"tagselect.val(\"\").change();"
			"queue.innerHTML = \"\";" // Remove URLs
			"alert(\"Success\");"
		"},"
		"dataType:\"text\""
	"});"
"}"

"function add_files__append(){"
	"const inp = document.getElementById('add-file-input');"
	"const x = inp.value;"
	"if(x !== \"\")"
		"document.getElementById('add-files-queue').innerHTML += \"<ul>\" + inp.value + \"</ul>\";"
	"inp.value = \"\";"
"}"
