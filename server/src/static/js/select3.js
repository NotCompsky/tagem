// CSV

function $$$select3__get_csv(inp){
	return $$$select3__get_ls(inp).join(',');
}

function $$$select3__get_ls(inp){
	return $$$select3__get_spans_from_inp(inp).map(x => x.dataset.x);
}

function $$$select3__get_dict(inp){
	return $$$select3__get_spans_from_inp(inp).map(function(x){return {id:x.dataset.x, text:x.innerText};});
}

function $$$select3__d2csv(d){
	return d.map(x => x.id).join(",");
}

function $$$select3__count(inp){
	return inp.parentNode.childNodes.length-2;
}


// <input> events

function $$$select3__onkeyup(inp, e, fn){
	switch(e.keyCode){
		case 13:
			// ENTER
			if (fn !== undefined)
				fn($$$select3__get_csv(inp));
			break;
		case 38:
			// UP
			// TODO: Select opt
			break;
		case 40:
			// DOWN
			// TODO: Select opt
			break;
		case 27:
			// ESCAPE
			const node = inp.parentNode;
			$$$select3__get_opts_div_from_root_div(node).style.display = "none";
			$$$select3__get_input_node_from_root_div(node).value = "";
	}
}


// DOM

function $$$select3__get_opts_div_from_root_div(r){
	return Array.from(r.childNodes).filter(x => x.tagName==="DIV")[0];
}

function $$$select3__get_opts_from_root_div(node){
	return $$$select3__get_opts_div_from_root_div(node).childNodes;
}

function $$$select3__get_input_node_from_root_div(node){
	return Array.from(node.childNodes).filter(x => x.tagName==="INPUT")[0];
}

function $$$select3__oninput(node){
	$$$select3__qry(node);
}

function $$$select3__add_tag(root,id,name){
	root.insertAdjacentHTML("beforeend", `<span data-x="${id}">${name}<a onclick="this.parentNode.remove()">x</a></span>`);
}

function $$$select3__get_spans_from_inp(inp){
	return Array.from(inp.parentNode.childNodes).filter(x => x.tagName === "SPAN");
}

function $$$select3__selected_node(node){
	xxx = node;
	if (node.dataset.x!==""){
		const root = node.parentNode.parentNode;
		const inp = $$$select3__get_input_node_from_root_div(root);
		if (!((inp.dataset.one==="1") && $$$select3__count(inp))){
			if (!$$$select3__get_dict(inp).map(x=>x.id).includes(node.dataset.x)){
				$$$select3__add_tag(root, node.dataset.x, node.innerText);
				inp.value = "";
				if (inp.dataset.fn !== undefined){
					window[inp.dataset.fn]($$$select3__get_ls(inp));
				}
			}
		}
	}
	node.parentNode.style.display = "none";
}

function $$$select3__wipe_values(inp){
	$$$select3__get_spans_from_inp(inp).map(x => x.remove());
}

function $$$select3__write_opts(_root,ls){
	if (ls.length === 0)
		ls = [["","<NO RESULTS>"]];
	$$$select3__get_opts_div_from_root_div(_root).style.display = "";
	const opts = $$$select3__get_opts_from_root_div(_root);
	for (let i = 0;  i < !!!MACRO!!!N_SELECT3_OPTS_STR;  ++i){
		const style = opts[i].style;
		if (i < ls.length){
			style.display = "";
			opts[i].dataset.x = ls[i][0];
			opts[i].innerText = ls[i][1];
		} else style.display = "none";
	}
}

function $$$select3__qry(node){
	const var_name = node.dataset.v;
	let ignore_ids = "0";
	const window_var = node.w;
	const x = node.value;
	
	if (x.length < 3)
		return;
	
	switch(var_name){
		case 'f':
			ignore_ids = $$$get_file_ids();
			break;
	}
	
	if (window_var === undefined){
		$$$ajax_POST_data_w_JSON_response(var_name+"/select3/"+($$$use_regex?'1':'0')+'/'+ignore_ids, x, function(ls){ // NOTE: Could do way more than regex
			$$$select3__write_opts(node.parentNode, ls);
		});
	} else {
		const ls = window[window_var];
		$$$select3__write_opts(node.parentNode, ls);
	}
}
