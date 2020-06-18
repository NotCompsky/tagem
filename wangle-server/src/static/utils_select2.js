"function guess_parenty_thing_from_name(var_name, s){"
	"return Object.entries(window[var_name]).map(([id, tpl]) => [id, tpl[0]]).filter(x => s.startsWith(x[1])).sort(function(a,b){return b[1].length-a[1].length;})[0];"
	// Use something like $('#dirselect').val(KEY).trigger('change') to update select2 selection box
"}"
