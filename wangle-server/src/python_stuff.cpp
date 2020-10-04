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

#include "python_stuff.hpp"
#include "os.hpp"
#include "errors.hpp"
#include <compsky/macros/likely.hpp>
#include <stdexcept>
#include <mutex>
#include <Python.h> // WARNING: Must be before any Qt includes, because slots is a macro name.


#define PY_ASSERT_NOT_NULL(obj, msg) \
	if (unlikely(obj == nullptr)) \
		throw std::runtime_error("Python error: " msg);


extern const char* YTDL_FORMAT;


namespace python {

// youtube_dl.YoutubeDL(options).download(args)
// MODULE     INSTANTIATION      FUNCTION

static PyObject* ytdl_obj;

namespace tagem_module {
	static PyObject* to_stdout;
	
	static PyObject* modul;
	
	static PyObject* whyyyyyyyyyyyyyyyy;
	
	static
	PyObject* to_stdout_fn(PyObject* const self,  PyObject* const args,  PyObject* const keyword_args){
		// Overrides to_stdout - saves file path instead of writing to stdout
		
		// NOTE: self is the module, not the class instantiation, for some reason.
		// So as a workaround, static C-string is set, and mutex lock is needed in the calling function
		// In reality, the download() function could take many minutes, so a mutex lock is completely infeasible. Hence we must *hope* that there won't be a data race.
		
		PyObject* const file_path = PyTuple_GetItem(args, 0);
		if (unlikely(file_path == nullptr)){
			Py_RETURN_NONE;
		}
		const char* const str = PyUnicode_AsUTF8(file_path);
		if (not os::is_local_file_or_dir(str))
			Py_RETURN_NONE;
		log("filepath == ", str);
		whyyyyyyyyyyyyyyyy = file_path;
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
	ytdl_obj = PyObject_GetAttrString(ytdl_module, "YoutubeDL");
	PY_ASSERT_NOT_NULL(ytdl_obj, "Cannot find youtube_dl.YoutubeDL");
	Py_INCREF(ytdl_module);
	Py_INCREF(ytdl_obj);
}

bool ytdl(char* const out_fmt_as_input__resulting_fp_as_output,  const char* const url){
	PyObject* const urls_ls = Py_BuildValue("[s]", url); // Or [item]?
	
	PyObject* const opts = PyDict_New();
	PyDict_SetItem(
		opts,
		PyUnicode_FromString("quiet"),
		Py_True
	);
	PyDict_SetItem(
		opts,
		PyUnicode_FromString("forcefilename"), // Forces it to 'print' the filename in to_stdout
		Py_True
	);
	PyDict_SetItem(
		opts,
		PyUnicode_FromString("outtmpl"),
		PyUnicode_FromString(out_fmt_as_input__resulting_fp_as_output)
	);
	PyDict_SetItem(
		opts,
		PyUnicode_FromString("format"),
		PyUnicode_FromString(YTDL_FORMAT)
	);
	PyObject* const instantiation_args = PyTuple_New(1);
	PY_ASSERT_NOT_NULL(instantiation_args, "Cannot instantiate instantiation args");
	PyTuple_SetItem(instantiation_args, 0, opts);
	PyObject* const ytdl_instantiation = PyObject_CallObject(ytdl_obj, instantiation_args); // Error here
	PY_ASSERT_NOT_NULL(ytdl_instantiation, "Cannot instantiate YoutubeDL object");
	PyObject* const ytdl_fn = PyObject_GetAttrString(ytdl_instantiation, "download");
	
	// Override to_stdout, so that the file path is written to a buffer instead of stdout
	PyObject_SetAttrString(ytdl_instantiation, "to_stdout", tagem_module::to_stdout);
	/*PyObject_SetAttrString(tagem_module::modul, "ytdl_instantiation", ytdl_instantiation);
	PyRun_String("tagem.ytdl_instantiation.to_stdout = tagem.to_stdout", Py_single_input, nullptr, nullptr);
	// The C API call acts differently - the first argument is NOT a pointer to the 'self' object, but remains a pointer to the module object*/
	
	PyObject* const fn_args = PyTuple_New(1);
	PyTuple_SetItem(fn_args, 0, urls_ls);
	const PyObject* const result = PyObject_CallObject(ytdl_fn, fn_args);
	
	PyObject* const file_path = tagem_module::whyyyyyyyyyyyyyyyy;
	Py_ssize_t sz;
	const char* const _file_path = PyUnicode_AsUTF8AndSize(file_path, &sz);
	memcpy(out_fmt_as_input__resulting_fp_as_output, _file_path, sz);
	out_fmt_as_input__resulting_fp_as_output[sz] = 0;
	
	Py_DECREF(file_path);
	Py_DECREF(result);
	Py_DECREF(fn_args);
	Py_DECREF(ytdl_fn);
	Py_DECREF(ytdl_instantiation);
	Py_DECREF(instantiation_args);
	Py_DECREF(opts);
	Py_DECREF(urls_ls);
	
	return false;
}

} // namespace python
