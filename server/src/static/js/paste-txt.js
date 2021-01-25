function $$$paste_txt(node,e){
	e.preventDefault();
	document.execCommand("insertHTML", false, e.clipboardData.getData("text/plain"));
}
