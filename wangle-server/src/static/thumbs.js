function $$$set_thumbnail_width(n){
	for(let e of document.getElementsByClassName('thumb'))
		e.style.width = n+"px";
	$$$set_cookie("w", n, 3600);
}
function $$$prompt_thumb_w(){
	$$$set_thumbnail_width(parseInt(prompt("Width")));
}
