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
	for(const id of [
		'text-editor',
		'add-f-backup-toggle-container',
		'add-f-backup',
		'f',
		'd',
		't',
		'merge-files-btn',
		'backup-files-btn',
		'view-as-playlist-btn',
		'tasks-container',
		'file-info',
		'next-f-in-playlist',
		'user-info',
		'post-container',
		'file2-container',
		'file2',
		'descr',
		'values-container',
		'tags-container',
		'parents-container',
		'children-container',
		'dirselect-container',
		'tagselect-files-container',
		'tagselect-files-btn',
		'tagselect-era-container',
		'tagselect-eras-container',
		'tagselect-dirs-container',
		'tagselect-self-p-container',
		'tagselect-self-p-btn',
		'tagselect-self-c-container',
		'tagselect-self-c-btn',
		'add-t-dialog',
		'add-f-dialog',
		'add-d-dialog',
		'add-D-dialog',
		'orig-src-dialog'
	]){
		if(ids.includes(id))
			$$$unhide(id);
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
