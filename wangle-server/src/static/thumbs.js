function $$$set_thumbnail_width(n){
	for(let e of $$$document_getElementsByClassName('thumb')){
		e.style.width = n+"px";
		e.style.maxHeight = n+"px";
	}
	$$$set_cookie("w", n, 3600);
}
function $$$prompt_thumb_w(){
	$$$set_thumbnail_width(parseInt(prompt("Width")));
}
