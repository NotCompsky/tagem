function $$$set_thumbnail_width(n){
	$$$set_cookie("w", n, 3600);
	$$$document_getElementById('css-thumb-sz').remove();
	const x = $$$document.createElement('style');
	x.id = 'css-thumb-sz';
	x.innerHTML='.thumbnail img{max-width:'+n+'px; max-height:'+n+'px;}';
	$$$document.head.appendChild(x);
}
function $$$prompt_thumb_w(){
	$$$set_thumbnail_width(parseInt($$$prompt("Width")));
}
