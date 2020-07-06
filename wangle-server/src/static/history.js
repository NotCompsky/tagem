function $$$add_window_location_hash_to_history(s){
	if((s==="")||($$$document.title===""))
		return;
	const x = $$$document_getElementById('recent-pages');
	for(node of x.childNodes){
		if(node.innerText===$$$document.title)
			return;
	}
	if(x.childNodes.length===10)
		x.removeChild(x.childNodes[9]);
	x.innerHTML = "<a onclick='$$$load_page_from_a_hash_string(\""+s+"\")'>" + $$$document.title + "</a>" + x.innerHTML; // WARNING: Not escaped
}
function $$$set_window_location_hash(s){
	const S = "#" + s;
	$$$add_window_location_hash_to_history(S);
	if(S!=history.state)
		history.pushState(S, "", S);
}
function $$$unset_window_location_hash(){
	$$$set_window_location_hash("");
}
