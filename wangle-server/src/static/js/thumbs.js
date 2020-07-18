function $$$apply_thumbnail_width(){
	const n = $$$get_cookie("w");
	for(let e of $$$document_getElementsByClassName('thumb')){
		e.style.width = n+"px";
		e.style.maxHeight = n+"px";
	}
}
function $$$set_thumbnail_width(n){
	$$$set_cookie("w", n, 3600);
	$$$apply_thumbnail_width();
}
function $$$prompt_thumb_w(){
	$$$set_thumbnail_width(parseInt($$$prompt("Width")));
}
