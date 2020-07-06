const $$$Mousetrap_bind   = Mousetrap.bind;
const $$$Mousetrap_unbind = Mousetrap.unbind;
const $$$document = document;
const $$$document_getElementById = $$$document.getElementById.bind($$$document);
const $$$document_getElementsByClassName = $$$document.getElementsByClassName.bind($$$document);
const $$$confirm = confirm;
const $$$prompt = prompt;
const $$$window_location = window.location;
function $$$add_window_location_hash_to_history(){
	if(($$$window_location.hash==="")||($$$document.title===""))
		return;
	const x = $$$document_getElementById('recent-pages');
	for(node of x.childNodes){
		if(node.innerText===$$$document.title)
			return;
	}
	if(x.childNodes.length===10)
		x.removeChild(x.childNodes[9]);
	x.innerHTML = "<a onclick='$$$load_page_from_a_hash_string(\""+$$$window_location.hash+"\")'>" + $$$document.title + "</a>" + x.innerHTML; // WARNING: Not escaped
}
function $$$set_window_location_hash(s){
	$$$add_window_location_hash_to_history();
	$$$window_location.hash = s;
}
function $$$unset_window_location_hash(){
	$$$set_window_location_hash("");
}
function $$$for_node_in_document_getElementsByClassName(s, fn){
	for(let e of $$$document_getElementsByClassName(s))
		fn(e);
}
