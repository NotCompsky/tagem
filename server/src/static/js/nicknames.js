// 
// Copyright 2020 Adam Gray
// This file is part of the tagem program.
// The tagem program is free software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published by the
// Free Software Foundation version 3 of the License.
// The tagem program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// This copyright notice should be included in any copy or substantial copy of the tagem source code.
// The absense of this copyright notices on some other files in this project does not indicate that those files do not also fall under this license, unless they have a different license written at the top of the file.
// 
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

function $$$for_node_in_document_getElementsByClassName_1args(s,fn,x){
	for(let e of $$$document_getElementsByClassName(s))
		fn(e,x);
}
function $$$for_node_in_document_getElementsByClassName_1args__reverse(s,fn,x){
	const ls = $$$document_getElementsByClassName(s);
	for(let i = ls.length;  i !== 0;  ){
		fn(ls[--i],x);
	}
}
const $$$for_node_in_document_getElementsByClassName = $$$for_node_in_document_getElementsByClassName_1args;
// Because why not? JS just ignores all the extra arguments.

function $$$get_tbl_body(id){
	return $$$document_getElementById(id).getElementsByClassName('tbody')[0];
}

function $$$focus(id){
	$$$document_getElementById(id).focus();
}

function $$$remove_node(e){
	e.remove();
}

function $$$for_each_of(ls,fn){
	for(let x of ls)
		fn(x);
}

function $$$rm_class(s){
	$$$for_node_in_document_getElementsByClassName_1args__reverse(s,$$$remove_node);
	// NOTE: Must reverse order of iteration, else removing first node puts the 2nd node in the 1st node's place and skips its deletion
}

function $$$get_dirselect_value(){
	return $$$select3__get_csv($$$dirselect);
}

function $$$unset_dirselect_value(){
	$$$select3__wipe_values($$$dirselect);
}

function $$$rm_class_from_node(x,c){
	x.classList.remove(c);
}

function $$$rm_class_from_applicable_nodes(name){
	$$$for_node_in_document_getElementsByClassName_1args__reverse(name,$$$rm_class_from_node,name);
}

function $$$get_last_child_node(n){
	// Weird stuff with inner text sometimes being considered the first child node
	const ch = n.childNodes;
	return ch[ch.length - 1];
}
