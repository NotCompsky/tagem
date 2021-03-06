// 
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
// 
function $$$nickname2fullname(obj_type){
	switch(obj_type){
		case 'f':
			return 'file';
		case 'd':
			return 'directory';
		case 'D':
			return 'device';
		case 'P':
			return 'protocol';
		case 't':
			return 'tag';
	}
}

function $$$add_to_db(obj_type){
	const queue = $$$document_getElementById('add-'+obj_type+'-queue');
	
	if(obj_type==='t'){
		const tag_names = [];
		queue.innerText.replace(/(?:^|\n)([^\n]+)/g, function(group0, group1){
			tag_names.push(group1);
		});
		if(tag_names.length===0)
			return;
		const tagselect = $$$document_getElementById('tagselect-self-p');
		const parent_ids = $$$select3__get_csv(tagselect);
		if(parent_ids === "")
			return;
		$$$ajax_POST_data_w_text_response("/t/add/"+parent_ids+"/", tag_names.join("\n"), function(){
			$$$select3__wipe_values(tagselect);
			queue.innerHTML = ""; // Remove URLs
			$$$alert("Success");
		});
		return;
	}
	
	const urls = [...queue.innerText.matchAll(/[^\n]+/g)];
	if(urls.length===0){
		$$$alert("No URLs");
		return;
	}
	const tagselect = $$$document_getElementById('tagselect-files');
	const tag_ids = $$$select3__get_csv(tagselect);
	if(tag_ids === ""){
		// TODO: Replace with confirmation dialog
		$$$alert("No tags");
		return;
	}
	
	const backup_dir_id = $$$get_dir_id_to_backup_into();
	if((obj_type==='f')&&(backup_dir_id===""||backup_dir_id===null)){
		$$$alert("Backup requested, but no directory selected");
		return;
	}
	
	$$$ajax_POST_data_w_text_response(
		obj_type + "/add/" + tag_ids+"/" + backup_dir_id + "/" + $$$is_ytdl_checked() + "/" + $$$is_audio_only_checked(), // is_ytdl_checked etc is meaningless for anything other than files. Safe to include though.
		urls.join("\n"),
		function(){
			$$$select3__wipe_values(tagselect);
			queue.innerHTML = ""; // Remove URLs
			$$$alert("Success");
			if((obj_type!=='f')&&(obj_type!=='d'))
				$$$refetch_json(obj_type, 'a/'+obj_type+'.json');
		}
	);
}

function $$$get_dir_id_to_backup_into(){
	const x = $$$document_getElementById_add_f_backup_toggle;
	if($$$is_node_hidden(x)||(!x.checked))
		return "0";
	return $$$get_dirselect_value();
}

function $$$uncheck_dl_locally(){
	$$$document_getElementById_add_f_backup_toggle.checked = false;
}

function $$$is_ytdl_checked(){
	return $$$document_getElementById_add_f_backup_ytdl.checked ? 1 : 0;
}

function $$$is_audio_only_checked(){
	return $$$document_getElementById_audio_only.checked ? 1 : 0;
}

function $$$reset_audio_only_checked(){
	$$$document_getElementById_audio_only.checked = false;
}

function $$$add_to_db__append(obj_type){
	const inp = $$$document_getElementById('add-' + obj_type + '-input');
	const x = inp.value;
	if(x === ""){
		$$$alert("Enter a tag or URL");
		return;
	}
	$$$document_getElementById('add-' + obj_type + '-queue').innerText += "\n" + x;
	$$$select3__wipe_values(inp);
}
