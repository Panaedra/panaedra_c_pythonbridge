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

	fprintf(stdout, "Initialize iMaxModulesIP: \"%i\"\n", iMaxModulesIP);

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
}

PyMODINIT_FUNC
	QxPy_SetCompiledPyCode(int iPyObjectIP, char *cPyIdIP, char *cPyCodeIP, char *cErrorOP)
{  
	void *oPyObject = 0;

	cErrorOP[0] = 0;

	oPyObject = (PyCodeObject*)Py_CompileString(cPyCodeIP, cPyIdIP, Py_file_input);
	if (oPyObject == 0)
	{
		char* cErr = "ERROR: Compile error, invalid python code.\t";
		strncat(cErrorOP, cErr, strnlen(cErr,MAXERRORLEN - strlen(cErrorOP) - 1));
		cErr = cPyIdIP;
		strncat(cErrorOP, cErr, strnlen(cErr,MAXERRORLEN - strlen(cErrorOP) - 1));
		cErr = ":\t";
		strncat(cErrorOP, cErr, strnlen(cErr,MAXERRORLEN - strlen(cErrorOP) - 1));
		{
			PyObject *ptype, *pvalue, *ptraceback;
			
			PyErr_Fetch(&ptype, &pvalue, &ptraceback); // Note: This fetch resets the error flag
            
			// Store results in cErrorOP
			cErr = PyString_AsString(PyObject_Str(ptype));
			strcat(cErr,"\t");
    		strncat(cErrorOP, cErr, strnlen(cErr,MAXERRORLEN - strlen(cErrorOP) - 1));

			cErr = PyString_AsString(PyObject_Str(pvalue));
			strcat(cErr,"\t");
    		strncat(cErrorOP, cErr, strnlen(cErr,MAXERRORLEN - strlen(cErrorOP) - 1));

			cErr = PyString_AsString(PyObject_Str(ptraceback));
    		strncat(cErrorOP, cErr, strnlen(cErr,MAXERRORLEN - strlen(cErrorOP) - 1));
			
		}
	}
	else
	{
		fprintf(stdout, "Initialize py object pointer: \"%p\" \"%s\"\n", oPyObject, cPyCodeIP);
		pModules[iPyObjectIP] = oPyObject;
	}

}

PyMODINIT_FUNC
	QxPy_RunCompiledPyCode(int iModuleIP)
{  
	if (pModules[iModuleIP] != 0)
	{
		PyObject* local_dict = 0;
		PyObject* pRet = 0;

		PyObject* pStrong = 0;

		fprintf(stdout, "QxPy_RunCompiledPyCode\"%i\"\n", iModuleIP);
		fprintf(stdout, "Run py object pointer: \"%p\" \"%p\"\n", pModules[0], pModules[1]);

		if (oMainModule == 0) oMainModule = PyImport_AddModule("__main__");
		if (oGlobalDict == 0) oGlobalDict = PyModule_GetDict(oMainModule);

		fprintf(stdout, "Run py object main module: \"%p\"\n", oMainModule);
		local_dict = PyDict_New();

		//fprintf(stdout, "Run py object pointer: \"%p\"-\"%p\"\n", oPyObject, (void*)(*((void)oPyObjectIP)));

		pRet = PyEval_EvalCode((PyCodeObject*)pModules[iModuleIP], oGlobalDict, local_dict);
		fprintf(stdout, "pRet object pointer: \"%p\"\n", ((void *)pRet));
		if (pRet != 0)
		{
			pStrong = PyObject_Str((PyObject*)pRet);
			PyObject_Print(pStrong, stdout, 0); // Should be 'None'
			fprintf(stdout, "\n");
			Py_DECREF(pRet);
		}
		Py_DECREF(local_dict);
	}
}

PyMODINIT_FUNC
	QxPy_FreeCompiledPyCode(int iPyObjectIP)
{  
	fprintf(stdout, "Del py object number: \"%i\"\n", iPyObjectIP);
	if (iPyObjectIP <= iMaxModules && pModules[iPyObjectIP] != 0) 
	{
		Py_DECREF(pModules[iPyObjectIP]);
		pModules[iPyObjectIP] = 0;
	}
}


PyMODINIT_FUNC 
	QxPy_FinalizeInterpreter()
{

	int i = 0;

	// Free malloc of pModules void pointer array
	fprintf(stdout, "Freeing pModules: \"%p\" \"%p\"\n", pModules, pModules[0]);
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

}


