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

#define MAXERRORLEN 10000 // cErrorOP parameters should be preallocated with this size
#define QXPYDEBUG 0

//static void *oPyObject = 0;
static void** pModules = 0;
static int iMaxModules = 0;
static PyObject* oMainModule = 0;
static PyObject* oGlobalDict = 0; 

PyMODINIT_FUNC
	QxPy_InitializeInterpreter(char *cPyExePathIP, int iMaxModulesIP, char *cErrorOP)
{

	int i;

	iMaxModules = iMaxModulesIP;
	cErrorOP[0] = 0;

    #if QXPYDEBUG
	fprintf(stdout, "Initialize iMaxModulesIP: \"%i\"\n", iMaxModulesIP);
    #endif

	pModules = (void**)malloc(sizeof(void*) * iMaxModulesIP);
	if (pModules == 0)
	{
		char* cErr = "ERROR: Out of memory\n";
		strncat(cErrorOP, cErr, strnlen(cErr,MAXERRORLEN - strlen(cErrorOP) - 1));
	}
	else
	{
		for (i = 0 ; i < iMaxModulesIP ; i++)
		{
			pModules[i] = 0;
		}

		// This function should be called before Py_Initialize() is called for the first time, if at all
		Py_SetProgramName(cPyExePathIP);

		// Initialize the Python interpreter
		Py_Initialize();

	}

} // QxPy_InitializeInterpreter

void QxPyH_TransferPyErrorToString(char *cErrorOP)
{
	// Transfer the python error to a string (concat), and reset the python error status

	char *cErr;
	PyObject *oPyObject = 0;
	PyObject *oPyType, *oPyValue, *oPyTraceback;

	PyErr_Fetch(&oPyType, &oPyValue, &oPyTraceback); // Note: This fetch resets the error flag

	// Store results in cErrorOP
	if (oPyType != 0)
	{
		oPyObject = PyObject_Str(oPyType);
		if (oPyObject != 0)
		{
			cErr = PyString_AsString(oPyObject);
			strcat(cErr,"\t");
			strncat(cErrorOP, cErr, strnlen(cErr,MAXERRORLEN - strlen(cErrorOP) - 1));
			Py_DECREF(oPyObject);
		}
		Py_DECREF(oPyType);
	}

	if (oPyValue != 0)
	{
		oPyObject = PyObject_Str(oPyValue);
		if (oPyObject != 0)
		{
			cErr = PyString_AsString(oPyObject);
			strcat(cErr,"\t");
			strncat(cErrorOP, cErr, strnlen(cErr,MAXERRORLEN - strlen(cErrorOP) - 1));
			Py_DECREF(oPyObject);
		}
		Py_DECREF(oPyValue);
	}

	if (oPyTraceback != 0)
	{
		oPyObject = PyObject_Str(oPyTraceback);
		if (oPyObject != 0)
		{
			cErr = PyString_AsString(oPyObject);
			strncat(cErrorOP, cErr, strnlen(cErr,MAXERRORLEN - strlen(cErrorOP) - 1));
			Py_DECREF(oPyObject);
		}
		Py_DECREF(oPyTraceback);
	}

} // QxPyH_TransferPyErrorToString

PyMODINIT_FUNC
	QxPy_SetCompiledPyCode(int iPyObjectIP, char *cPyIdIP, char *cPyCodeIP, char *cErrorOP)
{  
	void *oPyObject = 0;

	cErrorOP[0] = 0;
	cErrorOP[1] = 0;
	cErrorOP[2] = 0;
	cErrorOP[3] = 0;

	oPyObject = (PyCodeObject*)Py_CompileString(cPyCodeIP, cPyIdIP, Py_file_input);
	if (oPyObject == 0)
	{
		char* cErr = "Compile error, invalid python code.\t";
		strncat(cErrorOP, cErr, strnlen(cErr,MAXERRORLEN - strlen(cErrorOP) - 1));
		cErr = cPyIdIP;
		strncat(cErrorOP, cErr, strnlen(cErr,MAXERRORLEN - strlen(cErrorOP) - 1));
		cErr = ":\t";
		strncat(cErrorOP, cErr, strnlen(cErr,MAXERRORLEN - strlen(cErrorOP) - 1));
		QxPyH_TransferPyErrorToString(cErrorOP);
	}
	else
	{
		#if QXPYDEBUG
		fprintf(stdout, "Initialize py object pointer: \"%p\" \"%s\"\n", oPyObject, cPyCodeIP);
		#endif
		pModules[iPyObjectIP] = oPyObject;
	}

} // QxPy_SetCompiledPyCode


PyMODINIT_FUNC
	QxPy_RunCompiledPyCode(int iModuleIP, char *cErrorOP)
{  
	cErrorOP[0] = 0;
	cErrorOP[1] = 0;

	if (pModules[iModuleIP] != 0)
	{
		PyObject* local_dict = 0;
		PyObject* pRet = 0;

		PyObject* pStrong = 0;

		//fprintf(stdout, "QxPy_RunCompiledPyCode\"%i\"\n", iModuleIP);
		//fprintf(stdout, "Run py object pointer: \"%p\" \"%p\"\n", pModules[0], pModules[1]);

		if (oMainModule == 0) oMainModule = PyImport_AddModule("__main__");
		if (oGlobalDict == 0) oGlobalDict = PyModule_GetDict(oMainModule);

		//fprintf(stdout, "Run py object main module: \"%p\"\n", oMainModule);
		local_dict = PyDict_New();

		pRet = PyEval_EvalCode((PyCodeObject*)pModules[iModuleIP], oGlobalDict, local_dict);
		//fprintf(stdout, "pRet object pointer: \"%p\"\n", ((void *)pRet));
		if (pRet != 0)
		{
			pStrong = PyObject_Str((PyObject*)pRet);
			//PyObject_Print(pStrong, stdout, 0); // Should be 'None'
			//fprintf(stdout, "\n");
			if (pStrong != 0) Py_DECREF(pStrong);
			Py_DECREF(pRet);
		}
		Py_DECREF(local_dict);
	}

} // QxPy_RunCompiledPyCode


PyMODINIT_FUNC
	QxPy_FreeCompiledPyCode(int iPyObjectIP)
{  
	#if QXPYDEBUG
	fprintf(stdout, "Del py object number: \"%i\"\n", iPyObjectIP);
	#endif

	if (iPyObjectIP <= iMaxModules && pModules[iPyObjectIP] != 0) 
	{
		Py_DECREF(pModules[iPyObjectIP]);
		pModules[iPyObjectIP] = 0;
	}

} // QxPy_FreeCompiledPyCode


PyMODINIT_FUNC 
	QxPy_FinalizeInterpreter()
{

	int i = 0;

	// Free malloc of pModules void pointer array
	#if QXPYDEBUG
	fprintf(stdout, "Freeing pModules: \"%p\" \"%p\"\n", pModules, pModules[0]);
	#endif

	for (i = 0 ; i < iMaxModules ; i++)
	{
		if (pModules[i] != 0) 
		{
			Py_DECREF(pModules[i]);
			pModules[i] = 0;
		}
	}

	free(pModules);
	pModules = 0;

	if (oGlobalDict != 0)
	{
		Py_DECREF(oGlobalDict);
	}

	// Note: Py_DECREF(oMainModule) gave a Mem Violation in OE10.2B batch session. We don't create it ourselves, so it's probably not meant to be decreased anyway. Plus we only need one instance of the main module.

	Py_Finalize();

} // QxPy_FinalizeInterpreter




