// Copyright 2020 Adam Gray
// This file is part of the tagem program.
// The tagem program is free software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published by the
// Free Software Foundation version 3 of the License.
// The tagem program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// This copyright notice should be included in any copy or substantial copy of the tagem source code.
// The absense of this copyright notices on some other files in this project does not indicate that those files do not also fall under this license, unless they have a different license written at the top of the file.

function $$$hide_node(x){
	x.classList.add("hidden");
}
function $$$unhide_node(x){
	x.classList.remove("hidden");
}
function $$$toggle_node(x){
	x.classList.toggle("hidden");
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
	$$$select2_ids = $$$select2_ids_default;
	if(!ids.includes('file-info'))
		$$$playlist_file_ids = undefined; // Destroy playlist
	$$$document_getElementById_profile_img.removeAttribute('onclick');
	$$$hide_class('help');
	for(const id of [
		'qry-textarea',
		'admin-dashboard',
		'text-editor',
		'text-edit',
		'add-f-backup-ytdl-container',
		'audio-only-container',
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
		'meta',
		't-added',
		't1',
		't2',
		'descr',
		'values-container',
		'tags-container',
		'parents-container',
		'siblings-container',
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
		'tagselect-self-s-container',
		'tagselect-self-s-btn',
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
		'repeatness',
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
	$$$toggle_node($$$document_getElementById(id));
}
function $$$toggle_class(x){
	$$$for_node_in_document_getElementsByClassName(x, $$$toggle_node);
}
