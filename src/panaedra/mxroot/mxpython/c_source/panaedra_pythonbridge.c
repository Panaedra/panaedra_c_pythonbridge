#include <Python.h>

#ifndef __bool_true_false_are_defined
    #ifdef _Bool
        #define bool                        _Bool
    #else
        #define bool                        char
    #endif
    #define true                            1
    #define false                           0
    #define __bool_true_false_are_defined   1
#endif 

int main(int argc, char *argv[])
{
	Py_Initialize();
	PyRun_SimpleString("from time import time,ctime\n"
		"print 'Today it is ',ctime(time())\n");
	Py_Finalize();
	return 0;
}

PyMODINIT_FUNC
	QxPy_Initialize()
{
	Py_Initialize();
}

PyMODINIT_FUNC 
	QxPy_Finalize()
{
	Py_Finalize();
}

PyMODINIT_FUNC
	QxPy_SetProgramName(char *name)
{
	// This function should be called before Py_Initialize() is called for the first time, if at all
	Py_SetProgramName(name);
}

PyMODINIT_FUNC
	QxPy_Testing()
{
	PyRun_SimpleString("from time import time,ctime\n"
		"print 'Vandaag is het ',ctime(time())\n");
}

PyMODINIT_FUNC
	QxPy_TestEmail()
{
	PyRun_SimpleString(
		//"msg = 'Shall I?'\n"
		//   "shall = True if raw_input(\"%s (y/N) \" % msg).lower() == 'y' else False\n"
		//   "print(shall)\n"

		"import smtplib                                            \n"
		"														   \n"
		"SERVER = \"srv-mercury\"									   \n"
		"														   \n"
		"FROM = \"qx_python.noreply@domain.com\"						   \n"
		"TO = [\"_PPL_UNDISCLOSED_.noreply@domain.com\"] # must be a list			   \n"
		"														   \n"
		"SUBJECT = \"Hello!\"									   \n"
		"														   \n"
		"TEXT = \"This message was sent with Python's smtplib.\"   \n"
		"														   \n"
		"# Prepare actual message								   \n"
		"														   \n"
		"message = \"\"\"										   \n"
		"From: %s												   \n"
		"To: %s													   \n"
		"Subject: %s											   \n"
		"														   \n"
		"%s														   \n"
		"\"\"\" % (FROM, \", \".join(TO), SUBJECT, TEXT)		   \n"
		"server = smtplib.SMTP(SERVER)       \n"
		"server.sendmail(FROM, TO, message)	 \n"
		"server.quit()						 \n"
		);
}

PyMODINIT_FUNC
	QxC_Test1(int *i){
		*i+=5;
}

PyMODINIT_FUNC
  QxC_Test2(int i){
    i+=5;
}

PyMODINIT_FUNC
  QxC_Test3(int i, int* i2){
    *i2=i + 5;
}

PyMODINIT_FUNC
  QxPy_Test0(){
    PyRun_SimpleString("oHoi=None\n");
}

PyMODINIT_FUNC
  QxPy_Test01(){
    PyRun_SimpleString("oHoi01=None\noHoi02=None\noHoi03=None\noHoi04=None\noHoi05=None\noHoi06=None\noHoi07=None\noHoi08=None\noHoi09=None\noHoi10=None\n");
}

PyMODINIT_FUNC
	QxPy_Test1(int *i){
		*i+=5;
		//PyRun_SimpleString("print(\"hoi\")\n");
		PyRun_SimpleString("oHoi=None\n");
}

PyMODINIT_FUNC
	QxPy_Test2(char *c){
	//fprintf(stdout, "Recieved: \"%s\"\n", c);
}

PyMODINIT_FUNC
  QxPy_Test3(char *c){
  //fprintf(stdout, "Recieved: \"%s\"\n", c);
}


PyMODINIT_FUNC
  QxPy_Test4(char *c){
  PyCodeObject* pCode = 0;
  pCode = (PyCodeObject*)Py_CompileString(c, "dummydummy", Py_file_input); //  Py_eval_input 
  fprintf(stdout, "py object pointer: \"%p\"\n", ((void *)pCode));
  //fprintf(stdout, "Recieved: \"%s\"\n", c);
  
  //PyObject* pStrong = PyObject_Str((PyObject*)pCode);
  //PyObject_Print(pStrong, stdout, 0); // geeft: '<code object <module> at 112b3deb0, file "dummydummy", line 1>'
  PyObject* main_module = PyImport_AddModule("__main__");
  

  PyObject* global_dict = PyModule_GetDict(main_module); // 
  PyObject* local_dict = PyDict_New();
  int i;
  PyObject* pRet = 0;
  for (i = 0; i < 1000000; i++)
  {
    pRet = PyEval_EvalCode(pCode, global_dict, local_dict);
  }
  
  fprintf(stdout, "pRet object pointer: \"%p\"\n", ((void *)pRet));
  if (pRet != 0)
  {
    PyObject* pStrong = PyObject_Str((PyObject*)pRet);
    PyObject_Print(pStrong, stdout, 0); // geeft: 'None', python None type dus. 0 betekent: mislukt.
  }
  
  //PyObject* pRet = 0;
  //pRet = PyEval_EvalCode(pCode, 0, 0); //PyObject *globals , PyObject *locals
  
}


PyMODINIT_FUNC
	QxPy_TestCompleteExample(char* script, char* func, long* var1, long* var2, int* iError)
{
	PyObject *pName, *pModule, *pFunc;
	PyObject *pArgs, *pValue;
	bool bFunctionFail;

	bFunctionFail = false;

	pName = PyString_FromString(script);
	/* Error checking of pName left out */

	pModule = PyImport_Import(pName);
	Py_DECREF(pName);

	if (pModule != NULL) {
		pFunc = PyObject_GetAttrString(pModule, func);
		/* pFunc is a new reference */

		if (pFunc && PyCallable_Check(pFunc)) {
			pArgs = PyTuple_New(2); // size of tuple

			pValue = PyInt_FromLong(*var1);
			if (!pValue) {
				Py_DECREF(pArgs);
				Py_DECREF(pModule);
				fprintf(stderr, "Cannot convert first argument\n");
				*iError = 1;
				return;
			}
			PyTuple_SetItem(pArgs, 0, pValue);

			pValue = PyInt_FromLong(*var2);
			if (!pValue) {
				Py_DECREF(pArgs);
				Py_DECREF(pModule);
				fprintf(stderr, "Cannot convert second argument\n");
				*iError = 1;
				return;
			}
			PyTuple_SetItem(pArgs, 1, pValue);

			pValue = PyObject_CallObject(pFunc, pArgs);
			Py_DECREF(pArgs);
			if (pValue != NULL) {
				printf("Result of call: %ld\n", PyInt_AsLong(pValue));
				Py_DECREF(pValue);
			}
			else {
				Py_DECREF(pFunc);
				Py_DECREF(pModule);
				PyErr_Print();
				fprintf(stderr,"Call failed\n");
				*iError = 1;
				return;
			}
		}
		else {
			if (PyErr_Occurred())
				PyErr_Print();
			fprintf(stderr, "Cannot find function \"%s\"\n", func);
			bFunctionFail = true;
		}
		Py_XDECREF(pFunc);
		Py_DECREF(pModule);
		if (bFunctionFail)
		{
			*iError = 1;
			return;
		}
	}
	else {
		PyErr_Print();
		fprintf(stderr, "Failed to load \"%s\"\n", script);
		*iError = 1;
		return;
	}
	*iError = 0;
}
