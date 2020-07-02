function $$$alert(s){
	alert(s);
}

function $$$alert_success(){
	$$$alert("Success");
}

function $$$alert_requires_login(){
	$$$alert("Not authorised.\nPlease log in first.");
}

function $$$error_alert(title, text){
	$$$alert(title + "\n" + text);
}

function $$$err_alert(r,title,text){
	$$$error_alert(r.statusText, text + "\nfor url: " + this.url);
}

function $$$debug__err_alert(s){
	console.log("WARNING", s);
}
