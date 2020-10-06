#pragma once

#include <Python.h> // WARNING: Must be before any Qt includes, because slots is a macro name.


#define PY_ASSERT_NOT_NULL(obj, msg) \
	if (unlikely(obj == nullptr)) \
		throw std::runtime_error("Python error: " msg);
#define PY_ASSERT_ZERO(rc, msg) \
	if (unlikely(rc != 0)) \
		throw std::runtime_error("Python error: " msg);


namespace python {


class PyObj {
 public:
	PyObject* obj;
	
	
	void set_attr(const char* const attr_name,  PyObject* const _obj){
		log("Setting attribute: ", attr_name);
		PyObject_SetAttrString(this->obj, attr_name, _obj);
	}
	
	PyObject* get_attr(const char* const attr_name) const {
		log("Getting attribute: ", attr_name);
		PyObject* const rc = PyObject_GetAttrString(this->obj, attr_name);
		PY_ASSERT_NOT_NULL(rc, "Cannot get attribute")
		return rc;
	}
	
	
	/* Call self */
	
	template<size_t n,  typename... Args>
	PyObject* call(Args... args){
		this->print_as_str("Calling object: ");
		PyObject* const result = PyObject_CallObject(this->obj, PyObj(PyTuple_Pack(n, args...)).obj);
		PY_ASSERT_NOT_NULL(result, "Cannot call object")
		return result;
	}
	
	/* Call self without return value */
	
	template<size_t n,  typename... Args>
	void call_void(Args... args){
		Py_DECREF(this->call<n>(args...));
	}
	
	/* Call member function */
	
	template<size_t n,  typename... Args>
	PyObject* call_fn(const char* const fn_name,  Args... args){
		PyObj fn(this->obj, fn_name);
		log("Created fn");
		fn.print_as_str("Calling function: ");
		return PyObject_CallFunctionObjArgs(fn.obj, args..., NULL);
	}
	
	/* Call member function without return value */
	
	template<size_t n,  typename... Args>
	void call_fn_void(Args... args){
		PyObject* result = this->call_fn<n>(args...);
		if (unlikely(result == nullptr))
			// Python exception
			throw std::runtime_error("Python exception");
		Py_DECREF(result);
	}
	
	
	const char* as_str() const {
		return PyUnicode_AsUTF8(this->obj);
	}
	
	void copy_str(char* buf) const {
		Py_ssize_t sz;
		const char* const str = PyUnicode_AsUTF8AndSize(this->obj, &sz);
		memcpy(buf, str, sz);
		buf[sz] = 0;
	}
	
	void print_as_str(const char* const msg) const {
		log(msg, PyUnicode_AsUTF8(PyObject_Str(this->obj)));
	}
	
	
	PyObj(){}
	
	PyObj(PyObject* const _obj)
	: obj(_obj)
	{
		PY_ASSERT_NOT_NULL(this->obj, "this->obj is NULL")
		
		this->print_as_str("Created: ");
	}
	
	PyObj(PyObject* const _obj,  const char* const attr_name)
	: PyObj(PyObject_GetAttrString(_obj, attr_name))
	{
		log("Created object from attribute: ", attr_name);
	}
	
	PyObj(PyObj& _obj,  const char* const attr_name)
	: PyObj(_obj.obj, attr_name)
	{}
	
	~PyObj(){
		this->print_as_str("Destroyed: ");
		//Py_DECREF(this->obj);
	}
	
	PyObj& operator=(PyObj& other){
		this->print_as_str("Copying: ");
		this->obj = other.obj;
		return *this;
	}
};

template<size_t n>
class PyDict : public PyObj {
 private:
	PyObject* vals[n];
	
	template<size_t i>
	void instatiate(){
		static_assert(i == 0);
	}
	
	template<size_t i,  typename... Args>
	void instatiate(const char* const _key,  PyObject* const _obj,  Args... args){
		this->vals[n-i] = _obj;
		PY_ASSERT_ZERO(PyDict_SetItemString(this->obj, _key, _obj), "Cannot set dictionary item")
		this->instatiate<i-1>(args...);
	}
 public:
	template<typename... Args>
	PyDict(Args... args)
	: PyObj(PyDict_New())
	{
		this->instatiate<n>(args...);
	}
	
	~PyDict(){
		for (PyObject* const _key : this->vals){
			Py_DECREF(_key);
		}
		Py_DECREF(this->obj);
	}
	
	PyDict& operator=(PyDict& other){
		memcpy(this->vals, other.vals, sizeof(vals));
		this->obj = other.obj;
		return *this;
	}
};


} // namespace python
