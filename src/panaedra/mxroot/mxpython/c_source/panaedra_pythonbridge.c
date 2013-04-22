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

static void *oPyObject = 0;

PyMODINIT_FUNC
  QxPy_InitializeInterpreter(char *cPyExePathIP)
{
  // This function should be called before Py_Initialize() is called for the first time, if at all
  Py_SetProgramName(cPyExePathIP);
  Py_Initialize();
  oPyObject = (PyCodeObject*)Py_CompileString("oHoi = 'Boe'\nprint 'Hallo'", "dummydummy", Py_file_input);
  fprintf(stdout, "Initialize py object pointer: \"%p\"\n", oPyObject);
}

PyMODINIT_FUNC
  QxPy_GetCompiledPyCode(char *cPyCodeIP, void *oPyObjectOP)
{  
  fprintf(stdout, "Get py object pointer: \"%p\"\n", oPyObject);
  oPyObjectOP = oPyObject;
}

PyMODINIT_FUNC
  QxPy_RunCompiledPyCode(void **oPyObjectIP)
{  
  PyObject* main_module = 0;
  PyObject* global_dict = 0; 
  PyObject* local_dict = 0;
  PyObject* pRet = 0;

  PyObject* pStrong = 0;

  main_module = PyImport_AddModule("__main__");
  global_dict = PyModule_GetDict(main_module);
  local_dict = PyDict_New();

  fprintf(stdout, "Run py object pointer: \"%p\"\n", oPyObject);
  //fprintf(stdout, "Run py object pointer: \"%p\"-\"%p\"\n", oPyObject, (void*)(*((void)oPyObjectIP)));

  //pRet = PyEval_EvalCode((PyCodeObject*)oPyObjectIP, global_dict, local_dict);
  fprintf(stdout, "pRet object pointer: \"%p\"\n", ((void *)pRet));
  if (pRet != 0)
  {
    pStrong = PyObject_Str((PyObject*)pRet);
    PyObject_Print(pStrong, stdout, 0); // Should be 'None'
      fprintf(stdout, "\n");
      Py_DECREF(pRet);
  }

  Py_DECREF(main_module);
  Py_DECREF(global_dict);
  Py_DECREF(local_dict);
}

PyMODINIT_FUNC
  QxPy_FreeCompiledPyCode(void **oPyObjectIP)
{  
  fprintf(stdout, "Del py object pointer: \"%p\" \"%p\"\n", oPyObjectIP, *oPyObjectIP);
  Py_DECREF(oPyObjectIP);
}


PyMODINIT_FUNC 
  QxPy_FinalizeInterpreter()
{
  Py_Finalize();
}


