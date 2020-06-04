"function populate_f_table(url, selector){"
	"get_json(url, function(data){"
		"var s = \"\";"
		"for (var ls of data){"
			"s += \"<div class='tr' data-id='\" + ls[1] + \"'>\";"
				"s += '<div class=\"td\"><img class=\"thumb\" src=\"' + ls[0] + '\"></img></div>';"
				//"s += \"<td><a href='/d#\" + ls[1] + \"'>\" + ls[2] + \"</a></td>\";" // Dir  ID and name
				"s += \"<div class='td fname'><a href='/f#\" + ls[1] + \"'>\" + ls[2] + \"</a></div>\";" // File ID and name
				"s += \"<div class='td'>\" + ls[3] + \"</div>\";"
				
			"s += \"</div>\";"
		"}"
		"document.querySelector(selector).innerHTML = s;"
		"column_id2name('t', \"/a/t.json\", selector, '/t#', 2);"
	"});"
"}"
"function tag_files_then(file_ids, selector, fn){"
	"const tagselect = $(selector);"
	"const tag_ids = tagselect.val();"
	"$.post({"
		"url: \"/f/t/\" + file_ids + \"/\" + tag_ids.join(\",\"),"
		"success: function(){"
			"tagselect.val(\"\").change();" // Deselect all
			"fn(file_ids, tag_ids);"
		"},"
		"error: function(){"
			"alert(\"Error tagging file\");"
		"}"
	"});"
"}"

"function filter_f_tbl(selector){"
	"filter_tbl(selector, [1], 2);"
"}"

"function set_embed_html(selector, device_id, dir_name, file_name){"
	"set_var_to_json_then('D', \"/a/D.json\", function(){"
		"const embed_pre = D[device_id][2];"
		"if (embed_pre === \"\"){"
			
			"var src;"
			"if (dir_name.startsWith(\"/\")){"
				"src = \"/S/f/\" + file_id;"
			"} else {"
				"src = dir_name + file_name;"
			"}"
			
			"if (/.(jpe?g|png|gif)$/.exec(file_name) !== null){"
				"document.querySelector(selector).innerHTML = \"<img src='\" + src + \"'/>\";"
			"} else {"
				"document.querySelector(selector).innerHTML = \"<video controls><source type='\" + mimetype + \"' src='\" + src + \"'></source></video>\";"
			"}"
			
			"$(selector).attr(\"onclick\", \"\");"
		"} else {"
			"document.querySelector(selector).innerHTML = embed_pre + file_name + D[device_id][3];"
		"}"
	"});"
"}"
