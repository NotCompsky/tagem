"function guess_parenty_thing_from_name(var_name, s){"
	"return Object.entries(window[var_name]).map(([id, tpl]) => [tpl[0], id]).filter(x => s.startsWith(x[0])).sort().reverse()[0][1];"
	// Use something like $('#dirselect').val(KEY).trigger('change') to update select2 selection box
"}"
