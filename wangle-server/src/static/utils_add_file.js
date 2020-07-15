function $$$add_files_dialog(){
	$$$hide_all_except(['tagselect-files-container','add-f-backup-toggle-container','add-f-dialog']);
	$$$hide('add-f-backup-url');
	// TODO: Toggle switch to allow selecting dir ID for backup
	$('#dirselect').val(null).trigger('change');
}
