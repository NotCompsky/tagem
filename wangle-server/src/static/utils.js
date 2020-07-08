function $$$sleep(ms){
	return new Promise(resolve => setTimeout(resolve, ms));
}

const $$$YOUTUBE_DEVICE_ID = "1";

function $$$focus(id){
	$$$document_getElementById(id).focus();
}

function $$$init_selects(var_name){
	let s = "";
	const col = $$$tbl2namecol[var_name];
	$($$$tbl2selector[var_name]).select2({
		placeholder: $$$nickname2fullname[var_name] + ($$$use_regex)?" regexp":"",
		ajax:{
			transport: function (params, success, failure){
				let arr = Object.entries(window[var_name]); // WARNING: I don't see why there aren't scope issues
				if(params.data.q !== undefined){
					const pattern = ($$$use_regex) ? new RegExp(params.data.q) : params.data.q;
					arr = arr.filter(x => x[1][0].search(pattern)>=0);
				}
				if(arr.length > 50){
					arr = arr.slice(0, 50);
					arr.unshift(['0', ['Truncated to 50 results']]);
				}
				success(arr);
			},
			processResults: function(data){
				return{
					results: data.map(([id,tpl]) => ({id:id, text:((col===null)?tpl:tpl[col])}))
				};
			}
		}
	}); // Initialise
}

function $$$init_selects__ajax(var_name){
	let s = "";
	const col = $$$tbl2namecol[var_name];
	$($$$tbl2selector[var_name]).select2({
		placeholder: $$$nickname2fullname[var_name] + ($$$use_regex)?" regexp":"",
		ajax:{
			url: "/a/select2/regex/" + var_name,
			dataType:"json",
			data:function(params){
				return ($$$use_regex) ? {'q': params.term} : {'q': RegExp.escape(params.term)}
			},
			processResults:function(data){
				return{
					results:Object.entries(data).map(([id,name]) => ({id:id, text:name}))
				};
			}
		}
	});
}

function $$$refetch_json(var_name, url, fn){
	$$$ajax_GET_w_JSON_response(url + '?' + (new Date().getTime()), function(data){
		// Cache buster url parameter
		console.log("Cache busting", var_name);
		window[var_name] = data;
		if(fn !== undefined)
			fn();
		if((var_name==='d')||(var_name==='t'))
			$$$init_selects__ajax(var_name);
		else
			$$$init_selects(var_name);
	});
}

function $$$login(){
	const uname = prompt("Username");
	$$$set_cookie("username", uname, 3600*24); // Super 100% secure login with inbuilt blockchain neural networks
	$$$document_getElementById('username').innerText = uname;
	$$$refetch_all_jsons();
}
function $$$logout(){
	$$$unset_cookie("username");
	$$$document_getElementById('username').innerText = "GUEST";
	$$$refetch_all_jsons();
}
