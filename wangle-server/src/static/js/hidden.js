function $$$hide_node(x){
	x.classList.add("hidden");
}
function $$$unhide_node(x){
	x.classList.remove("hidden");
}

function $$$hide(id){
	$$$hide_node($$$document_getElementById(id));
}
function $$$unhide(id){
	$$$unhide_node($$$document_getElementById(id));
}

function $$$hide_class(s){
	$$$for_node_in_document_getElementsByClassName(s, $$$hide_node);
}
function $$$unhide_class(s){
	$$$for_node_in_document_getElementsByClassName(s, $$$unhide_node);
}

function $$$set_visibility(id,b){
	$$$set_node_visibility($$$document_getElementById(id),b);
}

function $$$set_node_visibility(node,b){
	if(b)
		node.classList.remove("hidden");
	else
		node.classList.add("hidden");
}

function $$$is_node_hidden(node){
	return node.classList.contains("hidden");
}


function $$$hide_all_except(ids,classes){
	// More like 'page switch' event now
	if(!ids.includes('file-info'))
		$$$playlist_file_ids = undefined; // Destroy playlist
	$$$document_getElementById('profile-img').removeAttribute('onclick');
	$$$hide_class('help');
	for(const id of $$$ALL_HIDDEN_IDS){
		if(ids.includes(id))
			$$$unhide(id); // id <= []
		else 
			$$$hide(id);
	}
	for(const c of [
		'file-meta'
	]){
		if((classes===undefined)||(!classes.includes(c)))
			$$$hide_class(c);
		else
			$$$unhide_class(c);
	}
}
function $$$is_visible(id){
	return (!$$$document_getElementById(id).classList.contains('hidden'));
}
function $$$toggle(id){
	$$$document_getElementById(id).classList.toggle("hidden");
}
