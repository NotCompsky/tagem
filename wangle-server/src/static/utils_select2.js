"function guess_parenty_thing_from_name(parent_var_name, s){"
	"if(parent_var_name==='P')"
		"return Object.entries(window[parent_var_name]).map(([id, tpl]) => [id, tpl]).filter(x => s.startsWith(x[1])).sort(function(a,b){return b[1].length-a[1].length;})[0];"
	"else "
		"return Object.entries(window[parent_var_name]).map(([id, tpl]) => [id, tpl[0]]).filter(x => s.startsWith(x[1])).sort(function(a,b){return b[1].length-a[1].length;})[0];"
	// Use something like $('#dirselect').val(KEY).trigger('change') to update select2 selection box
"}"
