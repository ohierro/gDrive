/********************************************************************
*   PROYECTO:       Internet Stfic Server/Client (ISSC) (1996-1999) *
*   SUBPROYECTO:    Rutinas de tratamiento de ficheros formato STFIC*
*                                                                   *
*   Autor:Pablo Jose Royo                                           * 
*   Fecha:Noviembre 1997                                            *
*   COPYRIGHT: INTERCOMPUTER                                        *	
*********************************************************************/

#ifndef __ICTRACE_HPP__
#define __ICTRACE_HPP__ 1

#include <stdio.h>   
#include <errno.h>
/*
#include <syslog.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/times.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/resource.h>
*/
#include <stdarg.h>


#ifdef __UNIX__
#define PATH_DEL '/'

#ifdef PURIFY

#ifdef __REENTRANT
#undef __REENTRANT
#endif

#endif


#ifdef __REENTRANT
#ifdef __UNIX__
#include "stf_threads.hpp"
#endif
#endif

#else
#define PATH_DEL '\\'
#endif

#define TRACE_ONG(level,mode,header,TraceID) IcTrace::TraceInit((level),(mode),(header),(TraceID))
#define TRACE_ON(level,mode,header,TraceID,acumula) IcTrace::TraceInit((level),(mode),(header),(TraceID),(acumula))
/*,(Mail))*/


#define TRACE_OFF() IcTrace::~IcTrace()
#define TRACE_CLOSE() IcTrace::Close()
#define TRACE(level,mensaje) IcTrace::StfTrace((level),(mensaje))
#define TRACE2(level,mensaje,mensaje1) IcTrace::StfTrace((level),(mensaje),(mensaje1))

#ifdef DANICTRACE
#	ifndef DEBUG
#		define QUITADEBUG
#		define DEBUG
#	endif
#endif

#ifndef _NO_TRACE

#if ((defined(WINDOWS) || defined(WIN32)) && defined(DEBUG)) || defined(__UNIX__)  || defined(REGIS)

#ifndef TRACE_FUNCTION

#define VTRACE IcTrace::VStfTrace
#define OUT_TRACE IcTrace::PrintOut
#else

#ifdef WIN32
#pragma message("VTRACE ES UNA FUNCION: gpfTrace debe estar definido")
#endif

class IcTrace;


extern "C" { extern void (*gpfTrace1)(int,char*,...); }

#define VTRACE if(gpfTrace1) gpfTrace1
#define OUT_TRACE if(gpfTrace1) gpfTrace1

#endif //TRACE_FUNCTION

#ifdef SunOS
#define RTRACE IcTrace::Usage
#endif
#else
#define VTRACE    //
#define OUT_TRACE //

#endif

#else

#define VTRACE    //
#define OUT_TRACE //
#endif

#ifdef DANICTRACE
#undef VTRACE
#define VTRACE IcTrace::VStfTrace
#endif


#if ((defined(WINDOWS) || defined(WIN32)) && defined(DEBUG)) || defined(__UNIX__) || defined(REGIS)
#define VTERROR IcTrace::VStfError 
#else
#define VTERROR //
#endif 
 
#ifdef DANICTRACE
#undef VTERROR
#define VTERROR IcTrace::VStfError
#endif

/*#define SEND_MAIL(from,dest) IcTrace::Send_Mail((from),(dest))*/

#define MODE_SYSLOG 1
#define MODE_STDOUT 2
#define MODE_FILE   3

#define HDR_NONE 0
#define HDR_TIME 1
#define HDR_PID  2
#define HDR_STFIC 3


#define HDR_RESOURCES 4
#define PUERTO_MAIL 25

#include "X509_name_isolatin.hpp"

/*!
	Clase auxiliar que se utilizara para tracear. 
*/

enum TRACE_FLAG {FUNCTION_NAME = 0x1,FILE_NAME = 0x2,LINE_NUMBER = 0x4};

class TraceObject
{
	public:
		TraceObject(int Flags) {flags = Flags;line = 0;funcion_name = NULL;file = NULL;};
		~TraceObject(){};
				
	public:
		TraceObject *SetLine(const char *File,int Line) {file = File;line = Line;return this;};
		void  OTrace(int iLevel,char *szFormat,...);
		
	public:
		int flags;
		const char *funcion_name;			/*<!  Nombre de la funcion. >*/
		const char *file;					/*<!  Nombre del fichero donde se tracea. >*/
		int line;				        	/*<!  Numero de linea. >*/
}; 

#define T_FUNCTION(x,y) TraceObject object(y);object.funcion_name=(x) 
#ifndef WIN32 
#define T_F() TraceObject object(FUNCTION_NAME);object.funcion_name = __func__      
#else
#define T_F(x) TraceObject object(FUNCTION_NAME);object.funcion_name = (x)
#endif
#define FTRACE object.SetLine( __FILE__ , __LINE__ );object.OTrace
 

//void VTRACE(int iLevel,char *szFormat,...);
//void VTERROR(int iLevel,char *szFormat,...);

/*!
	Clase de traceado de STFIC.
	Es una clase estatica usada profusamente en todo
	el codigo del sistema.
	
	Usa un descriptor de fichero para poder bloquearlo
	
*/	
class IcTrace
{
	public:
	
	IcTrace(){m_InitComm=0;};
	~IcTrace();
		
 	static int m_fdTrace;	/*!< Descriptor de fichero usado para bloquearlo*/
	static FILE *m_FILETrace; /*!< Descriptor FILE del fichero usado por vfprinf */
 	static int m_iTraceLevel;/*!< Nivel de traceado 0-9 */
 	static long m_numLinea; /*!< Numero aproximado de lineas del log. */
	static int  m_InitComm;
	static char *m_ptrAppName;
	/*! 
		Modo de traceado
			MODE_FILE -> Traza a fichero
			MODE_STDOUT -> Traza a la consola estandard
			MODE_SYSLOG -> Traza al syslog
	*/
	static int m_iTraceMode;
 	static int m_iHeader;	/*! Tipo de cabecera.PID generalmente*/
 	static char *m_pTraceID; /*! Identificador de fichero de traza */


#ifdef __REENTRANT
#ifdef __UNIX__
	static StfMutex *m_mutex; /*! Mutex utilizado para el bloqueo con threads*/
#endif	
#endif 	
	/*static char *m_pAddrMail;*/
 	
 	static int TraceInit(int iLevel,int iTraceMode,int iHeader,char *pTraceID = (char *)NULL,int acumula=0);
 	/*char *pAddrMail = NULL);*/
	
	static void Close(void);
	// TRACEs clasico y con autoformateo
	//static void Trace(int iLevel,char *szFormat,...);
	static void StfTrace(int iLevel,char *szMessage,char *szMessage1 = (char *) NULL);
	static void StfWTrace(int iBlock,int iLevel,char *szFormat,...);
	static void StfWLTrace(int iBlock,int iLevel,const char *szFormat,va_list va);
	static void VStfTrace(int iLevel,char *szFormat,...);
    	static void VStfError(int iLevel,char *szFormat,...);
	static void PrintOut(int i,char *szFormat,...);
	static void setAppName(char *name);
	static void unsetAppName();
	static char* getAppName();
	
#ifdef SunOS
	static void Usage(int i, char *szWhere=NULL);
#endif

#if 0	
	static int Send_Mail(char *from,char *dest);
	static int enviar_mail(int sock,char *from,char *dest);
	static int recibe(int sfd,char *buf);
#endif
};

#ifdef DANICTRACE
#	ifdef QUITADEBUG
#		undef DEBUG
#	endif
#endif





#endif
/*
class TraceFunction
{
	public:
		TraceFunction(void (*pf)(int,char*,...) = NULL)
		{
			m_pfTrace = pf;
		}

		void SetTraceFunction(void (*pf)(int,char*,...))
		{
			m_pfTrace = pf;
		}
	
		void (*GetTraceFunction(void (*pf)(int,char*,...)) )(void )
		{
			m_pfTrace = pf;
		}

		void Trace(int,char *format,...)
		{
			va_list arguments;
			va_start(arguments,format);

			m_pfTrace();
		}

	protected:
		void (*m_pfTrace)(int,char *,...);
};

static TraceFunction *gpfTrace = NULL;
*/