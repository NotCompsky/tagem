function $$$view_db(id){
	// TODO: Introduce a good external DB overview page
	$$$view_qry('f x "' + $$$x[id] + '"');
}

function $$$display_external_db(id, name, post_id){
	return "<a class='db-link' onclick='$$$view_post(" + id + ",\"" + post_id + "\")'>View post on " + name + "</a>"; // post_id is enclosed in quotes because Javascript uses doubles for integers and rounds big integers
	// NOTE: No need to $$$escape_html_text(name), as name is already restricted due to SQL table name restrictions
}
function $$$display_external_dbs(db_and_post_ids){
	let s = "";
	for (var i = 0;  i < db_and_post_ids.length;  ++i){
		s += $$$display_external_db(db_and_post_ids[i][0], $$$x[db_and_post_ids[i][0]], db_and_post_ids[i][1]) + "<br/>";
	}
	$$$document_getElementById("db-links").innerHTML = s;
}

function $$$get_external_db_id_from_name(s){
	return Object.entries(x).filter(x => x[1]==s)[0][0];
}

function $$$display_cmnts(_db_id, ls){
	// ls is an array of: [cmnt_id, parent_id, user_id, timestamp, cmnt_content], with parent_id==0 first (order by (parent_id=0) ASC)
	let tree = {"0":["0",0,""]};
	for (const [cmnt_id, parent_id, user_id, timestamp, username, cmnt_content] of ls){
		const _s = '' +
			'<div id="c' + cmnt_id + '" class="cmnt">' +
				'<div class="head">' +
					'<a class="user" onclick="$$$view_user(' + _db_id + ',\'' + user_id + '\')">' + username + '</a>' + // Encasing ID in quotes because Javascript rounds large numbers
					'<time data-t="' + timestamp + '">' + $$$timestamp2dt(timestamp) + '</time>' +
					'<button class="del" onclick="$$$rm_cmnt(' + _db_id + ',\'' + cmnt_id + '\')">Del</button>' +
				'</div>' +
				'<p>' +
					cmnt_content +
				'</p>' +
				'<div class=\'replies\'>'
		;
			// Needs a terminating '</div>'
		tree[cmnt_id] = [parent_id, 0, _s];
		if(tree[parent_id]===undefined){
			console.log("tree[",parent_id,"] undefined");
			continue;
		}
		++tree[parent_id][1];
	}
	let s = "";
	bigloopboi:
	while(true){
		for(const [key, [parent,n_children,_s]] of Object.entries(tree)){
			if(n_children !== 0)
				continue;
			if(tree[parent]===undefined){
				tree[0][2] += _s + '</div></div>';
			}else{
				tree[parent][2] += _s + '</div></div>';
				--tree[parent][1];
			}
			if(key === "0")
				break bigloopboi;
			delete tree[key];
		}
	}
	$$$document_getElementById('cmnts').innerHTML = tree[0][2];
	$$$unhide("cmnts");
}

function $$$display_post_meta(_db_id, tpl){
	const [user, t, n_likes, username, txt] = tpl;
	$$$document_getElementById('post-user').onclick = function(){$$$view_user(_db_id, user)};
	// I don't know much about Javascript's memory management, but _db_id - although a local parameter of the function within which the function is created - seems to be preserved
	$$$document_getElementById('post-user').innerText = username;
	$$$document_getElementById('post-time').innerText = $$$timestamp2dt(t);
	$$$document_getElementById('post-text').innerText = txt;
}

function $$$display_posts(_db_id, _user_id, _type){
	// _type is either 'l' (for liked posts) or 'u' (for posts which they authored)
	$$$ajax_GET_w_JSON_response('!!!MACRO!!!SERVER_ROOT_URL/a/x/u/p/'+_type+'/'+_db_id+'/'+_user_id, function(file_ids){
		if(file_ids.length === 0)
			return;
		$$$populate_f_table('id', file_ids.join(","), null, 0);
		for(let _ of ['f','f-action-btns','tagselect-files-container','tagselect-files-btn','merge-files-btn','backup-files-btn','view-as-playlist-btn'])
			$$$unhide(_); // _ <= ['f','f-action-btns','tagselect-files-container','tagselect-files-btn','merge-files-btn','backup-files-btn','view-as-playlist-btn']
		$$$get_file_ids = $$$get_selected_file_ids;
	});
}

function $$$view_post(_db_id, _post_id){
	$$$ajax_GET_w_JSON_response("!!!MACRO!!!SERVER_ROOT_URL/a/x/p/i/"+_db_id+"/"+_post_id, function(data){
		$$$display_post_meta(_db_id, data[0]);
		$$$display_cmnts(_db_id, data[1]);
		$$$db_id = _db_id;
		$$$post_id = _post_id;
		$$$hide('likes');
		$$$unhide('post-container');
	});
}

function $$$rm_cmnt(_db_id, cmnt_id){
	$$$ajax_POST_w_text_response("/x/c/rm/"+_db_id+"/"+cmnt_id, function(){
		$$$document_getElementById('c'+cmnt_id).classList.add('hidden');
	});
}

function $$$view_user(_db_id, _user_id){
	$$$hide_all_except(['tags-container','user-info']);
	$$$set_window_location_hash('x' + $$$db_id + '/u' + $$$user_id);
	if (_db_id !== undefined){
		$$$user_id = _user_id;
		$$$db_id = _db_id;
		$$$ajax_GET_w_JSON_response(
			"!!!MACRO!!!SERVER_ROOT_URL/a/x/u/i/"+$$$db_id+"/"+_user_id,
			function(data){
				$$$set_profile_thumb($$$BLANK_IMG_SRC);
				
				$$$document_getElementById('user-fullname').textContent = data[1];
				
				if (data[2])
					$$$unhide('verified');
				else 
					$$$hide('verified');
				
				$$$document_getElementById('n-followers').textContent = data[3] + ' followers';
				
				$$$display_tags($$$split_on_commas_but_make_empty_if_empty(data[4]), "tags", 'x/u');
				
				$$$user_name = data[0];
				$$$set_profile_name($$$user_name);
			}
		);
	} else {
		$$$set_profile_name($$$user_name);
	}
}

function $$$view_likes(){
	if ($$$file_id === undefined)
		return;
	$$$ajax_GET_w_JSON_response("!!!MACRO!!!SERVER_ROOT_URL/a/x/p/l/"+$$$db_id+"/"+$$$post_id, function(data){
		let _s = "";
		for(const [id,name] of data){
			_s += '<a class="user" onclick="$$$view_user(' + $$$db_id + ',\'' + id + '\')">' + name + '</a>';
		}
		$$$document_getElementById('likes-ls').innerHTML = _s;
		$$$unhide('likes');
	});
}
