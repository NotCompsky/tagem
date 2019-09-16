#include <Python.h>
#include "menu.hpp"
#include <QVBoxLayout>

#include <QDebug>


Menu::Menu()
{
	PyObject* __main__ = PyImport_AddModule("__main__");
	PyObject* globals = PyModule_GetDict(__main__);
	if (!globals)
		qDebug() << "globals == NULL";
	PyObject* options = PyDict_GetItemString(globals, "menu_options");
	if (!options)
		qDebug() << "options == NULL";
	
	const Py_ssize_t n_options = PyList_Size(options);
	this->btns.reserve(n_options);
	
	QVBoxLayout* l = new QVBoxLayout;
	
	for (auto i = 0;  i < n_options;  ++i){
		PyObject* option_pystr = PyList_GetItem(options, i);
		const char* const option = PyBytes_AsString(option_pystr);
		QPushButton* btn = new QPushButton(option);
		this->btns.push_back(btn);
		connect(btn, &QPushButton::clicked, this, &Menu::option_chosen);
		l->addWidget(btn);
	}
	
	this->setLayout(l);
	
	Py_DECREF(__main__);
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
