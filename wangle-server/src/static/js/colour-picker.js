const $$$css_colour_name2indx = {"fg":1, "bg":2};

function $$$set_css_colour(name,val){
	if(val===undefined)
		return;
	$$$set_cookie("css_colour_"+name, val, 3600);
	const c = $$$document.styleSheets[0];
	c.deleteRule($$$css_colour_name2indx[name]),
	c.insertRule('.' + name + '{background-color:'+val+';}', $$$css_colour_name2indx[name]);
}

function $$$on_css_colour_picker_change(e){
	const x = e.target;
	$$$set_css_colour(x.dataset.for,x.value);
}
