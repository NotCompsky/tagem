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

// WARNING: Boxes are messed up for images displayed within <iframe>. I do not think there is an easy fix to this, and no fixed is currently planned. Images should only be displayed in <img> tag, and if they are displayed in an <iframe> that is because the wrong mimetype has been set.


const $$$min_box_size_px = 10.0;
const $$$temporary_box_container = $$$document_getElementById("tmp-box-container");
const $$$temporary_box = $$$document_getElementById("tmp-box");


// "private" variables only to be used in this page's functions
var $$$box_to_be_created__x1; // The anchor - not necessarily the left of the box
var $$$box_to_be_created__y1;
var $$$box_to_be_created__x;
var $$$box_to_be_created__y;
var $$$box_to_be_created__w;
var $$$box_to_be_created__h;
function $$$draw_box__init_dimensions(box){
	[$$$box_to_be_created__x, $$$box_to_be_created__y, $$$box_to_be_created__w, $$$box_to_be_created__h] = box;
}


function $$$add_box_given_tags(tags){
	$$$draw_box(["0", 0, $$$box_to_be_created__x, $$$box_to_be_created__y, $$$box_to_be_created__w, $$$box_to_be_created__h, tags]);
}

function $$$draw_box(box){
	const [id,frame,x,y,w,h,tags] = box;
	const e = $$$document.createElement("div");
	e.dataset.id = id;
	
	// e.style.transform = "translateX("+100*x+"%) translateY("+100*y+"%)";
	// The insane thing is that CSS translate rules use the node's own dimensions for translates here, for no apparent reason. Even animation translates use the parent node's dimensions!
	// So to get around this, we create a container <div>, set its width to 100% of the parent, and translate that instead.
	const why = $$$document.createElement("div");
	why.style.transform = "translateX("+100*x+"%) translateY("+100*y+"%)";
	why.classList = "box-container unselectable";
	
	// transforms due to https://www.w3.org/TR/css-transforms-1/#containing-block-for-all-descendants (see CSS)
	e.style.width  = 100*w+"%";
	e.style.minHeight = 100*h+"%";
	e.style.maxHeight = 100*h+"%";
	// Both Firefox and Chrome treat height attribute on its own as min-height, and max-height+height as min-height+min-height.
	e.classList = "box";
	e.dataset.frame_n = frame;
	e.dataset.w = w;
	e.dataset.h = h;
	e.dataset.x = x;
	e.dataset.y = y;
	e.innerHTML =
		'<div class="resizer topL"></div><div class="resizer topR"></div>' +
		tags.map(([id,name]) => $$$display_tag(id, name, null, "$$$unlink_this_tag_from_this_box", 'b')).join("") +
		'<div class="bottom-anchor"><div class="resizer botL"></div><div class="resizer botR"></div></div>'
	;
	
	$$$make_resizeable(e);
	why.prepend(e);
	$$$document_getElementById("view").appendChild(why);
}

function $$$make_resizeable(box){
	// NOTE: Although we are dragging the item by an amount in pixels, we must only set the width to a % of it's parent (image) width.
	for (const resizer of box.getElementsByClassName("resizer")){
		resizer.addEventListener('mousedown', function(e){
			const corner_orig_x_px = e.pageX;
			const corner_orig_y_px = e.pageY;
			
			const box_orig_w_px = parseFloat(box.scrollWidth);
			const box_orig_h_px = parseFloat(box.scrollHeight);
			
			let x = parseFloat(box.dataset.x);
			let y = parseFloat(box.dataset.y);
			let w = parseFloat(box.dataset.w);
			let h = parseFloat(box.dataset.h);
			
			const xscale = parseFloat(box.dataset.w) / box_orig_w_px;
			const yscale = parseFloat(box.dataset.h) / box_orig_h_px;
			const min_x = $$$min_box_size_px * xscale;
			const min_y = $$$min_box_size_px * yscale;
			
			function user_dragging_new_box_event(e){
				const delta_y_norm = yscale * (e.pageY - corner_orig_y_px);
				if (resizer.classList.contains('topL') || resizer.classList.contains('topR')){
					y = parseFloat(box.dataset.y) + delta_y_norm;
					h = parseFloat(box.dataset.h) - delta_y_norm;
				} else {
					h = parseFloat(box.dataset.h) + delta_y_norm;
				}
				if(h >= min_y)
					box.style.minHeight = box.style.maxHeight = box.style.height = 100*h+"%";
				
				const delta_x_norm = xscale * (e.pageX - corner_orig_x_px);
				if (resizer.classList.contains('topL') || resizer.classList.contains('botL')){
					x = parseFloat(box.dataset.x) + delta_x_norm;
					w = parseFloat(box.dataset.w) - delta_x_norm;
				} else {
					w = parseFloat(box.dataset.w) + delta_x_norm;
				}
				if(w >= min_x)
					box.style.width = 100*w+"%";
				
				if ((!resizer.classList.contains('botR'))&&(h >= min_y)&&(w >= min_x))
					box.parentNode.style.transform = "translateX("+100*x+"%) translateY("+100*y+"%)";
			}
			
			window.addEventListener('mousemove', user_dragging_new_box_event);
			window.addEventListener('mouseup', function(){
				if(w >= min_x)
					box.dataset.w = w;
				if(h >= min_y)
					box.dataset.h = h;
				if((x >= 0)&&(x <= 1))
					box.dataset.x = x;
				if((y >= 0)&&(y <= 1))
					box.dataset.y = y;
				window.removeEventListener('mousemove', user_dragging_new_box_event);
			});
		});
	}
}

function $$$get_box_tags(node){
	return Array.from(node.getElementsByClassName("tag")).map(e => e.dataset.id).join(",");
}

function $$$save_boxes(){
	$$$ajax_POST_data_w_JSON_response(
		"/box/add/"+$$$file_id,
		Array.from($$$document_getElementsByClassName("box")).map(e => e.dataset.id+";"+e.dataset.frame_n+";"+e.dataset.x+";"+e.dataset.y+";"+e.dataset.w+";"+e.dataset.h+";"+$$$get_box_tags(e)).join("\n")+"\n",
		function(box_ids){
			// Update the IDs of the newly recorded boxes
			let i = 0;
			for(let e of $$$document_getElementsByClassName("box")){
				if(e.dataset.id !== "0")
					continue;
				e.dataset.id = box_ids[i++];
			}
			$$$alert_success();
		}
	);
}

function $$$create_box_from_dragging(e){
	let x = $$$box_to_be_created__x1;
	let y = $$$box_to_be_created__y1;
	let X = e.pageX - $$$document_getElementById("view").getBoundingClientRect().x;
	let Y = e.pageY - $$$document_getElementById("view").getBoundingClientRect().y;
	
	// Guarantee x <= X etc
	if (x > X)
		[x, X] = [X, x];
	if (y > Y)
		[y, Y] = [Y, y];
	
	if ((X > x + $$$min_box_size_px) && (Y > y + $$$min_box_size_px)){
		const scaleX = $$$document_getElementById("view").scrollWidth;
		const scaleY = $$$document_getElementById("view").scrollHeight;
		$$$draw_box__init_dimensions([x/scaleX,y/scaleY,(X-x)/scaleX,(Y-y)/scaleY]);
		window.removeEventListener('mousemove', $$$user_dragging_new_box_event);
		window.removeEventListener('mouseup', $$$user_mouseup_event);
		$$$prompt_for_tags($$$add_box_given_tags);
	}
	
	console.log(x,y,X,Y);
}


function $$$user_dragging_new_box_event(e){
	let x = $$$box_to_be_created__x1;
	let y = $$$box_to_be_created__y1;
	let X = e.pageX - $$$document_getElementById("view").getBoundingClientRect().x;
	let Y = e.pageY - $$$document_getElementById("view").getBoundingClientRect().y;
	
	// Guarantee x <= X etc
	if (x > X)
		[x, X] = [X, x];
	if (y > Y)
		[y, Y] = [Y, y];
	
	if ((X > x + $$$min_box_size_px) && (Y > y + $$$min_box_size_px)){
		$$$temporary_box_container.classList.remove("hidden");
		$$$temporary_box_container.style.transform = "translateX("+x+"px) translateY("+y+"px)";
		$$$temporary_box.style.width  = (X-x)+"px";
		$$$temporary_box.style.minHeight = $$$temporary_box.style.maxHeight = $$$temporary_box.style.height = (Y-y)+"px";
	}
	
	console.log(x,y,X,Y);
}

function $$$user_mouseup_event(e){
	$$$temporary_box_container.classList.add("hidden");
	$$$create_box_from_dragging(e);
	window.removeEventListener('mousemove', $$$user_dragging_new_box_event);
	window.removeEventListener('mouseup', $$$user_mouseup_event);
}

function $$$mousedown_on_image(e){
	$$$box_to_be_created__x1 = e.pageX - $$$document_getElementById("view").getBoundingClientRect().x;
	$$$box_to_be_created__y1 = e.pageY - $$$document_getElementById("view").getBoundingClientRect().y;
	window.addEventListener('mousemove', $$$user_dragging_new_box_event);
	window.addEventListener('mouseup', $$$user_mouseup_event);
}
