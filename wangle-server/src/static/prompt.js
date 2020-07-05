function $$$prompt_from(s, ls){
	while(true){
		const x = $$$prompt(s + "\nOptions:\n" + ls.join("\n"));
		if(ls.includes(x))
			return x;
	}
}
