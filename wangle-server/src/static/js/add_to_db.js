function $$$obj_type2parent_type(obj_type){
	switch(obj_type){
		case 'f':
			return 'd';
		case 'd':
			return 'D';
		case 'D':
			return 'P';
	}
}

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
	}
}

function $$$add_to_db(obj_type){
	const queue = $$$document_getElementById('add-'+obj_type+'-queue');
	const parent_type = $$$obj_type2parent_type(obj_type);
	
	if(obj_type==='t'){
		const tag_names = [];
		queue.innerText.replace(/(?:^|\n)([^\n]+)/g, function(group0, group1){
			tag_names.push(group1);
		});
		if(tag_names.length===0)
			return;
		const tagselect = $('#tagselect-self-p');
		const parent_ids = tagselect.val();
		if(parent_ids.length === 0)
			return;
		$$$ajax_POST_data_w_text_response("/t/add/"+parent_ids.join(",")+"/", tag_names.join("\n"), function(){
			tagselect.val("").change();
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
	const tagselect = $('#tagselect-files');
	const tag_ids = tagselect.val();
	if(tag_ids.length === 0){
		// TODO: Replace with confirmation dialog
		$$$alert("No tags");
		return;
	}
	
	const backup_dir_id = $$$get_dir_id_to_backup_into();
	if(backup_dir_id===""){
		$$$alert("Backup requested, but no directory selected");
		return;
	}
	
	$$$ajax_POST_data_w_text_response(
		"/" + obj_type + "/add/" + tag_ids.join(",")+"/" + backup_dir_id + "/" + $$$is_ytdl_checked(), // is_ytdl_checked is meaningless for anything other than files. Safe to include though.
		urls.join("\n"),
		function(){
			tagselect.val("").change();
			queue.innerHTML = ""; // Remove URLs
			$$$alert("Success");
			if((obj_type!=='f')&&(obj_type!=='d'))
				$$$refetch_json(obj_type, '!!!MACRO!!!SERVER_ROOT_URL/a/'+obj_type+'.json');
		}
	);
}

function $$$get_dir_id_to_backup_into(){
	const x = $$$document_getElementById("add-f-backup-toggle");
	if($$$is_node_hidden(x)||(!x.checked))
		return "0";
	return $('#dirselect').val();
}

function $$$is_ytdl_checked(){
	return $$$document_getElementById('add-f-backup-ytdl').checked ? 1 : 0;
}

function $$$add_to_db__append(obj_type){
	const inp = $$$document_getElementById('add-' + obj_type + '-input');
	const x = inp.value;
	if(x === ""){
		$$$alert("Enter a tag or URL");
		return;
	}
	const parent_type = $$$obj_type2parent_type(obj_type);
	
	$$$document_getElementById('add-' + obj_type + '-queue').innerText += "\n" + x;
	inp.value = "";
}
