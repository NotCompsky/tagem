function $$$view_tasks(){
	$$$hide_all_except(['tasks-container']);
	$.get({
		url:"!!!MACRO!!!SERVER_ROOT_URL/a/exec.json",
		success:function(data){
			let _s = "";
			for(const [id,name,descr,content] of data){
				_s += `\t
					<div class='task'>
						<h3 class='task-name'>` + name + `</h3>
						<p class='task-descr'>` + descr + `</p>
						<a onclick='$$$toggle("task-content` + id + `")'>Content</a>
						<div id='task-content` + id + `' class='hidden task-content'></div>
						<a onclick='$$$exec_task(` + id + `)'>Exec</a>
					</div>`;
				// NOTE: The tab is there to ensure that whitespace is not preserved
			}
			$$$document_getElementById('tasks').innerHTML = _s;
			for(const [id,name,descr,content] of data){
				$$$document_getElementById('task-content'+id).innerText = content;
			}
		},
		error:$$$alert_requires_login
	});
}

function $$$exec_task(task_id){
	$$$ajax_POST_w_text_response_generic_success("/exec/"+task_id);
}
