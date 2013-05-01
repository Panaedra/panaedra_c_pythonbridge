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

//static void *oPyObject = 0;
static void** pModules = 0;
static int iMaxModules = 0;
static PyObject* oMainModule = 0;
static PyObject* oGlobalDict = 0; 

PyMODINIT_FUNC
	QxPy_InitializeInterpreter(char *cPyExePathIP, int iMaxModulesIP)
{

	int i;
	//void *oPyObject = 0;

	iMaxModules = iMaxModulesIP;

	fprintf(stdout, "Initialize iMaxModulesIP: \"%i\"\n", iMaxModulesIP);

	pModules = (void**)malloc(sizeof(void*) * iMaxModulesIP);
	if (pModules == 0)
	{
		printf("ERROR: Out of memory\n");
	}
	else
	{
		for (i = 0 ; i < iMaxModulesIP ; i++)
		{
			pModules[i] = 0;
		}

		// This function should be called before Py_Initialize() is called for the first time, if at all
		Py_SetProgramName(cPyExePathIP);
		Py_Initialize();

		//oPyObject = (PyCodeObject*)Py_CompileString("oHoi = 'Boe'\nprint 'Hallo dit is de eerste'\n", "dummydummy1", Py_file_input);
		//fprintf(stdout, "Initialize py object pointer nr 1: \"%p\"\n", oPyObject);
		//pModules[0] = oPyObject;
		//oPyObject = (PyCodeObject*)Py_CompileString("oHoi = 'Ba'\nprint 'Hallo dit is de tweede'", "dummydummy2", Py_file_input);
		//fprintf(stdout, "Initialize py object pointer nr 2: \"%p\"\n", oPyObject);
		//pModules[1] = oPyObject;
	}
}

PyMODINIT_FUNC
	QxPy_SetCompiledPyCode(int iPyObjectIP, char *cPyCodeIP)
{  
	void *oPyObject = 0;

	oPyObject = (PyCodeObject*)Py_CompileString(cPyCodeIP, "dummydummy1", Py_file_input);
	fprintf(stdout, "Initialize py object pointer: \"%p\" \"%s\"\n", oPyObject, cPyCodeIP);
	pModules[iPyObjectIP] = oPyObject;

	//fprintf(stdout, "Set py object pointer: \"%p\"\n", pModules[0]);
}

PyMODINIT_FUNC
	QxPy_RunCompiledPyCode(int iModuleIP)
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


