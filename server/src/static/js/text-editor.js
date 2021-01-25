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
var $$$is_editing_existing_file = false; // Either we are creating a new file, or editing the file given by file_id

function $$$view_text_editor(){
	$$$hide_all_except(['text-editor','text-edit','dirselect','tagselect-files-container']);
}

function $$$text_editor_save(){
	const title = $$$document_getElementById('text-editor').getElementsByTagName('h2')[0].innerText.replace(/\n$/,'');
	if(title===""){
		$$$alert("Must name file");
		return;
	}
	if(/[\r\n\/\\]/.test(title)){
		// WARNING: Different filesystems will require different restrictions.
		// NOTE: This is a very lazy test
		$$$alert("File name must not contain newline, carriage return, or slashes of any kind");
		return;
	}
	const tagselect = $$$document_getElementById('tagselect-files');
	const tag_ids = $$$select3__get_csv(tagselect);
	if(tag_ids === ""){
		$$$alert("Must tag file");
		return;
	}
	const _dir_id = $$$get_dirselect_value();
	if((_dir_id === null)||(_dir_id==="")){
		$$$alert("Must assign file to directory");
		return;
	}
	const _dir_name = $$$dirselect.textContent;
	if(!_dir_name.startsWith("/")){
		const b = $$$confirm("Directory is not local.\nThe contents will be saved into the description field.\nStill continue?");
		if(b!==true)
			return;
	}
	$$$ajax_POST_data_w_text_response_generic_success(
		"/f/save/"+(($$$is_editing_existing_file)?$$$file_id:0)+"/"+_dir_id+"/"+tag_ids,
		title+"\n"+$$$text_edit.innerText
	);
}
