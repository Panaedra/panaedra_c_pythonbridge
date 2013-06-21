#include <Python.h>
#include <setjmp.h>

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

#ifndef _WIN32
#if defined(__cplusplus)
#  define PyMODINIT_PCHAR extern "C" __declspec(dllexport) char*
#else /* __cplusplus */
#  define PyMODINIT_PCHAR char*
#endif /* __cplusplus */
#else
#if defined(__cplusplus)
#  define PyMODINIT_PCHAR extern "C" __declspec(dllexport) char*
#else /* __cplusplus */
#  define PyMODINIT_PCHAR __declspec(dllexport) char*
#endif /* __cplusplus */
#endif /* _WIN32 */

// See: http://www.di.unipi.it/~nids/docs/longjump_try_trow_catch.html
#define TRY do{ jmp_buf ex_buf__; if( !setjmp(ex_buf__) ){
#define CATCH } else {
#define ETRY } }while(0)
#define THROW longjmp(ex_buf__, 1)

#define DATAOP_BUFFERED 1
#define DATAOP_UNBUFFERED 2

static void** pModules = 0;
static int iMaxModules = 0;
static PyObject* oMainModule = 0;
static PyObject* oGlobalDict = 0; 
static PyObject* oLastUnbuffered = 0; 
static char* pNullString = "";

PyMODINIT_FUNC
  QxPy_InitializeInterpreter(char *cPyExePathIP, int iMaxModulesIP, char *cErrorOP)
{

  int i = 0;

  iMaxModules = iMaxModulesIP;
  cErrorOP[0] = 0;
  cErrorOP[1] = 0; // For Utf-8 data
  cErrorOP[2] = 0;
  cErrorOP[3] = 0; // For Utf-16 data

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
    TRY
    {

      for (i = 0 ; i < iMaxModulesIP ; i++)
      {
        pModules[i] = 0;
      }

      // This function should be called before Py_Initialize() is called for the first time, if at all
      Py_SetProgramName(cPyExePathIP);

      // Initialize the Python interpreter
      // Note: exceptions can not be caught here. 
      // A fatal error is induced by calling Py_FatalError, which bids farewell with an explanatory message and then calls abort().
      // Sadly, there is no way to un-abort that.
      Py_Initialize();

      // Got code from: http://lyricsgrabber2.googlecode.com/svn-history/r38/trunk/foo_lyricsgrabber2/foo_lyricsgrabber2/py_site.cpp 
      // More info here: https://fedoraproject.org/wiki/Features/PythonEncodingUsesSystemLocale
      if (PyUnicode_SetDefaultEncoding("utf-8") != 0)
      {
        PyErr_Clear();
      }

    }
    CATCH
    {
      char* cErr = "ERROR: Python initialize interpreter failed.\n";
      strncat(cErrorOP, cErr, strnlen(cErr,MAXERRORLEN - strlen(cErrorOP) - 1)) ;
    }
    ETRY;

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
    oPyObject = PyObject_Str(oPyType); // Note: PyObject_Repr can also be used, mimic 'repr', for easier copy/paste in python
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
  cErrorOP[1] = 0; // For Utf-8 data
  cErrorOP[2] = 0;
  cErrorOP[3] = 0; // For Utf-16 data

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


void
  RunCompiledPyCodeImplement(int iModuleIP, char *cDataIP, int iDataOpModeIP, char **cDataOP, long long iDataOpLengthIP, char *cErrorOP)
{  
  cErrorOP[0] = 0;
  cErrorOP[1] = 0; // For Utf-8 data
  cErrorOP[2] = 0;
  cErrorOP[3] = 0; // For Utf-16 data

  if (iDataOpModeIP == DATAOP_UNBUFFERED)
  {
    // For exeptional flows, initialize unbuffered pointer to a static null string
    *cDataOP = pNullString;
  }
  else
  {
    cDataOP[0] = 0;
    cDataOP[1] = 0; // For Utf-8 data
    cDataOP[2] = 0;
    cDataOP[3] = 0; // For Utf-16 data
  }

  if (oLastUnbuffered != 0)
  {
    #if QXPYDEBUG
    fprintf(stdout, "Del oLastUnbuffered: \"%p\"\n", oLastUnbuffered);
    #endif
    Py_DECREF(oLastUnbuffered);
    oLastUnbuffered = 0;
  }

  if (pModules[iModuleIP] != 0)
  {
    PyObject* oLocalDict = 0;
    PyObject* pRet = 0;
    PyObject* pPyUnicodeDataIP = 0;
    PyObject* pPyUnicodeDataOP = 0;
    PyObject* pPyUnicodeDataRet = 0;
    Py_ssize_t iPyDataLength = 0;
    int iRetData = 0;

    #if QXPYDEBUG
    PyObject* pStrong = 0;
    #endif

    #if QXPYDEBUG
    fprintf(stdout, "QxPy_RunCompiledPyCode\"%i\"\n", iModuleIP);
    fprintf(stdout, "Run py object pointer: \"%p\" \"%p\"\n", pModules[0], pModules[1]);
    #endif

    if (oMainModule == 0) oMainModule = PyImport_AddModule("__main__");
    if (oGlobalDict == 0) oGlobalDict = PyModule_GetDict(oMainModule);

    #if QXPYDEBUG
    fprintf(stdout, "Run py object main module: \"%p\"\n", oMainModule);
    #endif
    oLocalDict = PyDict_New();

    pPyUnicodeDataIP = PyUnicode_FromString(cDataIP);
    pPyUnicodeDataOP = PyUnicode_FromString(*cDataOP);

    #if QXPYDEBUG
    fprintf(stdout, "cDataIP: \"%s\"\n", cDataIP);
    fprintf(stdout, "pPyUnicodeDataIP: \"%p\"\n", pPyUnicodeDataIP);
    #endif

    PyDict_SetItemString(oLocalDict, "cDataIP", pPyUnicodeDataIP);
    PyDict_SetItemString(oLocalDict, "cDataOP", pPyUnicodeDataOP);

    // The dictionary holds the item now (reference count increase), so it won't be garbage collected. We can decrease the reference count by one.
    Py_DECREF(pPyUnicodeDataIP);
    Py_DECREF(pPyUnicodeDataOP);

    pRet = PyEval_EvalCode((PyCodeObject*)pModules[iModuleIP], oGlobalDict, oLocalDict);

    pPyUnicodeDataRet = PyDict_GetItemString(oLocalDict, "cDataOP");
    
    #if QXPYDEBUG
    fprintf(stdout, "pPyUnicodeDataOP after eval: \"%p\"\n", pPyUnicodeDataOP);
    fprintf(stdout, "pPyUnicodeDataRet after getitem: \"%p\"\n", pPyUnicodeDataRet);
    fprintf(stdout, "pPyUnicodeDataRet->ob_refcnt after getitem: \"%zd\"\n", pPyUnicodeDataRet->ob_refcnt);
    fprintf(stdout, "pRet object pointer: \"%p\"\n", ((void *)pRet));
    #endif

    if (pRet == 0)
    {
      char* cErr = "Runtime error, invalid python code.\t";
      strncat(cErrorOP, cErr, strnlen(cErr,MAXERRORLEN - strlen(cErrorOP) - 1));
      cErr = ":\t";
      strncat(cErrorOP, cErr, strnlen(cErr,MAXERRORLEN - strlen(cErrorOP) - 1));
      QxPyH_TransferPyErrorToString(cErrorOP);
    }
    else
    {
      char *pOutOP = 0; 

      #if QXPYDEBUG
      pStrong = PyObject_Str((PyObject*)pRet);
      PyObject_Print(pStrong, stdout, 0); // Should be 'None'
      fprintf(stdout, "\n");
      if (pStrong != 0) Py_DECREF(pStrong);
      PyObject_Print(pPyUnicodeDataRet, stdout, 0);
      fprintf(stdout, "\n");
      fprintf(stdout, "iPyDataLength: \"%zd\"\n", iPyDataLength);
      fprintf(stdout, "iRetData: \"%i\"\n", iRetData);
      fprintf(stdout, "cDataOP: \"%s\"\n", *cDataOP);
      fprintf(stdout, "pOutOP: \"%s\"\n", pOutOP);
      #endif

      iRetData = PyString_AsStringAndSize(pPyUnicodeDataRet, &pOutOP, &iPyDataLength); // Returns int

      if (iRetData != 0)
      {
          char* cErr = "Error: cDataOP string could not be retrieved from python locals() dictionary.\t"; 
          strncat(cErrorOP, cErr, strnlen(cErr,MAXERRORLEN - strlen(cErrorOP) - 1));
          QxPyH_TransferPyErrorToString(cErrorOP);
      }
      else
      {
        if (iDataOpModeIP != DATAOP_UNBUFFERED)
        {
          // Transfer the string from the Python world to the DLL world.
          // If we want to switch the owner of the data we need this deep copy. 
          // Otherwise, use the unbuffered alternative.
          strncpy(*cDataOP, pOutOP , (size_t)iDataOpLengthIP);

          if (iDataOpLengthIP < iPyDataLength)
          {
            char cErr[100]; 
            sprintf(cErr, "Warning: Output parameter truncated. Python string length \"%zd\" is larger than user-supplied maximum length \"%lld\".\t", iPyDataLength, iDataOpLengthIP);
            strncat(cErrorOP, cErr, strnlen(cErr,MAXERRORLEN - strlen(cErrorOP) - 1));
          }
        }
        else
        {
          // Unbuffered. Increase reference counter by 1, hold python object until next call.
          Py_IncRef(pPyUnicodeDataRet);
          oLastUnbuffered = pPyUnicodeDataRet;
          *cDataOP = pOutOP;
          #if QXPYDEBUG
          fprintf(stdout, "Unbuffered cDataOP: \"%s\"\n", *cDataOP);
          fprintf(stdout, "Unbuffered &pOutOP: \"%p\"\n", &pOutOP);
          #endif
        }
      }
      Py_DECREF(pRet);
    }
    Py_DECREF(oLocalDict);

    #if QXPYDEBUG
    // Note: pPyUnicodeDataRet is a borrowed reference, don't decrease.
    fprintf(stdout, "pPyUnicodeDataRet->ob_refcnt at end: \"%zd\"\n", pPyUnicodeDataRet->ob_refcnt);
    #endif

  }

} // RunCompiledPyCodeImplement


PyMODINIT_FUNC
  QxPy_RunCompiledPyCode(int iModuleIP, char *cDataIP, char *cDataOP, long long iDataOpLengthIP, char *cErrorOP)
{  
  RunCompiledPyCodeImplement(iModuleIP, cDataIP, DATAOP_BUFFERED, &cDataOP, iDataOpLengthIP, cErrorOP);
} // QxPy_RunCompiledPyCode


// Note: The only way to return an uninitialized memptr in OpenEdge ABL is by a return parameter.
PyMODINIT_PCHAR
  QxPy_RunCompiledPyCodeUnbuffered(int iModuleIP, char *cDataIP, char *cErrorOP)
{  
  char *cDataOP;
  // Essentially the same as QxPy_RunCompiledPyCode, but instead of a deep copy to cDataOP, a pointer to 
  // the char buffer of the (kept-alive) python unicode object is returned.
  // This buffer should not be modified in any way.
  // The python object (and the underlying string buffer) are cleaned up at the very next call, or at system exit.
  RunCompiledPyCodeImplement(iModuleIP, cDataIP, DATAOP_UNBUFFERED, &cDataOP, 0, cErrorOP);
  return cDataOP;
} // QxPy_RunCompiledPyCodeUnbuffered


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

  if (oLastUnbuffered != 0)
  {
    Py_DECREF(oLastUnbuffered);
    oLastUnbuffered = 0;
  }

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

  // Note: Py_DECREF(oMainModule) gave a Mem Violation in OE10.2B batch session. 
  //       We don't create it ourselves, so it's probably not meant to be decreased anyway. 
  //       Plus we only need one instance of the main module.

  Py_Finalize();

} // QxPy_FinalizeInterpreter


// EOF
