const $$$Mousetrap_bind   = Mousetrap.bind;
const $$$Mousetrap_unbind = Mousetrap.unbind;
const $$$document = document;
const $$$document_getElementById = $$$document.getElementById.bind($$$document);
const $$$document_getElementsByClassName = $$$document.getElementsByClassName.bind($$$document);
const $$$document_getElementsByTagName = $$$document.getElementsByTagName.bind($$$document);
const $$$document_querySelector = $$$document.querySelector.bind($$$document);
const $$$confirm = confirm;
const $$$prompt = prompt;
const $$$window = window;
const $$$parseInt = parseInt;

function $$$for_node_in_document_getElementsByClassName(s, fn){
	for(let e of $$$document_getElementsByClassName(s))
		fn(e);
}

function $$$get_tbl_body(id){
	return $$$document_getElementById(id).getElementsByClassName('tbody')[0];
}

function $$$focus(id){
	$$$document_getElementById(id).focus();
}
