#include <Python.h>
#include "menu.hpp"
#include <QVBoxLayout>

#include <QDebug>


Menu::Menu()
{
	PyObject* __main__ = PyImport_AddModule("__main__");
	PyObject* options = PyObject_GetAttrString(__main__, "menu_options");
	if (!options)
		qDebug() << "options == NULL";
	
	const Py_ssize_t n_options = PyList_Size(options);
	this->btns.reserve(n_options);
	
	QVBoxLayout* l = new QVBoxLayout;
	
	for (auto i = 0;  i < n_options;  ++i){
		PyObject* option_pystr = PyList_GetItem(options, i);
		
		PyObject* option_pyutf8str = PyUnicode_AsUTF8String(option_pystr);
		const char* const option = PyBytes_AsString(option_pyutf8str);
		
		QPushButton* btn = new QPushButton(option);
		this->btns.push_back(btn);
		connect(btn, &QPushButton::clicked, this, &Menu::option_chosen);
		l->addWidget(btn);
		
		Py_DECREF(option_pyutf8str);
		Py_DECREF(option_pystr);
	}
	
	this->setLayout(l);
	
	Py_DECREF(options);
}


void Menu::option_chosen(){
	unsigned int indx = 0;
	for (QPushButton* const btn : this->btns){
		if (btn != sender()){
			++indx;
			continue;
		}
		this->done(indx);
		return;
	}
};
