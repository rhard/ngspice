/* Copyright 2013 Holger Vogt
 *
 * Modified BSD license 
 */

/* For comments and explanations see sharedspice.h */

/*******************/
/*   Defines       */
/*******************/

#ifdef _MSC_VER
#define SHAREDSPICE_version "25.1"
#define STDIN_FILENO    0
#define STDOUT_FILENO   1
#define STDERR_FILENO   2
#endif

/**********************************************************************/
/*              Header files for C functions                          */
/**********************************************************************/

#include <stdio.h>
#include <setjmp.h>

/* workaround since fputs, putc are replaced by sh_fputs,
sh_putc, through redefinition in ngspice.h */

int myputs(const char* inp, FILE* f);
int myputc(const char inp, FILE* f);
int myfputc(const char inp, FILE* f);

int
myputs(const char* inp, FILE* f)
{
    return fputs(inp, f);
}

int
myputc(const char inp, FILE* f)
{
    return putc(inp, f);
}

int
myfputc(const char inp, FILE* f)
{
    return fputc(inp, f);
}

#if defined(__MINGW32__) || defined(_MSC_VER)
#include <windows.h>
#endif

#include "ngspice/ngspice.h"
#include "misc/misc_time.h"

/*Use Windows threads if on W32 without pthreads*/
#ifndef HAVE_LIBPTHREAD

#if defined(__MINGW32__) || defined(_MSC_VER)
//#if defined(_MSC_VER)
#ifdef SRW
#define mutex_lock(a) AcquireSRWLockExclusive(a)
#define mutex_unlock(a) ReleaseSRWLockExclusive(a)
typedef SRWLOCK mutexType;
#else
#define mutex_lock(a) EnterCriticalSection(a)
#define mutex_unlock(a) LeaveCriticalSection(a)
typedef CRITICAL_SECTION mutexType;
#endif
#define thread_self() GetCurrentThread()
#define threadid_self() GetThreadId(GetCurrentThread())
typedef HANDLE threadId_t;
#define WIN_THREADS
#define THREADS

#endif

#else

#include <pthread.h>
#define mutex_lock(a) pthread_mutex_lock(a)
#define mutex_unlock(a) pthread_mutex_unlock(a)
#define thread_self() pthread_self()
#define threadid_self() 0  //FIXME t.b.d.
typedef pthread_mutex_t mutexType;
typedef pthread_t threadId_t;
#define THREADS

#endif


/* Copied from main.c in ngspice*/
#if defined(__MINGW32__)
#include <stdarg.h>
/* remove type incompatibility with winnt.h*/
#undef BOOLEAN
#include <windef.h>
#include <winbase.h>  /* Sleep */
#elif defined(_MSC_VER)
#include <stdarg.h>
/* remove type incompatibility with winnt.h*/
#undef BOOLEAN
#include <windows.h> /* Sleep */
#include <process.h> /* _getpid */
#define dup _dup
#define dup2 _dup2
#define open _open
#define close _close
#else
#include <unistd.h> /* usleep */
#endif /* __MINGW32__ */

#include "ngspice/iferrmsg.h"
#include "ngspice/ftedefs.h"
#include "ngspice/devdefs.h"
#include <spicelib/devices/dev.h>
#include <spicelib/analysis/analysis.h>
#include <misc/ivars.h>
#include <frontend/resource.h>
#include <frontend/com_measure2.h>
#ifdef _MSC_VER
#include <stdio.h>
#define snprintf _snprintf
#endif
#include <frontend/outitf.h>
#include "ngspice/memory.h"
#include <frontend/com_measure2.h>
#include <frontend/misccoms.h>

#ifndef HAVE_GETRUSAGE
#ifdef HAVE_FTIME
#include <sys/timeb.h>
#endif
#endif

/* To interupt a spice run */
#include <signal.h>
typedef void (*sighandler)(int);

#include <setjmp.h>
#include "frontend/signal_handler.h"

/*Included for the module to access data*/
#include "ngspice/dvec.h"
#include "ngspice/plot.h"

#ifdef __CYGWIN__
#undef WIN32
#endif
#include  "ngspice/sim.h"

/*For get_output*/
#include <fcntl.h>
#include <sys/stat.h>

#ifdef _MSC_VER
#define S_IRWXU _S_IWRITE
#endif

#ifdef HAVE_ASPRINTF
#ifdef HAVE_LIBIBERTY_H /* asprintf */
#include <libiberty.h>
#elif defined(__MINGW32__) || defined(__SUNPRO_C) /* we have asprintf, but not libiberty.h */
#include <stdarg.h>
extern int asprintf(char **out, const char *fmt, ...);
extern int vasprintf(char **out, const char *fmt, va_list ap);
#endif
#endif

extern IFfrontEnd nutmeginfo;

extern struct comm spcp_coms[ ];
extern void DevInit(void);
extern int SIMinit(IFfrontEnd *frontEnd, IFsimulator **simulator);
extern wordlist *cp_varwl(struct variable *var);
extern void create_circbyline(char *line);
extern void initw(void);


/*The current run (to get variable names, etc)*/
static runDesc *cur_run;

//static int ownVectors;

void sh_stdflush(FILE *f);
int  sh_vfprintf(FILE *f, const char *fmt, va_list args);

//int sh_fputs(const char *input, FILE* outf);
//int sh_fputc(const char input, FILE* outf);
//int sh_putc(const char input, FILE* outf);
int sh_fputsll(const char *input, FILE* outf);

int sh_ExecutePerLoop(void);
int sh_vecinit(runDesc *run);

void shared_exit(int status);

void sighandler_sharedspice(int num);

void wl_delete_first(wordlist **wlstart, wordlist **wlend);

#if !defined(low_latency)
static char* outstorage(char*, bool);
static void* printsend(void);
#endif

#include "ngspice/sharedspice.h"

static SendChar* pfcn;
static void* userptr;
static SendStat* statfcn;
static ControlledExit* ngexit;
static SendData* datfcn;
static SendInitData* datinitfcn;
static BGThreadRunning* bgtr;
static pvector_info myvec = NULL;
char **allvecs = NULL;
char **allplots = NULL;
static bool noprintfwanted = FALSE;
static bool nostatuswanted = FALSE;
static bool nodatawanted = FALSE;
static bool nodatainitwanted = FALSE;
static bool nobgtrwanted = FALSE;
static bool immediate = FALSE;
static bool coquit = FALSE;
static jmp_buf errbufm, errbufc;
static int intermj = 1;


// thread IDs
unsigned int main_id, ng_id, command_id;

#ifdef THREADS
mutexType triggerMutex;
mutexType allocMutex;
mutexType fputsMutex;
#endif

/* initialization status */
static bool is_initialized = FALSE;
static char* no_init = "Error: ngspice is not initialized!\n   Run ngSpice_Init first";

/*helper function*//*
static struct plot *
get_plot(int plot)
{
    struct plot *pl;
    pl = plot_list;
    for (; 0 < plot; plot--) {
        pl = pl->pl_next;
        if (!pl)
            return NULL;
    }
    return pl;
}
*/
static struct plot *
get_plot_byname(char* plotname)
{
    struct plot *pl;
    pl = plot_list;
    while (pl) {
        if(cieq(pl->pl_typename, plotname))
            break;
        pl = pl->pl_next;
    }
    return pl;
}


/*this holds the number of time points done (altered by spice)*/
//int steps_completed;



/******************************************************************/
/*     Main spice command executions and thread control           */
/*****************************************************************/

#ifdef THREADS
#ifdef __MINGW32__
static threadId_t tid, printtid, bgtid;
#else
static threadId_t tid, printtid, bgtid = (threadId_t) 0;
#endif

static bool fl_running = FALSE;
static bool fl_exited = TRUE;

#if defined(__MINGW32__) || defined(_MSC_VER)
#define EXPORT_FLAVOR WINAPI
#else
#define EXPORT_FLAVOR
#endif

static void * EXPORT_FLAVOR
_thread_run(void *string)
{
    ng_id = threadid_self();
    fl_exited = FALSE;
    /* notify caller that thread is running */
    if (!nobgtrwanted)
        bgtr(fl_exited, userptr);
    bgtid = thread_self();
    cp_evloop((char *)string);
    FREE(string);
#ifdef __MINGW32__nn
    bgtid.p = NULL;
    bgtid.x = 0;
#else
    bgtid = (threadId_t)0;
#endif
    fl_exited = TRUE;
    /* notify caller that thread has exited */
    if (!nobgtrwanted)
        bgtr(fl_exited, userptr);
    return NULL;
}


/*Stops a running thread, hopefully */
static int EXPORT_FLAVOR
_thread_stop(void)
{
    int timeout = 0;

    if (fl_running) {
        while (!fl_exited && timeout < 100) {
            /* ft_intrpt is the flag to stop simulation, if set TRUE !
               E.g. SPfrontEnd->IFpauseTest() in dctran.c points to
               OUTstopnow(void), which returns 1, which leads dctran
               to return with -1 and thus terminates the simulation*/
            ft_intrpt = TRUE;
            timeout++;
#if defined(__MINGW32__) || defined(_MSC_VER)
            Sleep(50); // va: windows native
#else
            usleep(10000);
#endif
        } 
        if (!fl_exited) {
            fprintf(stderr, "Error: Couldn't stop ngspice\n");
            return EXIT_BAD;
        }
        else
            fprintf(stdout, "Background thread stopped with timeout = %d\n", timeout);
#ifdef HAVE_LIBPTHREAD
        pthread_join(tid, NULL);
#endif
        fl_running = FALSE;
        ft_intrpt = FALSE;
        return EXIT_NORMAL;
    } else {
        fprintf(stderr, "Spice not running\n");
    }
    return EXIT_NORMAL;
}


void
sighandler_sharedspice(int num)
{
    NG_IGNORE(num);
    if (fl_running)
        _thread_stop();
    return;
}

#endif /*THREADS*/


static int
runc(char* command)
{
    char buf[1024] = "";
    sighandler oldHandler;
#ifdef THREADS
    char *string;
    bool fl_bg = FALSE;
    command_id = threadid_self();
    /* run task in background if command is preceeded by "bg_" */
    if (!cieq("bg_halt", command) && (ciprefix("bg_", command))) {
        strncpy(buf, command+3, 1024);
        fl_bg = TRUE;
    }
    else
        strncpy(buf, command, 1024);
#else
    strncpy(buf, command, 1024);
#endif

    /* Catch Ctrl-C to break simulations */
#if !defined(_MSC_VER) /*&& !defined(__MINGW32__) */
    oldHandler = signal(SIGINT, (SIGNAL_FUNCTION) ft_sigintr);
    if (SETJMP(jbuf, 1) != 0) {
        signal(SIGINT, oldHandler);
        return 0;
    }
#else
    oldHandler = SIG_IGN;
#endif

    

#ifdef THREADS
    /* run in the background */
    if (fl_bg && fl_exited) {
        if (fl_running)
            _thread_stop();
        fl_running = TRUE;
        string = copy(buf);     /*as buf gets freed fairly quickly*/
#ifdef HAVE_LIBPTHREAD
        pthread_create(&tid, NULL, (void * (*)(void *))_thread_run, (void *)string);
#elif defined _MSC_VER || defined __MINGW32__
        tid = (HANDLE)_beginthreadex(NULL, 0, (unsigned int (__stdcall *)(void *))_thread_run, 
            (void*)string, 0, NULL);
#else
        tid = CreateThread(NULL, 0, (PTHREAD_START_ROUTINE)_thread_run, (void*)string,
                         0, NULL);
#endif
    } else
        /* bg_halt (pause) a bg run */
        if (!strcmp(buf, "bg_halt")) {
            signal(SIGINT, oldHandler);
            return _thread_stop();
        } else
            /* cannot do anything if ngspice is running in the bg*/
            if (fl_running) {
                if (fl_exited) {
                    _thread_stop();
                    cp_evloop(buf);
                } else {
                    fprintf(stderr, "Warning: cannot execute \"%s\", type \"bg_halt\" first\n", buf);
                }
            } else {
                /*do the command*/
                cp_evloop(buf);
            }
#else
    cp_evloop(buf);
#endif /*THREADS*/
    signal(SIGINT, oldHandler);
    return 0;
}



/**********************************************************/
/* The functions exported explicitely from shared ngspice */
/**********************************************************/

#ifdef THREADS

/* Checks if spice is running in the background        */
IMPEXP
bool
ngSpice_running (void)
{
    return (fl_running && !fl_exited);
}
#endif


/*  Initialise spice and setup native methods          */
IMPEXP
int
ngSpice_Init(SendChar* printfcn, SendStat* statusfcn, ControlledExit* ngspiceexit, 
             SendData* sdata, SendInitData* sinitdata, BGThreadRunning* bgtrun, void* userData)
{
    sighandler old_sigint;

    pfcn = printfcn;
    /* if caller sends NULL, don't send printf strings */
    if (!pfcn)
        noprintfwanted = TRUE;
    userptr = userData;
    statfcn = statusfcn;
    /* if caller sends NULL, don't send status data */
    if (!statfcn)
        nostatuswanted = TRUE;
    ngexit = ngspiceexit;
    datfcn = sdata;
    /* if caller sends NULL, don't send data */
    if (!datfcn)
        nodatawanted = TRUE;
    /* if caller sends NULL, don't initialize and send data */
    datinitfcn = sinitdata;
    if (!datinitfcn)
        nodatawanted = nodatainitwanted = TRUE;
    bgtr = bgtrun;
    if (!bgtr)
        nobgtrwanted = TRUE;
    immediate = FALSE;

#ifdef THREADS
    /* init the mutexes */
#ifdef HAVE_LIBPTHREAD
    pthread_mutex_init(&triggerMutex, NULL);
    pthread_mutex_init(&allocMutex, NULL);
    pthread_mutex_init(&fputsMutex, NULL);
#else
#ifdef SRW
    InitializeSRWLock(&triggerMutex);
    InitializeSRWLock(&allocMutex);
    InitializeSRWLock(&fputsMutex);
#else
    InitializeCriticalSection(&triggerMutex);
    InitializeCriticalSection(&allocMutex);
    InitializeCriticalSection(&fputsMutex);
#endif
#endif
    // Id of primary thread
    main_id =  threadid_self();
    signal(SIGINT, sighandler_sharedspice);
#endif

    ft_rawfile = NULL;
    ivars(NULL);

    cp_in = stdin;
    cp_out = stdout;
    cp_err = stderr;

    /*timer*/
    init_time();

    /*IFsimulator struct initilised*/
    SIMinit(&nutmeginfo, &ft_sim);

    /* program name*/
    cp_program = ft_sim->simulator;

    srand((unsigned int) getpid());
    TausSeed();

    /*parameter fetcher, used in show, alter, altermod */
//    if_getparam = spif_getparam;
    if_getparam = spif_getparam_special;

    /* Get startup system limits */
    init_rlimits();

    /*Command prompt stuff */
    ft_cpinit();

    /* Read the user config files */
    /* To catch interrupts during .spiceinit... */
    old_sigint = signal(SIGINT, (SIGNAL_FUNCTION) ft_sigintr);
    if (SETJMP(jbuf, 1) == 1) {
        fprintf(cp_err, "Warning: error executing .spiceinit.\n");
        goto bot;
    }

#ifdef HAVE_PWD_H
    /* Try to source either .spiceinit or ~/.spiceinit. */
    if (access(".spiceinit", 0) == 0) {
        inp_source(".spiceinit");
    } else {
        char *s;
        struct passwd *pw;
        pw = getpwuid(getuid());

#ifdef HAVE_ASPRINTF
        asprintf(&s, "%s%s", pw->pw_dir, INITSTR);
#else
        s = TMALLOC(char, 1 + strlen(pw->pw_dir) + strlen(INITSTR));
        sprintf(s, "%s%s", pw->pw_dir, INITSTR);
#endif
        if (access(s, 0) == 0)
            inp_source(s);
    }
#else /* ~ HAVE_PWD_H */
    {
        FILE *fp;
        /* Try to source the file ".spiceinit" in the current directory.  */
        if ((fp = fopen(".spiceinit", "r")) != NULL) {
            (void) fclose(fp);
            inp_source(".spiceinit");
        }
    }
#endif /* ~ HAVE_PWD_H */
bot:
    signal(SIGINT, old_sigint);

    /* initilise display to 'no display at all'*/
    DevInit();

#ifdef FastRand
// initialization and seed for FastNorm Gaussian random generator
    {
        unsigned int rseed = 66;
        initnorm (0, 0);
        if (!cp_getvar("rndseed", CP_NUM, &rseed)) {
            time_t acttime = time(NULL);
            rseed = (unsigned int) acttime;
        }
        initnorm (rseed, 2);
        fprintf (cp_out, "SoS %f, seed value: %ld\n", renormalize(), rseed);
    }
#elif defined (WaGauss)
    {
        unsigned int rseed = 66;
        if (!cp_getvar("rndseed", CP_NUM, &rseed)) {
            time_t acttime = time(NULL);
            rseed = (unsigned int) acttime;
        }
        srand(rseed);
        initw();
    }
#endif

    com_version(NULL);

    is_initialized = TRUE;
     
    if(!myvec)
        myvec = TMALLOC(vector_info, sizeof(vector_info));

#if !defined(low_latency)
    /* If caller has sent valid address for pfcn */
    if (!noprintfwanted)
#ifdef HAVE_LIBPTHREAD
        pthread_create(&tid, NULL, (void * (*)(void *))printsend, (void *)NULL);
#elif defined _MSC_VER || defined __MINGW32__
        printtid = (HANDLE)_beginthreadex(NULL, 0, (unsigned int (__stdcall *)(void *))printsend, 
            (void*) NULL, 0, NULL);
#else
        printtid = CreateThread(NULL, 0, (PTHREAD_START_ROUTINE)printsend, (void*)NULL,
                         0, NULL);
#endif


#endif

    return 0;
}

/* retrieve a ngspice command from caller and run it 
immediately */
IMPEXP
int  ngSpice_Command(char* comexec)
{
    if ( ! setjmp(errbufc) ) {
//       HANDLE tid2;
    
        immediate = FALSE;       
        intermj = 1;

        if (!is_initialized) {
           fprintf(stderr, no_init);
           return 1;
       }
//       tid2 = (HANDLE)_beginthreadex(NULL, 0, (PTHREAD_START_ROUTINE)runc, (void*)comexec,
//                         0, NULL);
       runc(comexec);
       /* main thread prepares immediate detaching of dll,
       in case of controlled_exit from tid2 thread, caller is asked
       to detach dll via fcn ngexit() */
       immediate = TRUE;
       return 0;
    }
    return 1;
};

/* Return information about a vector to the caller */
IMPEXP
pvector_info  ngGet_Vec_Info(char* vecname)
{
    struct dvec* newvec;

    if (!is_initialized) {
        fprintf(stderr, no_init);
        return NULL;
    }

    newvec = vec_get(vecname);

    if (newvec == NULL) {
        fprintf(stderr, "Error: vector %s not found!\n", vecname);
        return NULL;
    }
    if (newvec->v_numdims > 1) {
        fprintf(stderr, "Error: vector %s is multidimensional!\n  This is not yet handled\n!", vecname);
        return NULL;
    }

    myvec->v_name = newvec->v_name;
    myvec->v_type = newvec->v_type;
    myvec->v_flags = newvec->v_flags;
    myvec->v_realdata = newvec->v_realdata;
    myvec->v_compdata = newvec->v_compdata;
    myvec->v_length = newvec->v_length;
    return myvec;
};

/* Receive a circuit from the caller as a
   pointer to an array of char* .
   Last entry in array has to be NULL
*/
IMPEXP
int ngSpice_Circ(char** circa){
    int entries = 0, i;
    char* newline;

    if ( ! setjmp(errbufm) ) {
        intermj = 0;
        /* count the entries */
        while (circa[entries]) {
            entries++;
        }
        /* create a local copy (to be freed in inpcom.c) */
        for (i = 0; i < entries; i++) {
            newline = copy(circa[i]);
            create_circbyline(newline);
        }
        return 0;
    }
    /* upon error */
    return 1;
}


/* return to the caller a pointer to the name of the current plot */
IMPEXP
char* ngSpice_CurPlot(void)
{
    struct plot *pl = plot_cur;
    return pl->pl_typename;
}

/* return to the caller a pointer to an array of all plots created 
by ngspice.dll */
IMPEXP
char** ngSpice_AllPlots(void)
{
    int len = 0, i = 0;
    struct plot *pl = plot_list;
    if (allplots)
        tfree(allplots);

    while (pl) {
        len++;
        pl = pl->pl_next;
    }
    allplots = TMALLOC(char*, len+1);
    pl = plot_list;
    for (i = 0; i < len; i++) {
        allplots[i] = pl->pl_typename;
        pl = pl->pl_next;
    }
    allplots[len] = '\0';
    return allplots;
}

/* return to the caller a pointer to an array of vector names in the plot
named by plotname */
IMPEXP
char** ngSpice_AllVecs(char* plotname)
{
    struct dvec *d;
    int len = 0, i = 0;
    struct plot *pl;

    if (allvecs)
        tfree(allvecs);

    /* get the plot plotname */
    pl = get_plot_byname(plotname);

    if (pl)
        for (d = pl->pl_dvecs; d; d = d->v_next)
            len++;

    if (len == 0) {
        fprintf(cp_err, "Error: There are no vectors currently active.\n");
        return NULL;
    }

    allvecs = TMALLOC(char*, len + 1);

    for (d = pl->pl_dvecs, i = 0; d; d = d->v_next, i++)
        allvecs[i] = d->v_name;

    allvecs[len] = NULL;

    return allvecs;
}


/*------------------------------------------------------*/
/* Redefine the vfprintf() functions for callback       */
/*------------------------------------------------------*/

/* handling of escape characters (extra \ added) is removed, may be added by 
   un-commenting some lines */

int
sh_vfprintf(FILE *f, const char *fmt, va_list args)
{
    char buf[1024];
    char *p/*, *s*/;
    int nchars, /*escapes,*/ result;
    size_t size;


    if ((fileno(f) != STDOUT_FILENO && fileno(f) != STDERR_FILENO &&
         f != stderr && f != stdout)
#ifdef THREADS
//        || (fl_running && bgtid == thread_self())
#endif
        )
        return vfprintf(f, fmt, args);

    p = buf;

    // size: how much ist left for chars and terminating '\0'
    size = sizeof(buf);
    // assert(size > 0);

    for (;;) {
        nchars = vsnprintf(p, size, fmt, args);

        if(nchars == -1) {           // compatibility to old implementations
            size *= 2;
            nchars = vsnprintf(p, size, fmt, args);
        } else if (size < (size_t)nchars + 1) {
            size = (size_t)nchars + 1;
        } else {
            break;
        }

        if(p == buf)
            p = TMALLOC(char, size);
        else
            p = TREALLOC(char, p, size);
    }

    /* add / to escape characters, if 'set addescape' is called in .spiceinit */
    if (cp_getvar("addescape", CP_BOOL, NULL)) {
        size_t escapes;
        const char * const escape_chars = "$[]\"\\";
        char *s = p;
        for (escapes = 0; ; escapes++) {
            s = strpbrk(s, escape_chars);
            if (!s)
                break;
            s++;
        }

        if (escapes) {

            size_t new_size = (size_t)nchars + escapes + 1;
            char *src, *dst;

            if (p != buf) {
                p = TREALLOC(char, p, new_size);
            } else if (new_size > sizeof(buf)) {
                p = TMALLOC(char, new_size);
                strcpy(p, buf);
            }

            src = p + nchars;
            dst = src + escapes;

            while (dst > src) {
                char c = *--src;
                *--dst = c;
                if (strchr(escape_chars, c))
                    *--dst = '\\';
            }
        }
    }

    /* use sharedspice implementation of fputs (sh_fputs)
       to assess callback function derived from address printfcn received via
       Spice_Init() from caller of ngspice.dll */


    result = sh_fputs(p, f);

    if (p != buf)
        tfree(p);

    return nchars;
}


/*----------------------------------------------------------------------*/
/* Reimplement fprintf() as a call to callback function pfcn                     */
/*----------------------------------------------------------------------*/

int
sh_fprintf(FILE *f, const char *format, ...)
{
    va_list args;
    int rtn;

    va_start (args, format);
    rtn = sh_vfprintf(f, format, args);
    va_end(args);

    return rtn;
}


/*----------------------------------------------------------------------*/
/* Reimplement printf() as a call to callback function pfcn                        */
/*----------------------------------------------------------------------*/

int
sh_printf(const char *format, ...)
{
    va_list args;
    int rtn;

    va_start (args, format);
    rtn = sh_vfprintf(stdout, format, args);
    va_end(args);

    return rtn;
}

int
sh_putc(const char inp, FILE* f) 
{
    char inpconv[2];
    if ((fileno(f) != STDOUT_FILENO && fileno(f) != STDERR_FILENO &&
    f != stderr && f != stdout))
        return myfputc(inp, f);

    sprintf(inpconv, "%c", inp); 
    fputs(inpconv, f);
    return inp;
}

int
sh_fputc(const char inp, FILE* f) 
{
    char inpconv[2];
    if ((fileno(f) != STDOUT_FILENO && fileno(f) != STDERR_FILENO &&
    f != stderr && f != stdout))
        return myfputc(inp, f);

    sprintf(inpconv, "%c", inp); 
    fputs(inpconv, f);
    return inp;
}

/*----------------------------------------------------------------------*/
/* Reimplement fputs() as a call to callback function pfcn              */
/*----------------------------------------------------------------------*/


/* Collect and cat strings.
   If \n is detected, send string 
   to caller via pfcn() */
static char* outstringerr = NULL;
static char* outstringout = NULL;

#ifdef low_latency

int
sh_fputsll(const char *input, FILE* outf)
{
    int result = 0;
    size_t len;
    char *delstring, *newstring, *prstring;
    size_t inputlen = strlen(input);

    /* If caller has sent NULL address for pfcn */
    if (noprintfwanted)
        return -1;

    if (outf == stderr) {
        if (!outstringerr)
            delstring = outstringerr = copy(input);
        else {
            len = strlen(outstringerr);
            delstring = outstringerr = TREALLOC(char, outstringerr, len + inputlen + 2);
            strcat(outstringerr, input);
        }
        if (strchr(input, '\n')) {
            while (outstringerr) {
                newstring = gettok_char(&outstringerr, '\n', FALSE, FALSE);
                if(!newstring)
                    break;
                prstring = TMALLOC(char, 7 + strlen(newstring) + 1);
                strcat(prstring, "stderr ");
                strcat(prstring, newstring);

                printtid = (HANDLE)_beginthreadex(NULL, 0, (unsigned int (__stdcall *)(void *))printing, 
                    (void*)prstring, 0, NULL);

//                result = pfcn(prstring, userptr);
                tfree(newstring);
                tfree(prstring);
            }
            /* copy the rest following \n, but without trailing \n to new address */
            if (outstringerr && *outstringerr != '\0')
                outstringerr = copy(outstringerr);
            else
                outstringerr = NULL;
            tfree(delstring);
            return result;
        } 
        else if (strchr(input, '\r')) {
            result = pfcn(outstringerr, userptr);
            tfree(outstringerr);
            return result;
        }
    }
    if (outf == stdout) {
        if (!outstringout)
            delstring = outstringout = copy(input);
        else {
            len = strlen(outstringout);
            delstring = outstringout = TREALLOC(char, outstringout, len + inputlen + 1);
            strcat(outstringout, input);
        }
        if (strchr(input, '\n')) {
            while (outstringout) {
                newstring = gettok_char(&outstringout, '\n', FALSE, FALSE);
                if(!newstring)
                    break;
                prstring = TMALLOC(char, 7 + strlen(newstring) + 1);
                strcat(prstring, "stdout ");
                strcat(prstring, newstring);
                result = pfcn(prstring, userptr);
                tfree(newstring);
                tfree(prstring);
            }
            /* copy the rest following \n, but without trailing \n to new address */
            if (outstringout && *outstringout != '\0')
                outstringout = copy(outstringout);
            else
                outstringout = NULL;
            tfree(delstring);
            return result;
        } 
        else if (strchr(input, '\r')) {
            result = pfcn(outstringout, userptr);
            tfree(outstringout);
            return result;
        }
    }
    else
        myputs(input, outf);

    return 0;
}

/* allow a lock around printing function */
int
sh_fputs(const char *input, FILE* outf)
{
    mutex_lock(&fputsMutex);
    sh_fputsll(input, outf);
    mutex_unlock(&fputsMutex);
    return 0;
}

#else

/* FIFO storage for strings created by fputs and all other printing commands. 
   A string will be appended to the FIFO by fcn
   outstorage() by the main thread or the background (bg_) thread.
   Each string is read from top of the FIFO by independent thread using
   again fcn outstoraghe(), top entry is deleted and string is 
   sent to caller in an endless loop by fcn printsend() */
static wordlist *wlstart = NULL, *wlend = NULL;



int
sh_fputs(const char *input, FILE* outf)
{
    int result = 0;
    size_t len;
    char *delstring, *newstring, *prstring;
    size_t inputlen = strlen(input);

    /* If caller has sent NULL address for pfcn */
    if (noprintfwanted)
        return -1;

    if (outf == stderr) {
        if (!outstringerr)
            delstring = outstringerr = copy(input);
        else {
            len = strlen(outstringerr);
            delstring = outstringerr = TREALLOC(char, outstringerr, len + inputlen + 2);
            strcat(outstringerr, input);
        }
        if (strchr(input, '\n')) {
            while (outstringerr) {
                newstring = gettok_char(&outstringerr, '\n', FALSE, FALSE);
                if(!newstring)
                    break;
                prstring = TMALLOC(char, 7 + strlen(newstring) + 1);
                strcat(prstring, "stderr ");
                strcat(prstring, newstring);
                mutex_lock(&fputsMutex);
                outstorage(prstring, TRUE);
                mutex_unlock(&fputsMutex);
                tfree(newstring);
                prstring = NULL; /* keep prstring here, address is in use */
            }
            /* copy the rest following \n, but without trailing \n to new address */
            if (outstringerr && *outstringerr != '\0')
                outstringerr = copy(outstringerr);
            else
                outstringerr = NULL;
            tfree(delstring);
            return result;
        } 
        else if (strchr(input, '\r')) {
            mutex_lock(&fputsMutex);
            outstorage(outstringerr, TRUE);
            mutex_unlock(&fputsMutex);
            outstringerr = NULL;
            return 0;
        }
    }
    if (outf == stdout) {
        if (!outstringout)
            delstring = outstringout = copy(input);
        else {
            len = strlen(outstringout);
            delstring = outstringout = TREALLOC(char, outstringout, len + inputlen + 1);
            strcat(outstringout, input);
        }
        if (strchr(input, '\n')) {
            while (outstringout) {
                newstring = gettok_char(&outstringout, '\n', FALSE, FALSE);
                if(!newstring)
                    break;
                prstring = TMALLOC(char, 7 + strlen(newstring) + 1);
                strcat(prstring, "stdout ");
                strcat(prstring, newstring);
                mutex_lock(&fputsMutex);
                outstorage(prstring, TRUE);
                mutex_unlock(&fputsMutex);
                tfree(newstring);
                prstring = NULL;
            }
            /* copy the rest following \n, but without trailing \n to new address */
            if (outstringout && *outstringout != '\0')
                outstringout = copy(outstringout);
            else
                outstringout = NULL;
            tfree(delstring);
            return result;
        } 
        else if (strchr(input, '\r')) {
            mutex_lock(&fputsMutex);
            outstorage(outstringout, TRUE);
            mutex_unlock(&fputsMutex);
            outstringout = NULL;
            return result;
        }
    }
    else
        myputs(input, outf);

    return 0;
}

/* Endless lop in its own thread for reading data from FIFO and sending to caller */
static void *
printsend(void)
{
    char *outsend = NULL;

    for (;;) {
#if defined(__MINGW32__) || defined(_MSC_VER)
        Sleep(100);
#else
        usleep(100000);
#endif
        mutex_lock(&fputsMutex);
        outsend = outstorage(NULL, FALSE);
        mutex_unlock(&fputsMutex);
        if (outsend) {
            /* requires outsend to be copied by the caller, 
            because it is freed immediately */
            pfcn(outsend, userptr);
            tfree(outsend);
        }
    }
}

/* remove the first entry of a wordlist, but keep wl->wl_word */
void wl_delete_first(wordlist **wlstart, wordlist **wlend)
{
    wordlist *wl_temp;
    
    if (!(*wlstart))
        return;
    if ((*wlstart) && !((*wlstart)->wl_next)) {
        tfree(*wlstart); /* keep wlstart->wl_word */
        (*wlstart) = NULL;
        (*wlend) = NULL;
        return;
    }
    wl_temp = (*wlstart)->wl_next;
    wl_temp->wl_prev = NULL;
    tfree(*wlstart); /* keep wlstart->wl_word */
    (*wlstart) = wl_temp;
}


/* create a wordlist FIFO using global static variables wlstart and wlend.
   wordin has to be malloced on the heap */
char* outstorage(char* wordin, bool write)
{
    char *wordout = NULL;
    
    if(write)
        wl_append_word(&wlstart, &wlend, wordin);
    else if (wlstart) {
        wordout = wlstart->wl_word;
        wl_delete_first(&wlstart, &wlend);
    }
    return wordout;
}

#endif


/* New progress report to statfcn().
   Update only every DELTATIME milliseconds */
#define DELTATIME 150
void SetAnalyse( 
   char * Analyse, /*in: analysis type */
   int DecaPercent /*in: 10 times the progress [%]*/
   /*HWND hwAnalyse, in: global handle to analysis window */   
) {
   static int OldPercent = -2;     /* Previous progress value */
   static char OldAn[128];         /* Previous analysis type */
   char* s;                        /* outputs to callback function */
   static struct timeb timebefore; /* previous time stamp */
   struct timeb timenow;           /* actual time stamp */
   int diffsec, diffmillisec;      /* differences actual minus prev. time stamp */
   int result;                     /* return value from callback function */

   /* If caller has sent NULL address for statfcn */
   if (nostatuswanted)
       return;

   if ((DecaPercent == OldPercent) && !strcmp(OldAn, Analyse)) 
       return;

   /* get actual time */
   ftime(&timenow);
   timediff(&timenow, &timebefore, &diffsec, &diffmillisec);
   s = TMALLOC(char, 128);

   if (DecaPercent >= 1000){
       sprintf( s, "--ready--");
       result = statfcn(s, userptr);
       tfree(s);
       return;
   }
   /* info every one percent of progress:
      actual time, progress, 
      to catch linearity of progress of simulation */
   if (ft_ngdebug && !strcmp(Analyse, "tran")) 
      if ((int)((double)DecaPercent/10.) > (int)((double)OldPercent/10.)) {
         printf("%3.1f%% percent progress after %4.2f seconds.\n", (double)DecaPercent/10., seconds());                
      }   
   OldPercent = DecaPercent;   
   /* output only into hwAnalyse window and if time elapsed is larger than
      DELTATIME given value, or if analysis has changed, else return */
   if ((diffsec > 0) || (diffmillisec > DELTATIME) || strcmp(OldAn, Analyse)) {
        if (DecaPercent < 0) {
            sprintf( s, "--ready--");
        }   
      else if (DecaPercent == 0) {
         sprintf( s, "%s", Analyse);
      }  
      else if (!strcmp(Analyse, "shooting")) {
         sprintf( s, "%s: %d", Analyse, DecaPercent);
      }
      else {
         sprintf( s, "%s: %3.1f%%", Analyse, (double)DecaPercent/10.);
      } 
      timebefore.dstflag = timenow.dstflag;
      timebefore.millitm = timenow.millitm;
      timebefore.time = timenow.time;
      timebefore.timezone = timenow.timezone;
      /* info when previous analysis period has finished */
      if (strcmp(OldAn, Analyse)) {
         if (ft_ngdebug && (strcmp(OldAn, "")))
            printf("%s finished after %4.2f seconds.\n", OldAn, seconds());
         strncpy(OldAn, Analyse, 127);
      }
          
      result = statfcn(s, userptr);
   }
   tfree(s);
}

/* a dll or shared library should never exit, but ask for graceful shutdown 
   (e.g. being detached) via a callback function*/
void shared_exit(int status)
{
    /* alert caller to detach dll (if we are in the main thread), 
    or detach after a short sleep, if immediate is true, and we are
    in a worker thread */
    if (immediate)
#if defined(__MINGW32__) || defined(_MSC_VER)
        Sleep(100); // va: windows native
#else
        usleep(10000);
#endif
    /* status >= 1000 tells us that we react on command 'quit' 
      hand this information over to caller */
    if (status >= 1000) {
        coquit = TRUE;
        fprintf(stdout, "\nNote: 'quit' asks for detaching ngspice.dll.\n");
        status -= 1000;
    }
    else {
        coquit = FALSE;
        fprintf(stderr, "Error: ngspice.dll cannot recover and awaits to be detached\n");
    }
    ngexit(status, immediate, coquit, userptr);
    if (!intermj)
        longjmp(errbufm,1); /* jump back to ngSpice_Circ() */
    else
        longjmp(errbufc,1); /* jump back to ngSpice_Command() */
}

static int len = 0;
static pvecvaluesall curvecvalsall;

#ifdef olld
static pvecvalues* curvecvals;
static char type_name[128];
int sh_ExecutePerLoop_old(void)
{
    struct dvec *d;
    int i, veclen;
    struct plot *pl = plot_cur;
    /* return immediately if callback not wanted */
    if (nodatawanted)
        return 2;

    /* reset data structure if there is a change in plot type */
    if (strcmp(type_name, pl->pl_typename)) {

        if (curvecvals) {
            for (i = 0; i < len; i++)
                tfree(curvecvals[i]);
            tfree(curvecvals);
        }
        len = 0;
        bzero(type_name, 128); 
    }

    /* initialize new for every new plot, e.g. if changed from op1 to ac1
       or from tran1 to tran2 */
    if ((pl) && (len == 0)) {
        strcpy(type_name, pl->pl_typename);
        for (d = pl->pl_dvecs; d; d = d->v_next)
            len++;

        if (len == 0) {
            fprintf(cp_err, "Error: There are no vectors currently active.\n");
            return 1;
        }
        /* allocate memory for the number of vectors */        
        curvecvals = TMALLOC(pvecvalues, len);

        /* allocate memory for each entry and add vector names once */
        for (d = pl->pl_dvecs, i = 0; d; d = d->v_next, i++) {
            curvecvals[i] = TMALLOC(vecvalues, 1);
            curvecvals[i]->name = copy(d->v_name);
        }
    }
    /* get the data of the last entry to the plot vector */
    veclen = pl->pl_dvecs->v_length - 1;
    for (d = pl->pl_dvecs, i = 0; d; d = d->v_next, i++) {
        /* test if real */
        if (d->v_flags & VF_REAL) {
            curvecvals[i]->is_complex = FALSE;
            curvecvals[i]->creal = d->v_realdata[veclen];
            curvecvals[i]->cimag = 0.;
        }
        else {
            curvecvals[i]->is_complex = TRUE;
            curvecvals[i]->creal = d->v_compdata[veclen].cx_real;
            curvecvals[i]->cimag = d->v_compdata[veclen].cx_imag;
        }
    }
    /* now call the callback function to return the data to the caller */
    if (!nodatawanted)
  //      datfcn(curvecvals, len, userptr);

    return 0;
}
#endif

/* called each time a new data set is written to the output vectors */
int sh_ExecutePerLoop(void)
{
    struct dvec *d;
    int i, veclen;
    double testval;
    struct plot *pl = plot_cur;
    /* return immediately if callback not wanted */
    if (nodatawanted)
        return 2;

    /* get the data of the last entry to the plot vector */
    veclen = pl->pl_dvecs->v_length - 1;
    for (d = pl->pl_dvecs, i = 0; d; d = d->v_next, i++) {
        /* test if real */
        if (d->v_flags & VF_REAL) {
            curvecvalsall->vecsa[i]->is_complex = FALSE;
            testval =  d->v_realdata[veclen];
            curvecvalsall->vecsa[i]->creal = d->v_realdata[veclen];
            curvecvalsall->vecsa[i]->cimag = 0.;
        }
        else {
            curvecvalsall->vecsa[i]->is_complex = TRUE;
            curvecvalsall->vecsa[i]->creal = d->v_compdata[veclen].cx_real;
            curvecvalsall->vecsa[i]->cimag = d->v_compdata[veclen].cx_imag;
        }
    }
    /* now call the callback function to return the data to the caller */
    datfcn(curvecvalsall, len, userptr);

    return 0;
}


/* called once for a new plot from beginPlot() in outitf.c, 
   after the vectors in ngspice have been set.
   Transfers vector information to the caller via callback datinitfcn() 
   and sets transfer structure for use in sh_ExecutePerLoop() */
int sh_vecinit(runDesc *run)
{
    struct dvec *d, *ds;
    int veccount, i;
    static pvecinfoall pvca = NULL;
    pvecinfo *pvc;

    /* return immediately if callback not wanted */
    if (nodatainitwanted)
        return 2;

    cur_run = run;

    len = veccount = cur_run->numData;

    if (veccount == 0) {
        fprintf(cp_err, "Error: There are no vectors currently active.\n");
        return 1;
    }

    /* delete the structs from the previous plot */
    if (pvca) {
        for (i = 0; i < pvca->veccount; i++)
            tfree(pvca->vecs[i]);
        tfree(pvca->vecs);
        tfree(pvca);
        pvca = NULL;
    }
    
    pvc = TMALLOC(pvecinfo, veccount);
    ds = cur_run->runPlot->pl_scale;
    for (i = 0, d = cur_run->runPlot->pl_dvecs; i < veccount; i++, d = d->v_next) {
        pvc[i] = TMALLOC(vecinfo, 1);
        pvc[i]->number = i;
        pvc[i]->pdvec = (void*)d;
        pvc[i]->pdvecscale = (void*)ds;
        pvc[i]->vecname = d->v_name;
        pvc[i]->is_real = (d->v_flags & VF_REAL);
    }
    pvca = TMALLOC(vecinfoall, 1);
    // the plot
    pvca->title = cur_run->runPlot->pl_title;
    pvca->date = cur_run->runPlot->pl_date;
    pvca->name = cur_run->runPlot->pl_name;
    pvca->type = cur_run->runPlot->pl_typename;
    pvca->veccount = veccount;
    // the data
    pvca->vecs = pvc;
    /* now call the callback function to return the data to the caller */
    datinitfcn(pvca, userptr);

    /* generate the data tranfer structure, 
       data will be sent from sh_ExecutePerLoop() via datfcn() */
    if (!curvecvalsall) {
        curvecvalsall = TMALLOC(vecvaluesall, 1);
        curvecvalsall->veccount = veccount;
    }
    else {
        for (i = 0; i < curvecvalsall->veccount; i++)
            tfree(curvecvalsall->vecsa[i]);
        tfree(curvecvalsall->vecsa);
    }

    curvecvalsall->vecsa = TMALLOC(pvecvalues, veccount);

    for (i = 0, d = cur_run->runPlot->pl_dvecs; i < veccount; i++, d = d->v_next) {
        curvecvalsall->vecsa[i] = TMALLOC(vecvalues,1);
        curvecvalsall->vecsa[i]->name = d->v_name;
        if (cieq(d->v_plot->pl_scale->v_name, d->v_name))
            curvecvalsall->vecsa[i]->is_scale = TRUE;
        else
            curvecvalsall->vecsa[i]->is_scale = FALSE;
    }
    return 0;
}

