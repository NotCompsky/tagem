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
	
	$$$use_regex = $$$get_and_set_default_user_setting_tofrom_cookie__bool("use_regex",false);
	$$$get_and_set_default_user_setting_tofrom_cookie__tbl_f_hide_col('thumbnail',false);
	$$$get_and_set_default_user_setting_tofrom_cookie__tbl_f_hide_col('fname',false);
	$$$get_and_set_default_user_setting_tofrom_cookie__tbl_f_hide_col('ftitle',false);
	$$$get_and_set_default_user_setting_tofrom_cookie__tbl_f_hide_col('tags',false);
	$$$get_and_set_default_user_setting_tofrom_cookie__tbl_f_hide_col('dateadded',true);
	$$$get_and_set_default_user_setting_tofrom_cookie__tbl_f_hide_col('date_origin',false);
	$$$get_and_set_default_user_setting_tofrom_cookie__tbl_f_hide_col('file_size',false);
	$$$get_and_set_default_user_setting_tofrom_cookie__tbl_f_hide_col('duration',false);
	$$$get_and_set_default_user_setting_tofrom_cookie__tbl_f_hide_col('w',true);
	$$$get_and_set_default_user_setting_tofrom_cookie__tbl_f_hide_col('h',true);
	$$$get_and_set_default_user_setting_tofrom_cookie__tbl_f_hide_col('views',true);
	$$$get_and_set_default_user_setting_tofrom_cookie__tbl_f_hide_col('likes',true);
	$$$get_and_set_default_user_setting_tofrom_cookie__tbl_f_hide_col('dislikes',true);
	$$$get_and_set_default_user_setting_tofrom_cookie__tbl_f_hide_col('fps',true);
	
	for(let [key,val] of [['sleep_on_inanimate_media',2],['sleep_after_media_err',2]]){
		const val2 = $$$get_cookie(key);
		$$$set_user_setting_as(key,((val2===undefined)?val:val2));
	}
	
	let w = $$$get_cookie("w");
	if(w===undefined)
		w=256;
	$$$set_thumbnail_width(w);
	
	$$$make_tbl_selectable("f");
	$$$make_tbl_selectable("t");
	$$$make_tbl_selectable("eras-info");
	$$$init_qry();
	
	for(node of $$$document_getElementsByTagName('input')){
		node.addEventListener('focusin',  $$$del_key_intercepts);
		node.addEventListener('focusout', $$$add_key_intercepts);
	}
	$$$add_key_intercepts();
	
	$$$document_getElementById('help-text').innerText = `
Credits
	The layout of this site's HTML is based on 'Responsive Social Platform' by Aysenur Turk: https://codepen.io/TurkAysenur/details/RwWKYMO
	Almost all SVG icons are sourced or derived from tabler-icons by Paweł Kuna: https://github.com/tabler/tabler-icons
		The exceptions are the two triangle icons by myself, and the search icon by Aysenur Turk
	JavaScript libraries used:
		JQuery
		datetimepicker: https://github.com/xdan/datetimepicker
			For the date-time picker dialog
		PHP-Date-Formatter by Kartik Visweswaran: https://github.com/kartik-v/php-date-formatter
			A dependency of datetimepicker
		Select2: https://github.com/select2/select2
			For fancier dropdown menus
		MouseTrap: https://github.com/ccampbell/mousetrap
			For intercepting key events

Keyboard Shortcuts
	b        Toggle the 'add backup file' dialog
	e        Add era vertex (start or end)
	m        Merge selected files
	p        View selected files as a playlist
	q        Make a qry
	v        Toggle the 'add file2 variable to file' dialog

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
