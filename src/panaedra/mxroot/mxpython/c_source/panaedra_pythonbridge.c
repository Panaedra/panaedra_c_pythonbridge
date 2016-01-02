#define PY_SSIZE_T_CLEAN

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

#define DATAIP_BUFFERED_UTF8 1
#define DATAIP_BUFFERED_BARRAY 3

#define DATAOP_BUFFERED_UTF8 1
#define DATAOP_UNBUFFERED_UTF8 2
#define DATAOP_BUFFERED_BARRAY 3
#define DATAOP_UNBUFFERED_BARRAY 4


#if defined(MS_WIN64) || defined(MS_WINDOWS)
#include <windows.h>
#include <stdio.h>
#include <wincon.h>
#include <winerror.h>
#include <conio.h>
#else
// AIX: See /usr/include/sys/stat.h , for mkfifo
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#endif

static void** pModules = 0;
static int iMaxModules = 0;
static PyObject* oMainModule = 0;
static PyObject* oGlobalDict = 0; 
static PyObject* oLastUnbuffered = 0; 
static char* pNullString = "";
static int iGilState = 0;
static PyThreadState *oSavedThreadState = 0; 

#if QXPYDEBUG
void initioretexample(void); /* Forward */
#endif

PyMODINIT_FUNC
  QxPy_InitializeInterpreter(char *cPyExePathIP, int iMaxModulesIP, char *cErrorOP, long long *iErrorLenOP)
{

  int i = 0;

  *iErrorLenOP = 0;

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
    *iErrorLenOP = strlen(cErrorOP);
  }
  else
  {
    TRY
    {

      char* argv[1];

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

      // Initialize thread support
      PyEval_InitThreads();

      /* Add static modules */
      #if QXPYDEBUG
      initioretexample();
      #endif

      argv[0] = cPyExePathIP;
      PySys_SetArgvEx(1, argv, 0);

      #if QXPYDEBUG
      fprintf(stdout, "PySys_SetArgvEx is now: \"%s\"\n", argv[0]);
      #endif

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
      *iErrorLenOP = strlen(cErrorOP);
    }
    ETRY;

  }

} // QxPy_InitializeInterpreter

#if false
void QxPyH_TransferPyErrorToStringOld(char *cErrorOP)
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

} // QxPyH_TransferPyErrorToStringOld
#endif

void QxPyH_TransferPyErrorToString(char *cErrorOP)
{
  // Based on: http://www.gossamer-threads.com/lists/python/python/150924 (but fixed some ref counter errors and other stuff)

  PyObject *pName, *pModule, *pDict, *pFunc; 
  PyObject *pArgs, *pValue; 
  PyObject *err = PyErr_Occurred(); 
  char *cErr;
  char tb_string[2048]; 
  if(err) 
  { 
    PyObject *temp, *exc_typ, *exc_val, *exc_tb; 

    PyErr_Fetch(&exc_typ,&exc_val,&exc_tb); 
    PyErr_NormalizeException(&exc_typ,&exc_val,&exc_tb); 

    pName = PyString_FromString("traceback"); 
    pModule = PyImport_Import(pName); 
    Py_DECREF(pName); 

    temp = PyObject_Str(exc_typ); 
    if (temp != NULL) 
    { 
      if (strlen(cErrorOP) > 0)
      {
        cErr = "\t"; 
        strncat(cErrorOP, cErr, strnlen(cErr,MAXERRORLEN - strlen(cErrorOP) - 1));
      }
      cErr = PyString_AsString(temp); 
      strncat(cErrorOP, cErr, strnlen(cErr,MAXERRORLEN - strlen(cErrorOP) - 1));
      Py_DECREF(temp); 
    } 
    temp = PyObject_Str(exc_val); 
    if (temp != NULL){ 
      cErr = "\t"; 
      strncat(cErrorOP, cErr, strnlen(cErr,MAXERRORLEN - strlen(cErrorOP) - 1));
      cErr = PyString_AsString(temp);
      strncat(cErrorOP, cErr, strnlen(cErr,MAXERRORLEN - strlen(cErrorOP) - 1));
      Py_DECREF(temp); 
    } 

    if (exc_tb != NULL && pModule != NULL ) 
    { 
      pDict = PyModule_GetDict(pModule); 
      if (pDict)
      {
        pFunc = PyDict_GetItemString(pDict, "format_tb"); 
        if (pFunc && PyCallable_Check(pFunc)) 
        { 
          pArgs = PyTuple_New(1); 
          PyTuple_SetItem(pArgs, 0, exc_tb); 
          pValue = PyObject_CallObject(pFunc, pArgs); 
          if (pValue != NULL) 
          { 
            int len = PyList_Size(pValue); 
            if (len > 0) { 
              PyObject *t,*tt; 
              int i; 
              char *buffer; 
              for (i = 0; i < len; 
                i++) { 
                  tt = 
                    PyList_GetItem(pValue,i); // Returns borrowed reference
                  t = 
                    Py_BuildValue("(O)",tt); 
                  if (t)
                  {
                    if (PyArg_ParseTuple(t,"s",&buffer)){ 
                      strcpy(tb_string,buffer); 
					  #ifndef _WIN32
					  // Note: On Windows, PyMem_Free crashes prowin32, at least in QxPy_RunCompiledPyCodeUnbuffered. 
					  //       We have to revise error handling anyway. A process restart is needed after a Python exception
					  //       through the bridge for the moment, even without this PyMem_Free patch. So for now we skip
					  //       the freeing of memory of this error string buffer. It will be cleaned up at the end of the process,
					  //       by the OS.
                      PyMem_Free(buffer);
                      #endif
                      cErr = "\t"; 
                      strncat(cErrorOP, cErr, strnlen(cErr,MAXERRORLEN - strlen(cErrorOP) - 1));
                      cErr = tb_string;
                      strncat(cErrorOP, cErr, strnlen(cErr,MAXERRORLEN - strlen(cErrorOP) - 1));
                    }
                    Py_DECREF(t); 
                  }
              } 
            } 
            Py_DECREF(pValue); 
          } 
          Py_DECREF(pArgs); 
        }
        if (pFunc) Py_DECREF(pFunc);
        // No Py_DECREF of pDict; borrowed reference
      }
    } 
    Py_DECREF(pModule); 

    #if false
    GUIDisplay("Error"+err_str); 
    PyErr_Restore(exc_typ, exc_val, exc_tb); 
    PyErr_Print(); 
    #endif

    return; 
  } 
} // QxPyH_TransferPyErrorToString

void GilAcquire()
{
  if (iGilState == 1)
  {
    iGilState = 0;
    // Get the GIL (PyEval_AcquireLock() is called) and restore the previous thread state.
    PyEval_RestoreThread(oSavedThreadState); 
  }
}

void GilRelease()
{
  if (iGilState == 0)
  {
    iGilState = 1;
    // Release the GIL (PyEval_ReleaseLock is called; other python threads can do their work) and save the state of this thread
    oSavedThreadState = PyEval_SaveThread();
    /* Main thread is now released. No Python API allowed beyond this point. */
  }
}

PyMODINIT_FUNC
  QxPy_SetCompiledPyCode(int iPyObjectIP, char *cPyIdIP, char *cPyCodeIP, char *cErrorOP, long long *iErrorLenOP)
{  
  void *oPyObject = 0;

  cErrorOP[0] = 0;
  cErrorOP[1] = 0; // For Utf-8 data
  cErrorOP[2] = 0;
  cErrorOP[3] = 0; // For Utf-16 data
  *iErrorLenOP = 0;

  GilAcquire();

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
    *iErrorLenOP = strlen(cErrorOP);
  }
  else
  {
    #if QXPYDEBUG
    fprintf(stdout, "Initialize py object pointer: \"%p\" \"%s\"\n", oPyObject, cPyCodeIP);
    #endif
    pModules[iPyObjectIP] = oPyObject;
  }

  GilRelease();

} // QxPy_SetCompiledPyCode


void
  RunCompiledPyCodeImplement(int iModuleIP, long long iDataLenIP, char *cDataIP, int iDataIpModeIP, int iDataOpModeIP, char **cDataOP, long long *iDataLenOP, long long iDataOpMaxLengthIP, char *cErrorOP)
{  
  cErrorOP[0] = 0;
  cErrorOP[1] = 0; // For Utf-8 data
  cErrorOP[2] = 0;
  cErrorOP[3] = 0; // For Utf-16 data

  if (iDataOpModeIP == DATAOP_UNBUFFERED_UTF8 || iDataOpModeIP == DATAOP_UNBUFFERED_BARRAY)
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

  GilAcquire();

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
    PyObject* pPyObjDataIP = 0;
    PyObject* pPyObjDataOP = 0;
    PyObject* pPyObjDataRet = 0;
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
    oLocalDict = PyDict_Copy(oGlobalDict);

    if (iDataIpModeIP == DATAIP_BUFFERED_BARRAY)
      pPyObjDataIP = PyByteArray_FromStringAndSize(cDataIP, (Py_ssize_t)iDataLenIP);
      // Note: a 'bytes' object, a python immutable type, would be preferable. 
      // 'PyBytes_FromStringAndSize' however, is (at least in python 2.7) simply a macro 
      // for 'PyString_FromStringAndSize'. See <bytesobject.h>.
      // Therefore we create a mutable 'bytearray' object, using 'PyByteArray_FromStringAndSize'
    else
      pPyObjDataIP = PyUnicode_FromStringAndSize(cDataIP, (Py_ssize_t)iDataLenIP); 

    pPyObjDataOP = PyUnicode_FromString(*cDataOP);

    #if QXPYDEBUG
    fprintf(stdout, "cDataIP: \"%s\"\n", cDataIP);
    fprintf(stdout, "pPyObjDataIP: \"%p\"\n", pPyObjDataIP);
    #endif

    PyDict_SetItemString(oLocalDict, "cDataIP", pPyObjDataIP);
    PyDict_SetItemString(oLocalDict, "cDataOP", pPyObjDataOP);

    // The dictionary holds the item now (reference count increase), so it won't be garbage collected. We can decrease the reference count by one.
    Py_DECREF(pPyObjDataIP);
    Py_DECREF(pPyObjDataOP);

    pRet = PyEval_EvalCode((PyCodeObject*)pModules[iModuleIP], oLocalDict /*global*/, oLocalDict /*local*/); // We pass the same dict as the globals() and the locals() reference, so code with imports and class definitions will execute similar to when running a file object.

    pPyObjDataRet = PyDict_GetItemString(oLocalDict, "cDataOP");
    
    #if QXPYDEBUG 
    fprintf(stdout, "pPyObjDataOP after eval: \"%p\"\n", pPyObjDataOP);
    fprintf(stdout, "pPyObjDataRet after getitem: \"%p\"\n", pPyObjDataRet);
    fprintf(stdout, "pPyObjDataRet->ob_refcnt after getitem: \"%zd\"\n", pPyObjDataRet->ob_refcnt);
    fprintf(stdout, "pRet object pointer: \"%p\"\n", ((void *)pRet));
    #endif

    if (pRet == 0)
    {
      char* cErr = "Runtime error in python code.\t";
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
      PyObject_Print(pPyObjDataRet, stdout, 0);
      fprintf(stdout, "\n");
      fprintf(stdout, "iPyDataLength: \"%zd\"\n", iPyDataLength);
      fprintf(stdout, "iRetData: \"%i\"\n", iRetData);
      fprintf(stdout, "cDataOP: \"%s\"\n", *cDataOP);
      fprintf(stdout, "pOutOP: \"%s\"\n", pOutOP);
      #endif

      if (iDataOpModeIP == DATAOP_BUFFERED_BARRAY || iDataOpModeIP == DATAOP_UNBUFFERED_BARRAY)
      {
        // If the API output parameter is requested as bytearray, treat it as such
        pOutOP = PyByteArray_AS_STRING(pPyObjDataRet);
        *iDataLenOP = PyByteArray_GET_SIZE(pPyObjDataRet);
        // Suppress (one line) possible loss of data warning.
        // Is safe here, because in a 32 bit process only 32 bit memory
        // pointers are passed to the .dll/.so. So even when stored in an int64, limit 
        // of int32 is not exceeded (otherwise it's garbage / faulty code anyway).
        #if defined(MS_WIN64) || defined(MS_WINDOWS)
        #pragma warning(disable: 4244)  
        #endif
        iPyDataLength = *iDataLenOP;
        #if defined(MS_WIN64) || defined(MS_WINDOWS)
        #pragma warning(default: 4244)  
        #endif
        iRetData = 0;
      }
      else
      {
        // If the API output parameter is not requested as bytearray, treat it as unicode (or nonunicode) string
        iRetData = PyString_AsStringAndSize(pPyObjDataRet, &pOutOP, &iPyDataLength); // Returns int
      }

      if (iRetData != 0)
      {
          char* cErr = "Error: cDataOP string could not be retrieved from python locals() dictionary.\t"; 
          strncat(cErrorOP, cErr, strnlen(cErr,MAXERRORLEN - strlen(cErrorOP) - 1));
          QxPyH_TransferPyErrorToString(cErrorOP);
      }
      else
      {
        if (iDataOpModeIP != DATAOP_UNBUFFERED_UTF8 && iDataOpModeIP != DATAOP_UNBUFFERED_BARRAY)
        {
          // Transfer the string from the Python world to the DLL / shared object world.
          // Memory is allocated by the caller of the DLL / shared object.
          // If we want to switch the owner of the data we need this deep copy. 
          // Otherwise, use the unbuffered alternative, which returns a direct pointer to 
          // the python object, which will be valid until the next api call.
          strncpy(*cDataOP, pOutOP, (size_t)iDataOpMaxLengthIP); // Note on strncpy: this also works on null characters inside pOutOP, exactly what we want for DATAOP_BUFFERED_BARRAY mode.

          if (iDataOpMaxLengthIP < iPyDataLength)
          {
            char cErr[100]; 
            sprintf(cErr, "Warning: Output parameter truncated. Python string length \"%zd\" is larger than user-supplied maximum length \"%lld\".\t", iPyDataLength, iDataOpMaxLengthIP);
            strncat(cErrorOP, cErr, strnlen(cErr,MAXERRORLEN - strlen(cErrorOP) - 1));
          }
        }
        else
        {
          // Unbuffered. Increase reference counter by 1, hold python object until next call.
          Py_IncRef(pPyObjDataRet);
          oLastUnbuffered = pPyObjDataRet;
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
    // Note: pPyObjDataRet is a borrowed reference, don't decrease.
    fprintf(stdout, "pPyObjDataRet->ob_refcnt at end: \"%zd\"\n", pPyObjDataRet->ob_refcnt);
    #endif

  }

  GilRelease();
  
} // RunCompiledPyCodeImplement


PyMODINIT_FUNC
  QxPy_RunCompiledPyCode(int iModuleIP, long long iDataLenIP, char *cDataIP, char *cDataOP, long long iDataOpMaxLengthIP, long long *iDataLenOP, char *cErrorOP, long long *iErrorLenOP)
{  
  long long iDataLenDummy = 0;
  RunCompiledPyCodeImplement(iModuleIP, iDataLenIP, cDataIP, DATAIP_BUFFERED_UTF8, DATAOP_BUFFERED_UTF8, &cDataOP, &iDataLenDummy, iDataOpMaxLengthIP, cErrorOP);
  *iDataLenOP = strlen(cDataOP);
  *iErrorLenOP = strlen(cErrorOP);
} // QxPy_RunCompiledPyCode

PyMODINIT_FUNC
  QxPy_RunCompiledPyCodeB(int iModuleIP, long long iDataLenIP, char *cDataIP, char *cDataOP, long long iDataOpMaxLengthIP, long long *iDataLenOP, char *cErrorOP, long long *iErrorLenOP)
{  
  // See QxPy_RunCompiledPyCode. output=byte-array alternative.
  long long iDataLenDummy = 0;
  RunCompiledPyCodeImplement(iModuleIP, iDataLenIP, cDataIP, DATAIP_BUFFERED_UTF8, DATAOP_BUFFERED_BARRAY, &cDataOP, &iDataLenDummy, iDataOpMaxLengthIP, cErrorOP);
  *iDataLenOP = strlen(cDataOP);
  *iErrorLenOP = strlen(cErrorOP);
} // QxPy_RunCompiledPyCodeB


PyMODINIT_FUNC
  QxPy_RunCompiledPyCodeBB(int iModuleIP, long long iDataLenIP, char *cDataIP, char *cDataOP, long long iDataOpMaxLengthIP, long long *iDataLenOP, char *cErrorOP, long long *iErrorLenOP)
{  
  // See QxPy_RunCompiledPyCode. input&output=byte-array alternative.
  RunCompiledPyCodeImplement(iModuleIP, iDataLenIP, cDataIP, DATAIP_BUFFERED_BARRAY, DATAOP_BUFFERED_BARRAY, &cDataOP, iDataLenOP, iDataOpMaxLengthIP, cErrorOP);
  *iErrorLenOP = strlen(cErrorOP);
} // QxPy_RunCompiledPyCodeBB

// Note: The only way to return an uninitialized memptr in OpenEdge ABL is by a return parameter.
PyMODINIT_PCHAR
  QxPy_RunCompiledPyCodeUnbuffered(int iModuleIP, long long iDataLenIP, char *cDataIP, long long *iDataLenOP, char *cErrorOP, long long *iErrorLenOP)
{  
  // Essentially the same as QxPy_RunCompiledPyCode, but instead of a deep copy to cDataOP, a pointer to 
  // the char buffer of the (kept-alive) python unicode object is returned.
  // This buffer should not be modified in any way.
  // The python object (and the underlying string buffer) are cleaned up at the very next call, or at system exit.
  char *cDataOP;
  long long iDataLenDummy = 0;
  RunCompiledPyCodeImplement(iModuleIP, iDataLenIP, cDataIP, DATAIP_BUFFERED_UTF8, DATAOP_UNBUFFERED_UTF8, &cDataOP, &iDataLenDummy, 0, cErrorOP);
  *iDataLenOP = strlen(cDataOP);
  *iErrorLenOP = strlen(cErrorOP);
  return cDataOP;
} // QxPy_RunCompiledPyCodeUnbuffered

PyMODINIT_PCHAR
  QxPy_RunCompiledPyCodeUnbufferedB(int iModuleIP, long long iDataLenIP, char *cDataIP, long long *iDataLenOP, char *cErrorOP, long long *iErrorLenOP)
{  
  // See: QxPy_RunCompiledPyCodeUnbuffered. output=byte-array alternative.
  char *cDataOP;
  long long iDataLenDummy = 0;
  RunCompiledPyCodeImplement(iModuleIP, iDataLenIP, cDataIP, DATAIP_BUFFERED_BARRAY, DATAOP_UNBUFFERED_UTF8, &cDataOP, &iDataLenDummy, 0, cErrorOP);
  *iDataLenOP = strlen(cDataOP);
  *iErrorLenOP = strlen(cErrorOP);
  return cDataOP;
} // QxPy_RunCompiledPyCodeUnbufferedB

PyMODINIT_PCHAR
  QxPy_RunCompiledPyCodeUnbufferedBB(int iModuleIP, long long iDataLenIP, char *cDataIP, long long *iDataLenOP, char *cErrorOP, long long *iErrorLenOP)
{  
  // See: QxPy_RunCompiledPyCodeUnbuffered. input&output=byte-array alternative.
  char *cDataOP;
  RunCompiledPyCodeImplement(iModuleIP, iDataLenIP, cDataIP, DATAIP_BUFFERED_BARRAY, DATAOP_UNBUFFERED_BARRAY, &cDataOP, iDataLenOP, 0, cErrorOP);
  *iErrorLenOP = strlen(cErrorOP);
  return cDataOP;
} // QxPy_RunCompiledPyCodeUnbufferedBB

PyMODINIT_FUNC
  QxPy_FreeCompiledPyCode(int iPyObjectIP)
{  
  #if QXPYDEBUG
  fprintf(stdout, "Del py object number: \"%i\"\n", iPyObjectIP);
  #endif

  GilAcquire();

  if (iPyObjectIP <= iMaxModules && pModules[iPyObjectIP] != 0) 
  {
    Py_DECREF(pModules[iPyObjectIP]);
    pModules[iPyObjectIP] = 0;
  }

  GilRelease();

} // QxPy_FreeCompiledPyCode


PyMODINIT_FUNC 
  QxPy_FinalizeInterpreter()
{

  int i = 0;
  
  // Since we initialized thread support, we have to acquire the GIL before 
  // calling Py_Finalize(), and before we call any Python API whatsoever.
  GilAcquire();

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
  //       We don't create it ourselves, so it's probably not meant to be decreased 
  //       anyway (meaning: borrowed reference). 
  //       Plus we only need one instance of the main module.

  // Shut down the interpreter
  Py_Finalize();

} // QxPy_FinalizeInterpreter

PyMODINIT_FUNC
  QxPy_MkFifo(char *cPathIP, int *iErrorOP)
{ 

#  if defined(MS_WIN64) || defined(MS_WINDOWS)
  // wouldhave
  // This codeblock is not tested/finished yet, just a headstart 
  // for when we will implement this on windows
  // See: K:\progress\DLC.11.0A.DOC\OpenEdge_Doc\openedge\dvpin\dvpin.pdf
  HANDLE hPipe;
  *iErrorOP = 0;
  hPipe = CreateNamedPipe(
    "\\\\.\\pipe\\custpipe",
    PIPE_ACCESS_DUPLEX,
    PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
    1,
    0,
    0,
    NMPWAIT_USE_DEFAULT_WAIT,
    NULL);
  if (hPipe == INVALID_HANDLE_VALUE)
  {
    *iErrorOP = GetLastError();
    printf("Error creating pipe, %ld\n", *iErrorOP);
  }
#  else
  *iErrorOP = 0;
  *iErrorOP = mkfifo(cPathIP, S_IFIFO | 0666 );
#  endif

} // QxPy_MkFifo

PyMODINIT_FUNC
  QxPy_RmFifo(char *cPathIP, int *iErrorOP)
{ 
  *iErrorOP = 0;

# if defined(MS_WIN64) || defined(MS_WINDOWS)
  // wouldhave (see above); CloseHandle(hPipe);
# else
  *iErrorOP = remove(cPathIP);
# endif

} // QxPy_RmFifo

PyMODINIT_FUNC
  QxPy_UnlinkFifo(char *cPathIP, int *iErrorOP) 
{ 
  *iErrorOP = 0;

# if defined(MS_WIN64) || defined(MS_WINDOWS)
# else
  // Like QxPy_RmFifo, but unlinks the fifo from the parent dir even when in use. 
  // Caution, can cause FAT strangeness when used incorrectly. Don't ever use 
  // on non-empty dirs.
  *iErrorOP = unlink(cPathIP);
# endif

} // QxPy_UnlinkFifo

#if QXPYDEBUG // ioretexample: an example of a python module injected by the bridge. The needed 'iocodec' module is later developed in Cython, different project.

static char cDataOut[100];
static Py_ssize_t iDataOutLen = 0;
static PyObject *oRet = 0;

static PyObject *
ioretexample_exp2pack(PyObject *self, PyObject* args)
{
  const char *cDataIn = 0;
  Py_ssize_t iDataInLen = 0;
  //char *cDataOut = 0;

  //cDataOut = (char*)malloc(sizeof(char) * iDataInLen);

  if (!PyArg_ParseTuple(args, "s#", &cDataIn, &iDataInLen))
          return NULL;

  iDataOutLen = 11;
  strncpy(cDataOut,cDataIn,11);
  
  //fprintf(stdout, "cDataOut is now: \"%s\"\n", cDataOut);

  oRet = Py_BuildValue("s#", cDataOut, iDataOutLen);

  //free(cDataOut);
  //cDataOut = 0;
  //return Py_BuildValue("s#", "hello", 4);
  return oRet;
}


static PyMethodDef ioretexample_methods[] = {
    {"exp2pack", ioretexample_exp2pack, METH_VARARGS,
     "Encode example."},
    {NULL, NULL} /* sentinel */
};


void
initioretexample(void)
{
    PyImport_AddModule("ioretexample");
    Py_InitModule3("ioretexample", ioretexample_methods,"Example of a module injected by the bridge.");
}

#endif // ioretexample

// EOF
