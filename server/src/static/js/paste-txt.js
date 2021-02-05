function $$$paste_txt(node,e){
	e.preventDefault();
	document.execCommand("insertText", false, e.clipboardData.getData("text/plain"));
}
