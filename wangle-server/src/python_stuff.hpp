/*
Copyright 2020 Adam Gray
This file is part of the tagem program.
The tagem program is free software: you can redistribute it and/or
modify it under the terms of the GNU General Public License as published by the
Free Software Foundation version 3 of the License.
The tagem program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
This copyright notice should be included in any copy or substantial copy of the tagem source code.
The absense of this copyright notices on some other files in this project does not indicate that those files do not also fall under this license, unless they have a different license written at the top of the file.
*/

#pragma once

#include "python_stuff.hpp"
#include "os.hpp"
#include "errors.hpp"
#include "python.hpp"
#include <compsky/macros/likely.hpp>
#include <stdexcept>


extern const char* YTDL_FORMAT;


namespace python {

// youtube_dl.YoutubeDL(options).download(args)
// MODULE     INSTANTIATION      FUNCTION

static PyObj ytdl_obj;
static PyObj view_remote_dir__parse_url_fn;

namespace tagem_module {
	static PyObject* to_stdout;
	
	static PyObject* modul;
	
	static PyObj file_path;
	static PyObj json_metadata;
	
	static
	PyObject* to_stdout_fn(PyObject* const self,  PyObject* const args,  PyObject* const keyword_args){
		// Overrides to_stdout - saves file path instead of writing to stdout
		
		// NOTE: self is the module, not the class instantiation, for some reason.
		// So as a workaround, static C-string is set, and mutex lock is needed in the calling function
		// In reality, the download() function could take many minutes, so a mutex lock is completely infeasible. Hence we must *hope* that there won't be a data race.
		
		PyObject* const _data = PyTuple_GetItem(args, 0);
		if (unlikely(_data == nullptr)){
			Py_RETURN_NONE;
		}
		const PyObj data(_data);
		const char* const str = data.as_str();
		if (str[0] == '{')
			json_metadata.obj = data.obj;
		else if (os::is_local_file_or_dir(str))
			file_path.obj = data.obj;
		Py_RETURN_NONE;
	}
	
	static
	PyMethodDef _methods[] = {
		{"to_stdout", (PyCFunction)to_stdout_fn, METH_VARARGS | METH_KEYWORDS, "Description."},
		{nullptr, nullptr, 0, nullptr}
	};
	
	struct _state {
		PyObject* error;
	};
	
	static
	struct PyModuleDef _module = {
		PyModuleDef_HEAD_INIT,
		"tagem",
		nullptr,
		sizeof(struct _state),
		_methods,
		nullptr,
		0,
		0,
		nullptr
	};
}

void init_ytdl(){
	Py_SetProgramName(L"tagem");
	Py_Initialize();
	tagem_module::modul = PyModule_Create(&tagem_module::_module);
	tagem_module::to_stdout = PyObject_GetAttrString(tagem_module::modul, "to_stdout");
	
	PyObject* ytdl_module = PyImport_ImportModule("youtube_dl");
	PY_ASSERT_NOT_NULL(ytdl_module, "Cannot import youtube_dl");
	ytdl_obj.obj = PyObject_GetAttrString(ytdl_module, "YoutubeDL");
}

bool import_view_remote_dir(const char* const usr,  const char* const pwd,  const char* const host,  const char* const path){
	PyObject* _module = PyImport_ImportModule("youtube_dl");
	if (unlikely(_module == nullptr))
		return true;
	
	PyObj init_mysql_fn(_module, "init_mysql");
	
	PyStr _usr(usr);
	PyStr _pwd(pwd);
	PyStr _host(host);
	PyStr _path(path);
	
	init_mysql_fn.call(_usr.obj, _pwd.obj, _host.obj, _path.obj);
	
	return false;
}

bool import_view_remote_dir(){
	return false;
}

bool view_remote_dir(const char* const url,  const char* const scripts_dir){
	return false;
}

} // namespace python
