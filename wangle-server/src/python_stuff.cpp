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
#include <memory> // for std::unique_ptr


#define PY_ASSERT_NOT_NULL(obj, msg) \
	if (unlikely(obj == nullptr)) \
		throw std::runtime_error("Python error: " msg);


extern const char* YTDL_FORMAT;


namespace python {

// youtube_dl.YoutubeDL(options).download(args)
// MODULE     INSTANTIATION      FUNCTION

typedef std::shared_ptr<PyObject> PyObjectPtr;

static
PyObjectPtr make_py_obj_ptr(PyObject* const obj){
	return PyObjectPtr(obj,  [](PyObject* o){ Py_DECREF(o); });
}

static PyObject* ytdl_obj;

namespace tagem_module {
	static PyObject* to_stdout;
	
	static PyObject* modul;
	
	static PyObjectPtr file_path;
	
	static
	PyObject* to_stdout_fn(PyObject* const self,  PyObject* const args,  PyObject* const keyword_args){
		// Overrides to_stdout - saves file path instead of writing to stdout
		
		// NOTE: self is the module, not the class instantiation, for some reason.
		// So as a workaround, static C-string is set, and mutex lock is needed in the calling function
		// In reality, the download() function could take many minutes, so a mutex lock is completely infeasible. Hence we must *hope* that there won't be a data race.
		
		file_path = make_py_obj_ptr(PyTuple_GetItem(args, 0));
		if (unlikely(file_path == nullptr)){
			Py_RETURN_NONE;
		}
		const char* const str = PyUnicode_AsUTF8(file_path.get());
		if (not os::is_local_file_or_dir(str))
			Py_RETURN_NONE;
		log("filepath == ", str);
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
}

bool ytdl(char* const out_fmt_as_input__resulting_fp_as_output,  const char* const url){
	PyObjectPtr opts = make_py_obj_ptr(PyDict_New());
	PyDict_SetItem(
		opts.get(),
		PyUnicode_FromString("quiet"),
		Py_True
	);
	PyDict_SetItem(
		opts.get(),
		PyUnicode_FromString("forcefilename"), // Forces it to 'print' the filename in to_stdout
		Py_True
	);
	PyDict_SetItem(
		opts.get(),
		PyUnicode_FromString("outtmpl"),
		PyUnicode_FromString(out_fmt_as_input__resulting_fp_as_output)
	);
	PyDict_SetItem(
		opts.get(),
		PyUnicode_FromString("format"),
		PyUnicode_FromString(YTDL_FORMAT)
	);
	PyObjectPtr instantiation_args = make_py_obj_ptr(PyTuple_New(1));
	PY_ASSERT_NOT_NULL(instantiation_args.get(), "Cannot instantiate instantiation args");
	PyTuple_SetItem(instantiation_args.get(), 0, opts.get());
	PyObjectPtr ytdl_instantiation = make_py_obj_ptr(PyObject_CallObject(ytdl_obj, instantiation_args.get())); // Error here
	PY_ASSERT_NOT_NULL(ytdl_instantiation.get(), "Cannot instantiate YoutubeDL object");
	
	// Override to_stdout, so that the file path is written to a buffer instead of stdout
	PyObject_SetAttrString(ytdl_instantiation.get(), "to_stdout", tagem_module::to_stdout);
	/*PyObject_SetAttrString(tagem_module::modul, "ytdl_instantiation", ytdl_instantiation);
	PyRun_String("tagem.ytdl_instantiation.to_stdout = tagem.to_stdout", Py_single_input, nullptr, nullptr);
	// The C API call acts differently - the first argument is NOT a pointer to the 'self' object, but remains a pointer to the module object*/
	
	PyObjectPtr fn_args = make_py_obj_ptr(PyTuple_New(1));
	PyObjectPtr urls_ls = make_py_obj_ptr(Py_BuildValue("[s]", url)); // Or [item]?
	PyTuple_SetItem(fn_args.get(), 0, urls_ls.get());
	PyObjectPtr ytdl_fn = make_py_obj_ptr(PyObject_GetAttrString(ytdl_instantiation.get(), "download"));
	PY_ASSERT_NOT_NULL(ytdl_fn.get(), "ytdl_fn is NULL");
	PyObjectPtr result = make_py_obj_ptr(PyObject_CallObject(ytdl_fn.get(), fn_args.get()));
	
	Py_ssize_t sz;
	const char* const _file_path = PyUnicode_AsUTF8AndSize(tagem_module::file_path.get(), &sz);
	memcpy(out_fmt_as_input__resulting_fp_as_output, _file_path, sz);
	out_fmt_as_input__resulting_fp_as_output[sz] = 0;
	
	return false;
}

} // namespace python
