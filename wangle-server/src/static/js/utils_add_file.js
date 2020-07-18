function $$$add_files_dialog(){
	$$$document_getElementById('add-f-backup-toggle').checked = false;
	$$$hide_all_except(['tagselect-files-container','add-f-backup-toggle-container','add-f-dialog']);
	$$$hide('add-f-backup-url');
	// TODO: Toggle switch to allow selecting dir ID for backup
	$('#dirselect').val(null).trigger('change');
}
