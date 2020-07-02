function hide_rows_with_zero_id(){
	for(row of document.getElementById('f').getElementsByClassName('tr')){
		if(row.dataset.id === "0")
			row.classList.add("hidden");
	}
}
