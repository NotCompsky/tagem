function $$$display_cmnts(_db_id, ls){
	// ls is an array of: [cmnt_id, parent_id, user_id, timestamp, cmnt_content], with parent_id==0 first (order by (parent_id=0) ASC)
	let tree = {"0":["0",0,""]};
	for (const [cmnt_id, parent_id, user_id, timestamp, username, cmnt_content] of ls){
		const _s = '' +
			'<div id="c' + cmnt_id + '" class="cmnt">' +
				'<div class="head">' +
					'<a class="user" onclick="$$$view_user(' + _db_id + ',\\'' + user_id + '\\')">' + username + '</a>' + // Encasing ID in quotes because Javascript rounds large numbers
					'<time data-t="' + timestamp + '">' + $$$timestamp2dt(timestamp) + '</time>' +
					'<button class="del" onclick="$$$rm_cmnt(' + _db_id + ',\\'' + cmnt_id + '\\')">Del</button>' +
				'</div>' +
				'<p>' +
					cmnt_content +
				'</p>' +
				'<div class=\\'replies\\'>'
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
	$$$unhide('f');
	$.ajax({
		dataType: "json",
		url: '/a/x/u/p/' + _type + '/' + _db_id + '/' + _user_id,
		success: function(file_ids){
			if(file_ids.length === 0)
				return;
			$$$populate_f_table('/a/f/id/' + file_ids.join(","));
		},
		error:err_alert
	});
}

function $$$view_post(_db_id, _post_id){
	$$$unhide('post-container');
	$.ajax({
		dataType: "json",
		url: "/a/x/p/i/"+_db_id+"/"+_post_id,
		success: function(data){
			$$$display_post_meta(_db_id, data[0]);
			$$$display_cmnts(_db_id, data[1]);
			$$$db_id = _db_id;
			$$$post_id = _post_id;
			$$$hide('likes');
		},
		error:$$$err_alert
	});
}

function $$$rm_cmnt(_db_id, cmnt_id){
	$.ajax({
		type:"POST",
		dataType: "text",
		url: "/x/c/rm/"+_db_id+"/"+cmnt_id,
		success: function(){
			$$$document_getElementById('c'+cmnt_id).classList.add('hidden');
		},
		error:$$$err_alert
	});
}

function $$$view_user(_db_id, _user_id){
	$$$hide_all_except(['tags-container','user-info']);
	
	if (_db_id !== undefined){
		$$$user_id = _user_id;
		$$$db_id = _db_id;
		$.ajax({
			dataType: "json",
			url: "/a/x/u/i/"+$$$db_id+"/"+_user_id,
			success: function(data){
				$$$document_getElementById('profile-img').src = "";
				
				$$$document_getElementById('user-fullname').textContent = data[1];
				
				if (data[2])
					$$$unhide('verified');
				else 
					$$$hide('verified');
				
				$$$document_getElementById('n-followers').textContent = data[3] + ' followers';
				
				const tags = data[4];
				if (tags === "")
					$$$display_tags([], "#tags");
				else 
					$$$display_tags(tags.split(","), "#tags");
				
				$$$user_name = data[0];
				$('#profile-name').text($$$user_name);
			},
			error:$$$err_alert
		});
	} else {
		$('#profile-name').text($$$user_name);
	}
	
	window.location.hash = 'x' + $$$x[db_id] + '/u' + $$$user_id; // db_id is actually the index of the database in the server's runtime command arguments, and could therefore easily change. Hence using the database name instead.
}

function $$$view_likes(){
	if ($$$file_id === undefined)
		return;
	$.ajax({
		dataType: "json",
		url: "/a/x/p/l/"+$$$db_id+"/"+$$$post_id,
		success: function(data){
			let _s = "";
			for(const [id,name] of data){
				_s += '<a class="user" onclick="$$$view_user(' + $$$db_id + ',\\'' + id + '\\')">' + name + '</a>';
			}
			$$$document_getElementById('likes-ls').innerHTML = _s;
			$$$unhide('likes');
		},
		error:$$$err_alert
	});
}
