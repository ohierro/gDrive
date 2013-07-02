/********************************************************************
*   PROYECTO:       Internet Stfic Server/Client (ISSC) (1996-1999) *
*   SUBPROYECTO:    Rutinas de tratamiento de ficheros formato STFIC*
*                                                                   *
*   Autor:Pablo Jose Royo                                           *
*   Fecha:Mayo 1997                                            *
*   COPYRIGHT: INTERCOMPUTER                                        *
*********************************************************************/

#include <time.h>
#include <sys/types.h>


#if defined(WIN32) || defined(WINDOWS)
#include <windows.h>
#include <io.h>
#include <process.h>
#else
#include <unistd.h>
#include <syslog.h>
#ifdef SunOS
#include <sys/resource.h>
#endif
#endif



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>


#ifdef WIN32
	#include <winsock.h>
	#include <time.h>
	#include <io.h>
#else
	#include "trace_resources.hpp"
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netdb.h>
#endif



/*
#include <syslog.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/times.h>
*/

#include <errno.h>

/*
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/resource.h>
*/

#include "ictrace.hpp"

#ifdef DANICTRACE
#	ifndef DEBUG
#		define QUITADEBUG
#		define DEBUG
#	endif
#endif

//extern long tamanio(char *fn);

#ifdef VLOG_INCLUDE
#include <LogProxy.h>
#endif

#define SIZE_TEXT 8192

char *tiempo_actual(char *cad);

int IcTrace::m_fdTrace = -1;
int IcTrace::m_iTraceLevel = 1;
int IcTrace::m_iTraceMode = MODE_STDOUT;
int IcTrace::m_iHeader = HDR_NONE;
char *IcTrace::m_pTraceID = NULL;
FILE *IcTrace::m_FILETrace = NULL;
long IcTrace::m_numLinea = 0L;
int IcTrace::m_InitComm=0;
char* IcTrace::m_ptrAppName=NULL;

#ifdef __UNIX__
static TraceResources *g_resources = NULL;
#endif


#ifdef __REENTRANT
#ifdef __UNIX__
StfMutex  *IcTrace::m_mutex = NULL; /*! Mutex utilizado para el bloqueo con threads*/
#endif	
#endif 	



/*char *IcTrace::m_pAddrMail = (char *) strdup("127.0.0.1");*/

/*!
	Funcion de apertura del sistema de traceado.
	Esta funcion se llama dentro de la macro TRACE_ON()
	
	\param iLevel 	Nivel de traza.Nivel cero nada,los demas progresivamente
	\param iTraceMode Modo de traceado.
	\param iHeader 	Tipo de comienzo de la linea de traza
	\param pTraceID	Nombre del fichero de LOG, o del recurso que sea
	\param acumula Si no es cero, no se machaca el log al abrirlo,y se añade en el 
*/
int IcTrace::TraceInit(int iLevel,int iTraceMode,int iHeader,
			char *pTraceID,int acumula)           /*,char *pAddrMail)*/
{
	struct stat buf;
	int sock=-1;

#ifdef __UNIX__
	if( iHeader == HDR_RESOURCES )
		g_resources = CREATE_RESOURCES( RUSAGE_SELF );
#endif

/*Cambiamos de sitio este bloque para que cree el mutex si es
necesario aunque m_fdTrace sea distinto de -1, para que desde
fuera puedan pasarnos el descriptor del fichero de LOG.  Esto 
se utiliza en el HERMES_UNIX.*/
#ifdef __REENTRANT
#ifdef __UNIX__
	if(!m_mutex)
		m_mutex = CreateMutex(); /*! Mutex utilizado para el bloqueo con threads*/
	if(!m_mutex)
		return -1; 
#endif	
#endif 
	
	if(m_fdTrace != -1)
	{
		/* Se esta llamando a TRACE_ON() y ya se habia hecho */
		VStfTrace(5,"TRACE_ON() llamado previamente");
		return 1;
	}

       	memset((void *)&buf,0,sizeof(buf));

	if(m_pTraceID)
		free(m_pTraceID);

	m_pTraceID = (char *) malloc( (strlen(pTraceID) +1)*sizeof(char));
	
	if(!m_pTraceID)
		return -1;

	strcpy(m_pTraceID,pTraceID);
 
 /*
        if( stat(pTraceID,&buf) >= 0 )
        {
		time_t l_time;
		char szNewPathLog[450];
		char *ptr = NULL,date[50];
		
		l_time = time( (time_t*) NULL );
		
#if defined WIN32
		struct tm *tim;
		tim = localtime(&l_time);
		strftime(date,sizeof(date),"%d_%m_%Y_%H%M%S.log",tim);
#else
		struct tm tim;
		localtime_r(&l_time,&tim);
		strftime(date,sizeof(date),"%d_%m_%Y_%H%M%S.log",&tim);
#endif
		strcpy( szNewPathLog , pTraceID );
		
		if( ptr = strrchr(szNewPathLog,'.') )
			*ptr = 0;
		
		strcat(szNewPathLog,"_");
		strcat(szNewPathLog,date);
		
		rename(pTraceID,szNewPathLog);			
	}
*/	
	m_fdTrace = -1;
#if ((defined(WINDOWS) || defined(WIN32)) && defined(DEBUG)) || defined(__UNIX__) || defined(REGIS)
	
	m_iHeader = iHeader;
	
	if(iTraceMode == MODE_STDOUT)
	{
		/*if(isatty(1))
		  comprueba que el desc. 1(salida) es una terminal.(¿P´a que?)*/
		  m_iTraceLevel = iLevel;
		  m_iTraceMode = MODE_STDOUT;
	}
	else
	{
		char *aux=NULL;		
		m_numLinea = 0L;


		/*La segunda comparacion es para descartar una unidad de Widows*/
		if( ( aux = strchr(pTraceID,':') ) && pTraceID[1] != ':' )
		{	
			/*Enviamos el log a un servicio.*/
			char *host=NULL;
			int puerto=0;
			struct hostent *regishost=NULL,hostentstruct;
			struct sockaddr_in addr;
			int ha_errno=0;
			unsigned long on = 0;

			if( !( host = (char *) malloc( strlen(pTraceID) + 1 ) ) )
			{
				fprintf(stderr,"Error(%s) malloc en TraceOn\n",strerror(errno)); 
				return -1;
			}
			strcpy(host,pTraceID);
			
			aux = strchr(host,':');
			*aux = 0;
			puerto = atoi(aux + 1);
			
			if( puerto < 0  || puerto > 65536 )
			{
				fprintf(stderr,"Error en puerto %d.\n",puerto); 
				free(host);
				return -1;
			}
#ifdef WIN32
			int sock_opt = SO_SYNCHRONOUS_NONALERT;	

			WSADATA info;
			if ( (WSAStartup(MAKEWORD(1,1),&info)) !=0) 
			{
				fprintf(stderr,"Error(%s) abriendo libreria winsocket:\n",strerror(errno)); 
				free(host);
				return -1;
			}

			/*Permitimos que los socket se comporten con descriptores de ficheros.*/	
			if( setsockopt(INVALID_SOCKET,SOL_SOCKET, SO_OPENTYPE, (char *)&sock_opt,
						sizeof(sock_opt))  != NO_ERROR)
			{
				fprintf(stderr,"Error en setsockopt.");
				free(host);
				return -1;
			}
#endif			
			m_InitComm = 1;

#if defined __UNIX__ && defined SunOS	
			char buf[8192];
			regishost = gethostbyname_r(host,&hostentstruct,buf,8192,&ha_errno);
#else		 
			if(( regishost = gethostbyname( (const char *) host )))
				memcpy(&hostentstruct,regishost,sizeof(struct hostent));		
			ha_errno = errno;
#endif		
			if(!regishost)
			{
				fprintf(stderr,"E Connect():connect() Error Localizando %s",host);
				free(host);
				return -1;
			}

			addr.sin_family = AF_INET;
   			addr.sin_port = htons((unsigned short) puerto);
			addr.sin_addr =  *( (struct in_addr *) hostentstruct.h_addr );

			memset(addr.sin_zero,0,sizeof(addr.sin_zero));

			if( (sock = socket(AF_INET,SOCK_DGRAM,0)) < 0 )
			{
				fprintf(stderr,"Error en apertura socket(%d): %s.\n",errno,strerror(errno));
				free(host);
				return -1;
			}
				
#ifdef WIN32	
			/*Comvertimos sock que es un handler de Windows a un descriptor de la LIBC.*/
			m_fdTrace = _open_osfhandle(sock,_O_RDWR|_O_BINARY);
			if(m_fdTrace < 0)
			{
				fprintf(stderr,"Error _open_osfhandle.\n");
				free(host);
				return -1;
			}
			ioctlsocket(m_fdTrace,FIONBIO,&on); /*socket no bloqueante */
#else
			fcntl(m_fdTrace,F_SETFL,fcntl(m_fdTrace,F_GETFL,0) | O_NONBLOCK); 
			m_fdTrace = sock;
#endif
			if( connect(sock,( struct sockaddr *)&addr,sizeof(struct sockaddr_in)) < 0 )
			{
				fprintf(stderr,"Error en connect(%d): %s.\n",errno,strerror(errno));
				free(host);
				return -1;
			}
			
			free(host);
		}
		else
		{
			if(acumula)
				m_fdTrace = open((char *)pTraceID,O_RDWR|O_CREAT|O_APPEND,0640);
			else
				m_fdTrace = open((char *)pTraceID,O_RDWR|O_CREAT|O_TRUNC,0640);		
		}
		
		if ( m_fdTrace < 0 )
    	{
        		fprintf(stderr,"Error en apertura Trace(%d): %s.",errno,strerror(errno));
        		return -1;
    	}
	
		/*Solo hay que liberar m_FILETrace con fclose,
		 no m_fdTrace, lo	hace este por debajo*/
		if(!(m_FILETrace = fdopen(m_fdTrace,"a")))
		{
			fprintf(stderr,"Error en apertura Trace %s.",strerror(errno));
        		return(-1);
		}

		m_iTraceLevel = iLevel;
		m_iTraceMode = MODE_FILE;
	}
	return 1;
#else
    return 1;
#endif
}

/*! Destructor */
IcTrace::~IcTrace()
{
	if(m_pTraceID)
		free(m_pTraceID);

	m_pTraceID = NULL;

#ifdef __REENTRANT
#ifdef __UNIX__
	if(m_mutex)
		delete m_mutex; 
		
#endif	
#endif 	

#if ((defined(WINDOWS) || defined(WIN32)) && defined(DEBUG)) || defined(__UNIX__) || defined(REGIS)
	switch(m_iTraceMode)
	{
#ifdef __UNIX__
		case MODE_SYSLOG:
			closelog();
			break;
#endif
		case MODE_STDOUT:
			close(1);
			break;
		case MODE_FILE:
			Close();
			break;
		default:break;
	}
	m_iTraceLevel = 0;
	m_iTraceMode = 0;
	m_fdTrace = -1;
	m_FILETrace=NULL;
	/*free(m_pAddrMail);*/
	//free(m_pTraceID);
#endif
}

/*! Cierre del descriptor */
void IcTrace::Close()
{

#ifdef __REENTRANT
#ifdef __UNIX__
	if(m_mutex)
		delete m_mutex; 
	m_mutex=NULL;	
#endif	
#endif 	


#if ((defined(WINDOWS) || defined(WIN32)) && defined(DEBUG)) || defined(__UNIX__) || defined (REGIS)
	
	/*Se libera solo uno de ellos*/
	if(m_FILETrace)
	{ 		
		fclose(m_FILETrace);
#ifdef 	WIN32
/*	
	Comentamos esto porque si no peta y no tenemos tiempo de mirar porque, cuando 
	se haga un exis la librerria se encarga de todo.
	if(m_InitComm)
		WSACleanup();
*/
	m_InitComm=0;
#endif		
		goto err;
	}
	else if(m_fdTrace >= 0)	
		close(m_fdTrace);
err:
#endif

	m_fdTrace = -1;
	m_FILETrace = NULL;
	return;
}



/*!
	Funcion a la que se pasa un va_list y se tracea lo que alli va.
		\param iBlock.    Le indica si debe o no bloquear el fichero.
		\param iLevel.	  Nivel de traza.
		\param szFormat.  Formato a tracear.
		\param va.		  Lista con los argumentos que se usaran para tracear.	
*/


void IcTrace::StfWLTrace(int iBlock,int iLevel,const char *szFormat,va_list va)
{
	if(!iLevel)
		return;

	if(!szFormat || !*szFormat || iLevel < 0 )
		return;
	
#ifdef __UNIX__
	#ifdef  __REENTRANT
	LockGuard guard(m_mutex);
	#endif
	struct flock lock;	
#endif	

	if( iLevel > IcTrace:: m_iTraceLevel )
		return;	
		
#if ((defined(WINDOWS) || defined(WIN32)) && defined(DEBUG)) || defined(__UNIX__) || defined (REGIS)	
	switch(IcTrace::m_iTraceMode)
	{
		case MODE_STDOUT:vprintf(szFormat,va);break;
		case MODE_FILE:
		{

#ifdef __UNIX__
			if(iBlock)
			{
#ifdef __REENTRANT
			/* Esto lo hacemos para evitar problemas de bloqueo en UNIX cuando
			lo hacen threads en UNIX.*/
				guard.Lock();
#endif	
				lock.l_type = F_WRLCK;
				lock.l_whence = lock.l_start = lock.l_len = 0;

				if(fcntl(IcTrace::m_fdTrace,F_SETLKW,&lock) == -1)
      				return;
			}	
#endif
			if(va)
				vfprintf(m_FILETrace,szFormat,va);
			else
				fprintf(m_FILETrace,"%s",szFormat);
			
			fflush(m_FILETrace);

#ifdef __UNIX__
			if(iBlock)
			{
				lock.l_type = F_UNLCK;
				lock.l_whence = lock.l_start = lock.l_len = 0;
				if(fcntl(IcTrace::m_fdTrace,F_SETLKW,&lock) == -1)
        			return;
#ifdef __REENTRANT
			/* Esto lo hacemos para evitar problemas de bloqueo en UNIX cuando
			lo hacen threads en UNIX.*/
			guard.UnLock();
#endif	
			}
#endif
			break;
		}
		default:break;
	}
#endif

	return;
}

/*!
	Funcion a utilizar cuando se quiere tracear a stdout
	sin mas miramientos.
*/
void IcTrace::PrintOut(int i,char *szFormat,...)
{
	char text[1024];
	
	va_list arguments;
	va_start(arguments,szFormat);
	
	vsprintf(text,szFormat,arguments);
	fprintf(stdout,"%s\n",text);
	va_end(arguments);
	
	return;
}

/*!
	
	Funcion que unicamente escribe lo que se le pasa, no escribe
	ninguna cabecera.
	
		\param iBlock. 		Le indica si debe bloquear el fichero.
		\param iLevel. 		Nivel de Traza.
		\param szFormat. 	Formato a tracear.
		
*/

void IcTrace::StfWTrace(int iBlock,int iLevel,char *szFormat,...)
{
	if(!iLevel)
		return;

	if( iLevel > IcTrace:: m_iTraceLevel )
		return;	
		
	/*char text[SIZE_TEXT]*/;
	va_list arguments;
	va_start(arguments,szFormat);
	

#ifdef OSF1
 		if(va_arg(arguments, char *))
		{
			va_start(arguments,szFormat);
#else	
		if(arguments)
		{
#endif	
			IcTrace::StfWLTrace(iBlock,iLevel,szFormat,arguments);
		}
		
	return;
}



/*!
	TRACE clasico, se le pasan cadenas ya formateadas 
*/
void IcTrace::StfTrace(int iLevel,char *szMessage,char *szMessage1)
{
	struct stat buf;
	int iMaxLogLength = 0;
	
	if(!iLevel)
		return;
	
#ifdef __UNIX__
	if( g_resources )
		g_resources->TRACE_RESOURCES( 0 , 10 );
#endif
			
		
	++m_numLinea;
	memset((void *)&buf,0,sizeof(buf));
	
	if( fstat(m_fdTrace,&buf) >= 0 && buf.st_size > 2000000000  )
	{
		iMaxLogLength = 1;
		szMessage = "Alcanzado el maximo tamaño peritido de 2 GBytes";
		szMessage1 = "\n";
	}	
		
#ifdef VLOG_INCLUDE
	if(iLevel <= IcTrace::m_iTraceLevel)
	{
		TRACE_R(DEFAULT____,"IcTrace",TRACER_FORMAT("%d -- %s\n",GetCurrentThreadId(),szMessage));
	}
	return;
#else

	if( m_fdTrace == -1 && m_iTraceMode == MODE_FILE)
		return;
#if defined(UNIX) || defined(__UNIX__)	

#ifdef __REENTRANT
#ifdef __UNIX__
	LockGuard guard(m_mutex);
#endif
#endif
	struct flock lock;	
#endif	

#if ((defined(WINDOWS) || defined(WIN32)) && defined(DEBUG)) || defined(__UNIX__) || defined (REGIS)
	char aux_time[30],aux_id[200],s[SIZE_TEXT],pid[10];
	switch(m_iHeader)
	{
		case HDR_TIME:
			strcpy(aux_id,(char *)tiempo_actual(aux_time));
			break;
		case HDR_PID: case HDR_RESOURCES:
#ifdef WIN32
			sprintf(pid,"%ld",GetCurrentThreadId());
#else
			
	#ifdef __REENTRANT
			sprintf(pid,"%d/%ld",getpid(),pthread_self() );			
	#else
			sprintf(pid,"%d",getpid());
	#endif

#endif	
			strcpy(aux_id,pid);
			break;
		case HDR_STFIC:
			strcpy(aux_id,"STFIC ");
			break;
		default:
			strcpy(aux_id," ");
	}
		//Añadimos el nombre de la aplicacion
	if (m_ptrAppName!=NULL)
	{
		strcat(aux_id,":");
		strcat(aux_id,m_ptrAppName);
	}

	switch(m_iTraceMode)
	{
		case MODE_STDOUT:
		{
			if(iLevel <= m_iTraceLevel)
			{
				if(szMessage1 != (char *) NULL)
					sprintf(s,"%s -- %s %s\n",aux_id,szMessage,szMessage1);
				else
					sprintf(s,"%s -- %s\n",aux_id,szMessage);

				fprintf(stdout,"%s",s);
			}
			break;
		}
		case MODE_FILE:
		{
			if(iLevel <= m_iTraceLevel)
			{
#ifdef __UNIX__
#ifdef __REENTRANT
				/* Esto lo hacemos para evitar problemas de bloqueo en UNIX cuando
				lo hacen threads en UNIX.*/
				guard.Lock();
#endif				
				lock.l_type = F_WRLCK;
				lock.l_whence = lock.l_start = lock.l_len = 0;
				if(fcntl(m_fdTrace,F_SETLKW,&lock) == -1)
				{
        				/*perror("Error lockf(F_TLOCK) en Trace");*/
        				return;
				}	
#endif
			}
			if(szMessage1 != (char *) NULL)
				sprintf(s,"%s -- %s %s\n",aux_id,szMessage,szMessage1);
			else
			{	
				sprintf(s,"%s -- %s\n",aux_id,szMessage);
				write(m_fdTrace,s,strlen(s));

#ifdef __UNIX__
				lock.l_type = F_UNLCK;
				lock.l_whence = lock.l_start = lock.l_len = 0;
				if(fcntl(m_fdTrace,F_SETLKW,&lock) == -1)
 				{
        			/*perror("Error lockf(F_ULOCK) en Trace");*/
        			return;
    			}
#ifdef __REENTRANT
				guard.UnLock();
#endif	
				
#endif
				if(iMaxLogLength)
					Close();
			
			}/*else*/
			break;		
		}
				
		default:
		{
			break;
		}	
	}
#else
    return;
#endif
#endif //VLOG_INCLUDE
}

/*! 
	TRACE modelno, con autoformato al estilo de las vprintf() 
*/
void IcTrace::VStfTrace(int iLevel,char *szFormat,...)
{
	struct stat buff;
	char *buf = NULL;
	int iMaxLogLength = 0;
	char *szNewFormat=NULL;
	
#ifdef __UNIX__
	if(g_resources)
		buf = g_resources->GetBuffer();
#endif		
	memset((void *)&buff,0,sizeof(buff));

	if( fstat(m_fdTrace,&buff) >= 0 && buff.st_size > 2000000000  )
	{
		iMaxLogLength = 1;
		szFormat = "Alcanzado el maximo tamaño peritido de 2 GBytes";
	}	
	
	if(!iLevel)
		return;
		
	++m_numLinea;

#ifdef VLOG_INCLUDE
	if(iLevel <= IcTrace::m_iTraceLevel)
	{
		va_list arguments;
		va_start(arguments,szFormat);
		vsprintf(buffer_necesario_pal_tracer_declarame_ya,szFormat,arguments);
		va_end(arguments);
		TRACE_R(DEFAULT____,"IcTrace",buffer_necesario_pal_tracer_declarame_ya);
	}
	return;
#else

	if( ( m_fdTrace == -1 || !m_FILETrace) && m_iTraceMode == MODE_FILE )
			return;
		
#if ((defined(WINDOWS) || defined(WIN32)) && defined(DEBUG)) || defined(__UNIX__)|| defined (REGIS)
	char aux_time[30],aux_id[200]/*,s[SIZE_TEXT]*/,pid[10];
	/*char text[SIZE_TEXT]*/;
	va_list arguments;
	va_start(arguments,szFormat);
#if defined(UNIX) || defined(__UNIX__)	

#ifdef __REENTRANT
	LockGuard guard(m_mutex);
#endif
	struct flock lock;	
#endif	

	switch(IcTrace::m_iHeader)
	{
		case HDR_TIME:
			strcpy(aux_id,(char *)tiempo_actual(aux_time));
			break;
		case HDR_PID:case HDR_RESOURCES: 
#ifdef WIN32
			sprintf(pid,"%ld",GetCurrentThreadId());
#elif defined(__REENTRANT)			
			sprintf(pid,"%d/%ld",getpid(),pthread_self());
#else
			sprintf(pid,"%d",getpid());
#endif	
			strcpy(aux_id,pid);
			break;
		case HDR_STFIC:
			strcpy(aux_id,"STFIC ");
			break;
		default:
			strcpy(aux_id," ");
	}
	//Añadimos el nombre de la aplicacion
	if (m_ptrAppName!=NULL)
	{
		strcat(aux_id,":");
		strcat(aux_id,m_ptrAppName);
	}
	strcat(aux_id," -- ");

	if( !(szNewFormat = (char *) malloc( strlen(szFormat) + sizeof(aux_id) + 2 ) ))
		return;	

	strcpy(szNewFormat,aux_id);
	strcat(szNewFormat,szFormat);
	strcat(szNewFormat,"\n");


	switch(IcTrace::m_iTraceMode)
	{
		case MODE_STDOUT:
		{
			if(iLevel <= IcTrace::m_iTraceLevel)
			{
				printf(aux_id);
				vprintf(szFormat,arguments);
				printf("\n");
			}
			break;
		}
		case MODE_FILE:
		{
			if(iLevel > IcTrace:: m_iTraceLevel)
			{
				free(szNewFormat);
				break;

			}
			if(iLevel <=IcTrace:: m_iTraceLevel)
			{
	
#ifdef __UNIX__
#ifdef __REENTRANT
				/* Esto lo hacemos para evitar problemas de bloqueo en UNIX cuando
				lo hacen threads en UNIX.*/
				guard.Lock();
#endif	
				lock.l_type = F_WRLCK;
				lock.l_whence = lock.l_start = lock.l_len = 0;

				if(fcntl(IcTrace::m_fdTrace,F_SETLKW,&lock) == -1)
				{
        			/*perror("Error lockf(F_TLOCK) en Trace");*/
					va_end(arguments);
        			free(szNewFormat);
					return;
    			}
#endif
		
#ifdef OSF1
	 		if(va_arg(arguments, char *))
			{
				va_start(arguments,szFormat);
#else	
				if(arguments)
				{
#endif	
					vfprintf(m_FILETrace,szNewFormat,arguments);
					if( buf )
					{
						fprintf( m_FILETrace,"%s", buf );
						free( buf );
					}
					fflush(m_FILETrace);
				}	
			
				free(szNewFormat);
			
#ifdef __UNIX__
				lock.l_type = F_UNLCK;
				lock.l_whence = lock.l_start = lock.l_len = 0;
				if(fcntl(IcTrace::m_fdTrace,F_SETLKW,&lock) == -1)
				{
        				/*perror("Error lockf(F_ULOCK) en Trace");*/
					va_end(arguments);
        			return;
    			}

		
#ifdef __REENTRANT
				/* Esto lo hacemos para evitar problemas de bloqueo en UNIX cuando
				lo hacen threads en UNIX.*/
				guard.UnLock();
#endif	
				
#endif
				if(iMaxLogLength)
					Close();
			}
			break;
		}
		default:break;
	}
	va_end(arguments);
#endif
	return;
#endif //VLOG_INCLUDE
}

//#endif


/*!
	Traceado como los anteriores, pero poniendo STF_ERR
	al principio de la linea para que se señale que es
	un error mas o menos grace.
*/
void IcTrace::VStfError(int iLevel,char *szFormat,...)
{
	if(!iLevel)
		return;
	
	++m_numLinea;

	if(m_fdTrace == -1)
		return;
#if defined(UNIX) || defined(__UNIX__)	
#ifdef __REENTRANT	
	LockGuard guard(m_mutex);
#endif
	struct flock lock;	
#endif			
#if ((defined(WINDOWS) || defined(WIN32)) && defined(DEBUG)) || defined(__UNIX__)|| defined (REGIS)
	char aux_time[40],aux_id[40],s[SIZE_TEXT];
	char text[SIZE_TEXT];
	va_list arguments;

	va_start(arguments,szFormat);
#if ( defined(UNIX) || defined(__UNIX__)) && defined (__REENTRANT)	
	sprintf(aux_id,"%ld STF_ERR %s",pthread_self(),(char *)tiempo_actual(aux_time));
#else
	sprintf(aux_id,"%d STF_ERR %s",getpid(),(char *)tiempo_actual(aux_time));
#endif	

	switch(IcTrace::m_iTraceMode)
	{
		case MODE_STDOUT:
		{
			if(iLevel <= IcTrace::m_iTraceLevel)
			{
				vsprintf(text,szFormat,arguments);
				sprintf(s,"%s -- %s\n",aux_id,text);
				fprintf(stdout,"%s",s);
			}
			break;
		}
		case MODE_FILE:
		{
			if(iLevel <=IcTrace:: m_iTraceLevel)
			{
#ifdef __UNIX__
#ifdef __REENTRANT
				/* Esto lo hacemos para evitar problemas de bloqueo en UNIX cuando
				lo hacen threads en UNIX.*/
				guard.Lock();
#endif					
				lock.l_type = F_WRLCK;
				lock.l_whence = lock.l_start = lock.l_len = 0;
				
				if(fcntl(IcTrace::m_fdTrace,F_SETLKW,&lock) == -1)
				{
        			/*perror("Error lockf(F_TLOCK) en Trace");*/
					va_end(arguments);
        			return;
    			}
#endif
				vsprintf(text,szFormat,arguments);
				sprintf(s,"%s -- %s\n",aux_id,text);

#ifndef VLOG_INCLUDE
				write(m_fdTrace,s,strlen(s));
#else
				TRACE_R(DEFAULT____,"IcTrace",s);
#endif
#ifdef __UNIX__
				lock.l_type = F_UNLCK;
				lock.l_whence = lock.l_start = lock.l_len = 0;
				if(fcntl(IcTrace::m_fdTrace,F_SETLKW,&lock) == -1)
				{
        			/*perror("Error lockf(F_ULOCK) en Trace");*/
					va_end(arguments);
        			return;
    			}
#ifdef __REENTRANT
				/* Esto lo hacemos para evitar problemas de bloqueo en UNIX cuando
				lo hacen threads en UNIX.*/
				guard.UnLock();
#endif					
#endif
			}
			break;
		}
		default:break;
	}
	va_end(arguments);
#endif
	return;
}


#ifdef MAIL_SYSTEM
int IcTrace::Send_Mail(char *from,char *dest)
{
	int sock_mail,retval,on=1;

	struct sockaddr_in mail_addr;

	if( (strcmp(from,"NO") == 0) || (strcmp(dest,"NO") == 0))
	{
		return(1);
	}

	if((sock_mail = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		Trace(4,"Send_Mail():Error socket(mail)",(char *)strerror((int)errno));
		return(-1);
	}

	mail_addr.sin_family = AF_INET;
	mail_addr.sin_addr.s_addr = inet_addr(m_pAddrMail);
	mail_addr.sin_port = htons(PUERTO_MAIL);

	memset(mail_addr.sin_zero,0,sizeof(mail_addr.sin_zero));

	if(connect(sock_mail,(struct sockaddr *)&mail_addr,sizeof(sockaddr)) == -1)
	{
		Trace(4,"Send_Mail():Error connect(mail)",(char *)strerror((int)errno));
		close(sock_mail);
		return(-1);
	}

#if 0
	retval = ioctl(sock_mail,FIONBIO,&on); /*socket no bloqueante */
	if(retval == -1)
	{
		Trace(4,"Send_Mail():Error ioctl(mail)",(char *)strerror((int)errno));
		close(sock_mail);
		return(-1);
	}
#endif
	retval = IcTrace::enviar_mail(sock_mail,from,dest);
	close(sock_mail);

	return(1);
}

int IcTrace::enviar_mail(int sock,char *from,char *dest)
{
	int retval,tam;
	char cad[605],buffer_r[256],buffer_s[256];
	long tam1;
	FILE *pf_mail;
	char *buf_mail,*mensaje;

	memset(buffer_r,0,256);
	memset(buffer_s,0,256);

	retval = IcTrace::recibe(sock,buffer_r);
	if(memcmp(buffer_r,"220",3))
	{
		Trace(4,"enviar_mail():No se recibio saludo",(char *)strerror((int)errno));
		return(-1);
	}

	sprintf(buffer_s,"%s\n","HELO");
	retval = send(sock,buffer_s,strlen(buffer_s),0);
	if(retval<0)
	{
		Trace(4,"enviar_mail():No pudo enviarse HELO",(char *)strerror((int)errno));
		return(-1);
	}

	retval = IcTrace::recibe(sock,buffer_r);
	if(retval<0 || memcmp(buffer_r,"250",3))
	{
		Trace(4,"enviar_mail():No se recibio respuesta a HELO",(char *)strerror((int)errno));
		return(-1);
	}

	sprintf(buffer_s,"%s %s\n","MAIL FROM:",from);

	retval = send(sock,buffer_s,strlen(buffer_s),0);
	if(retval<0)
	{
		Trace(4,"enviar_mail():No pudo enviarse FROM",(char *)strerror((int)errno));
		return(-1);
	}

	retval = IcTrace::recibe(sock,buffer_r);
	if(retval<0 || memcmp(buffer_r,"250",3))
	{
		Trace(4,"enviar_mail():No se recibio respuesta a FROM",(char *)strerror((int)errno));
		return(-1);
	}
	sprintf(buffer_s,"%s %s\n","RCPT TO:",dest);

	retval = send(sock,buffer_s,strlen(buffer_s),0);
	if(retval<0)
	{
		Trace(4,"enviar_mail():No pudo enviarse RCPT",(char *)strerror((int)errno));
		return(-1);
	}

	retval = IcTrace::recibe(sock,buffer_r);
	if(retval<0 || memcmp(buffer_r,"250",3))
	{
		Trace(4,"enviar_mail():No se recibio respuesta a RCPT",(char *)strerror((int)errno));
		return(-1);
	}

	sprintf(buffer_s,"%s\n","DATA");
	retval = send(sock,buffer_s,strlen(buffer_s),0);
	if(retval<0)
	{
		Trace(4,"enviar_mail():No pudo enviarse DATA",(char *)strerror((int)errno));
		return(-1);
	}

	retval = IcTrace::recibe(sock,buffer_r);
	if(retval<0 || memcmp(buffer_r,"354",3))
	{
		Trace(4,"enviar_mail():No se recibio respuesta a DATA",(char *)strerror((int)errno));
		return(-1);
	}
	/*
	sprintf(buf_mail,"%s\n.\n","STFIC le envio este mail en periodo de pruebas.\nSi puede Vd. leer esto,indica que el servido STFIC no se encuentra operativo.");
    */

    tam1 = tamanio("LOG.LOG");
    sprintf(cad,"%ld",tam1);
    tam = atoi(cad);

	pf_mail = fopen("LOG.LOG","rb");
	if(pf_mail == NULL)
	{
		Trace(4,"enviar_mail():Error fopen(log)",(char *)strerror((int)errno));
		return(-1);
	}

	buf_mail = (char *)malloc(tam*sizeof(char));
	if(buf_mail == NULL)
		return(-1);

	mensaje = (char *)malloc(tam*sizeof(char)+4);
	if(mensaje == NULL)
		return(-1);

	/*printf("voy a fread:\n");*/
	retval = fread(buf_mail,1,(int)tam,pf_mail);
	/*printf("Rtval:%d\n",retval);*/

	fclose(pf_mail);

    sprintf(mensaje,"%s\n.\n",buf_mail);

	retval = send(sock,mensaje,strlen(mensaje),0);
	if(retval<0)
	{
		Trace(4,"enviar_mail():No se pudo enviar cuerpo de mensaje",(char *)strerror((int)errno));
		return(-1);
	}

	retval = IcTrace::recibe(sock,buffer_r);
	if(retval<0 || memcmp(buffer_r,"250",3))
	{
		Trace(4,"enviar_mail():No se recibio respuesta",(char *)strerror((int)errno));
		return(-1);
	}

	free(buf_mail);

	sprintf(buffer_s,"%s\n","QUIT");
	retval = send(sock,buffer_s,strlen(buffer_s),0);
	if(retval<0)
	{
		Trace(4,"enviar_mail():No pudo enviarse QUIT",(char *)strerror((int)errno));
		return(-1);
	}

	retval = IcTrace::recibe(sock,buffer_r);
	if(memcmp(buffer_r,"221",3))
	{
		Trace(4,"enviar_mail():No se recibio respuesta a QUIT",(char *)strerror((int)errno));
		return(-1);
	}

	return(1);
}

int IcTrace::recibe(int sfd,char *buf)
{
	int ret,i=0;

	/*
		Intentamos leer 100 bytes,y buscamos si el CR esta en ellos
		Las cadenas enviadas por el servidor de correo nunca tienen
		mas de 100 bytes.
	*/

	/*for(;;)
	{
		ret = read(sfd,buf,90);
		if((void *)memchr(buf,'\n',1)) break;
	}*/

	char c;

	do
	{
		ret = read(sfd,&c,1);
		buf[i++] = c;
		/*printf("c:%c\n",c);*/
	}while(c != '\n');

	buf[i] = 0;
	puts(buf);

	return(1);
}
#endif

char *tiempo_actual(char *cad)
{
#ifndef WIN32
struct tm ptr;
#endif
 char *p;
 time_t lt;

 lt = time(NULL);

#ifndef WIN32
	localtime_r(&lt,&ptr);

#ifndef SunOS
  #if defined Linux || defined _POSIX_PTHREAD_SEMANTICS
	asctime_r(&ptr,cad);
  #else
	asctime_r(&ptr,cad,30);
  #endif
#else
    asctime_r(&ptr,cad,30);
#endif

#else
	p = asctime(localtime(&lt));
	if(p)
		strcpy(cad,p);
#endif

 p = strchr(cad,'\n');
 if(p) *p = '\0';

 return(cad);
}

/*!
	Funcion que permite tracear el nombre de la funcion, 
	linea y nombre dle fichero si se desea.
		\param iLevel.   Nivel de traceado.
		\parma szFormat. Formato a tracear. 
*/


void  TraceObject::OTrace(int iLevel,char *szFormat,...)
{
	if( !szFormat || iLevel <= 0 )
		return;

#if defined(UNIX) || defined(__UNIX__)	
#ifdef __REENTRANT	
	LockGuard guard(IcTrace::m_mutex);
#endif
	struct flock lock;	
#endif	

#ifdef __UNIX__
#ifdef __REENTRANT
		guard.Lock();
#endif	
		lock.l_type = F_WRLCK;
		lock.l_whence = lock.l_start = lock.l_len = 0;

		if(fcntl(IcTrace::m_fdTrace,F_SETLKW,&lock) == -1)
   			return;
#endif

		
#if ((defined(WINDOWS) || defined(WIN32)) && defined(DEBUG)) || defined(__UNIX__) || defined (REGIS)	
	char aux_time[30],aux_id[30],pid[10];
	
	if(iLevel > IcTrace::m_iTraceLevel)
		return;
	
	switch( IcTrace::m_iHeader )
	{
		case HDR_TIME:
			strcpy(aux_id,(char *)tiempo_actual(aux_time));
			break;
		case HDR_PID:
#ifdef WIN32
			sprintf(pid,"%ld",GetCurrentThreadId());
#else
	#ifdef __REENTRANT
			sprintf(pid,"%d/%ld",getpid(),pthread_self() );			
	#else
			sprintf(pid,"%d",getpid());
	#endif

#endif	
			strcpy(aux_id,pid);
			break;
		case HDR_STFIC:
			strcpy(aux_id,"STFIC ");
			break;
		default:
			strcpy(aux_id," ");
	}
	
	IcTrace::StfWTrace(0,iLevel,"%s ",aux_id);
		
	if( ( flags & FUNCTION_NAME ) && funcion_name && * funcion_name )
		IcTrace::StfWTrace(0,iLevel,"%s(): ",funcion_name);
				
	if( ( flags & FILE_NAME ) && file && *file )
		IcTrace::StfWTrace(0,iLevel,"PATH: %s ",file);
		
	if( ( flags & LINE_NUMBER ) && line)
		IcTrace::StfWTrace(0,iLevel,"LINEA: %d ",line);
		
	va_list arguments;
	va_start(arguments,szFormat);
	
#ifdef OSF1
 	if(va_arg(arguments, char *))
	{
		va_start(arguments,szFormat);
#else	
	if(arguments)
	{
#endif	
		IcTrace::StfWLTrace(0,iLevel,szFormat,arguments);
	}			
	
#endif
	
	IcTrace::StfWTrace(0,iLevel,"%s","\n");
	
#ifdef __UNIX__
	lock.l_type = F_UNLCK;
	lock.l_whence = lock.l_start = lock.l_len = 0;
	if(fcntl(IcTrace::m_fdTrace,F_SETLKW,&lock) == -1)
       	return;
#ifdef __REENTRANT
	guard.UnLock();
#endif					
#endif
	
	return;		 
}

void IcTrace::setAppName(char *name)
{
	unsetAppName();
	m_ptrAppName = strdup(name);
}

void IcTrace::unsetAppName()
{
	if(m_ptrAppName) free(m_ptrAppName);
	m_ptrAppName = NULL;
}

char* IcTrace::getAppName()
{
	return m_ptrAppName;
}


#ifdef SunOS
void IcTrace::Usage(int iLevel,char *szWhere)
{
	long maxsize,size,pagesize;
	struct rusage usage;

	pagesize = getpagesize();
	getrusage(RUSAGE_SELF,&usage);
	
	maxsize = pagesize * usage.ru_maxrss;
	size = pagesize * usage.ru_idrss;

	if(szWhere == NULL)
		VStfTrace(iLevel,"RUSAGE at %s >>> MaxSize %ld IntegralSize %ld",szWhere,maxsize,size);
	else
		VStfTrace(iLevel,"RUSAGE >>> MaxSize %ld IntegralSize %ld",maxsize,size);
}
#endif

#ifdef DANICTRACE
#	ifdef QUITADEBUG
#		undef DEBUG
#	endif
#endif

