var $$$dummy_text_node = $$$document.createElement('div');
function $$$escape_html_text(s){
	$$$dummy_text_node.textContent = s;
	return $$$dummy_text_node.innerHTML;
}
