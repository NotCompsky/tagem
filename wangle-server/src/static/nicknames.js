const $$$Mousetrap_bind   = Mousetrap.bind;
const $$$Mousetrap_unbind = Mousetrap.unbind;
const $$$document_getElementById = document.getElementById.bind(document);
const $$$confirm = confirm;
const $$$prompt = prompt;
const $$$window_location = window.location;
const $$$window_location_hash = $$$window_location.hash;
function $$$set_window_location_hash(s){
	$$$window_location.hash = s;
}
function $$$unset_window_location_hash(){
	$$$set_window_location_hash("");
}
