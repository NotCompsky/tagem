function $$$prompt_from(s, ls){
	while(true){
		const x = $$$prompt(s + "\nOptions:\n" + ls.join("\n"));
		if(ls.includes(x))
			return x;
	}
}

function $$$get_int_with_prompt(s){
	while(true){
		const s = $$$prompt(s);
		if(s===null)
			// User cancelled the prompt
			return null;
		const n = $$$parseInt(s);
		if(!isNaN(n))
			return n;
	}
}

function $$$get_int(){
	return $$$get_int_with_prompt("Integer");
}
