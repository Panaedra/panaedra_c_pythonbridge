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

PyMODINIT_FUNC
  QxPy_InitializeInterpreter(char *cPyExePathIP, int iMaxModulesIP)
{

  int i;
  void *oPyObject = 0;

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
    oPyObject = (PyCodeObject*)Py_CompileString("oHoi = 'Boe'\nprint 'Hallo dit is de eerste'\n", "dummydummy1", Py_file_input);
    fprintf(stdout, "Initialize py object pointer nr 1: \"%p\"\n", oPyObject);
    pModules[0] = oPyObject;
    oPyObject = (PyCodeObject*)Py_CompileString("oHoi = 'Ba'\nprint 'Hallo dit is de tweede'", "dummydummy2", Py_file_input);
    fprintf(stdout, "Initialize py object pointer nr 2: \"%p\"\n", oPyObject);
    pModules[1] = oPyObject;
  }
}

PyMODINIT_FUNC
  QxPy_SetCompiledPyCode(int iPyObjectIP, char *cPyCodeIP)
{  
  fprintf(stdout, "Set py object pointer: \"%p\"\n", pModules[0]);
  //pModules[iPyObjectIP] = ...;
}

PyMODINIT_FUNC
  QxPy_RunCompiledPyCode(int iModuleIP)
{  
  PyObject* main_module = 0;
  PyObject* global_dict = 0; 
  PyObject* local_dict = 0;
  PyObject* pRet = 0;

  PyObject* pStrong = 0;

  fprintf(stdout, "QxPy_RunCompiledPyCode: \"%i\"\n", iModuleIP);
  fprintf(stdout, "Run py object pointer: \"%p\" \"%p\"\n", pModules[0], pModules[1]);

  main_module = PyImport_AddModule("__main__");
  fprintf(stdout, "Run py object main module: \"%p\"\n", main_module);
  global_dict = PyModule_GetDict(main_module);
  local_dict = PyDict_New();

  
  //fprintf(stdout, "Run py object pointer: \"%p\"-\"%p\"\n", oPyObject, (void*)(*((void)oPyObjectIP)));

  pRet = PyEval_EvalCode((PyCodeObject*)pModules[0], global_dict, local_dict);
  fprintf(stdout, "pRet object pointer: \"%p\"\n", ((void *)pRet));
  if (pRet != 0)
  {
    pStrong = PyObject_Str((PyObject*)pRet);
    PyObject_Print(pStrong, stdout, 0); // Should be 'None'
    fprintf(stdout, "\n");
    Py_DECREF(pRet);
  }
  pRet = PyEval_EvalCode((PyCodeObject*)pModules[1], global_dict, local_dict);
  fprintf(stdout, "pRet object pointer: \"%p\"\n", ((void *)pRet));
  if (pRet != 0)
  {
    pStrong = PyObject_Str((PyObject*)pRet);
    PyObject_Print(pStrong, stdout, 0); // Should be 'None'
    fprintf(stdout, "\n");
    Py_DECREF(pRet);
  }

  //Py_DECREF(main_module);
  Py_DECREF(global_dict);
  Py_DECREF(local_dict);
}

PyMODINIT_FUNC
  QxPy_FreeCompiledPyCode(int iPyObjectIP)
{  
  fprintf(stdout, "Del py object number: \"%i\"\n", iPyObjectIP);
  //Py_DECREF(oPyObjectIP);
}


PyMODINIT_FUNC 
  QxPy_FinalizeInterpreter()
{

  int i = 0;

  Py_Finalize();

  // Free malloc of pModules void pointer array
  fprintf(stdout, "Freeing pModules: \"%p\" \"%p\"\n", pModules, pModules[0]);
    for (i = 0 ; i < iMaxModules ; i++)
    {
      if (pModules[i] != 0) 
      {
        Py_DECREF(pModules[i]);
      }
    }

  free(pModules);
  pModules = 0;

}


