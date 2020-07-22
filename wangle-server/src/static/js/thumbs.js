function $$$set_thumbnail_width(n){
	$$$set_cookie("w", n, 3600);
	const c = $$$document.styleSheets[0];
	c.deleteRule(0),
	c.insertRule('.thumbnail img{max-width:'+n+'px; max-height:'+n+'px;}', 0);
}
function $$$prompt_thumb_w(){
	$$$set_thumbnail_width(parseInt($$$prompt("Width")));
}
