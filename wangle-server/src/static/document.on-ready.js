function $$$on_document_ready(){
	$$$hide_all_except([]);
	
	const uname = $$$get_cookie("username");
	$$$document_getElementById('username').innerText = (uname === undefined)?"GUEST":uname;
	
	$$$refetch_all_jsons();
	$$$init_selects__ajax('d');
	$$$init_selects__ajax('t');
	
	// From google's documentation: This code loads the IFrame Player API code asynchronously.
	let node = document.createElement('script');
	node.src = "https://www.youtube.com/iframe_api";
	let firstScriptTag = $$$document_getElementsByTagName('script')[0];
	firstScriptTag.parentNode.insertBefore(node, firstScriptTag);
	
	let css = $$$get_cookie("css");
	if(css===undefined)
		css = $$$stylesheet_opts[0];
	$$$switch_stylesheet(css);
	
	if($$$use_regex === undefined){
		$$$use_regex = $$$get_cookie("use_regex");
		if($$$use_regex !== undefined){
			$$$use_regex = ($$$use_regex === "1");
		}else{
			$$$use_regex = confirm("Use Regular Expressions?\nSay no if you don't know what they are");
			$$$set_cookie("use_regex", ($$$use_regex)?"1":"0", 3600);
		}
	}
	
	let w = $$$get_cookie("w");
	if(w===undefined)
		w=256;
	$$$set_thumbnail_width(w);
	
	$$$fancify_tbl("f");
	$$$init_qry();
	
	for(node of $$$document_getElementsByTagName('input')){
		node.addEventListener('focusin',  $$$del_key_intercepts);
		node.addEventListener('focusout', $$$add_key_intercepts);
	}
	$$$add_key_intercepts();
	
	$$$document_getElementById('help-text').innerText = `
Date Query
	USAGE
		[DATE]
		[DATE] - [DATE]
			Second date should be the largest
	DATE
		yyyy
		yyyy/mm
		yyyy/mm/dd
	EXAMPLES
		2020
		2020/03
		2018-2020
		2016/05-2020/06

Suppose you have a screenshot of a Twitter post. You can get the link to the original post - for instance, by using 'find-tweet' from https://github.com/NotCompsky/utils. You can then go to that file, click "Update src", and change the source to the Twitter post's URL. This will keep the screenshot as a 'backup' of the post.
	`;
}
