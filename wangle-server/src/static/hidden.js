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

function $$$set_node_visibility(node,b){
	if(b)
		node.classList.remove("hidden");
	else
		node.classList.add("hidden");
}

function $$$is_node_hidden(node){
	return node.classList.contains("hidden");
}
