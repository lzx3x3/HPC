#if !defined(TICTOC_H)
#define      TICTOC_H

#ifdef __cplusplus
extern "C"{
#endif

/* feature test macro for proper c99 behavior */
#if __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500
#endif /* __STDC_VERSION__ */

// ---------
// Profiling
// ---------
#include <stdint.h>

typedef struct _tic_toc_timer
{ uint64_t last; ///< last observation on the counter
  uint64_t rate; ///< counts per second
} TicTocTimer;

static TicTocTimer tic(void);              ///< Intitializes a timer.
static double      toc(TicTocTimer *last); ///< Updates timer. \param[in,out] if \a last is NULL, a global timer is used. \returns time since last in seconds.

#define error(...) {fprintf(stderr,__VA_ARGS__); exit(-1); }

#ifdef DEBUG
#define DEBUG_TIC_TOC_TIMER
#define debug(...) printf(__VA_ARGS__)
#include <stdio.h>
#else
#define debug(...)
#endif

#if !defined(__APPLE__) && !defined(_MSC_VER)
#define TICTOC_HAVE_POSIX_TIMER
#include <time.h>
#ifdef CLOCK_MONOTONIC
#define TICTOC_CLOCKID CLOCK_MONOTONIC
#else
#define TICTOC_CLOCKID CLOCK_REALTIME
#endif
#endif

#ifdef __APPLE__
#define TICTOC_HAVE_MACH_TIMER
#include <mach/mach_time.h>
#endif

#ifdef _MSC_VER
#define TICTOC_HAVE_WIN32_TIMER
#include <windows.h>
#include <Strsafe.h>
#define tictoc_Guarded_Assert_WinErr(expression) \
  if(!(expression))\
  { tictoc_ReportLastWindowsError();\
    error("Windows call failed: %s\n\tIn %s (line: %u)\n", #expression, __FILE__ , __LINE__ );\
  }
static void tictoc_ReportLastWindowsError(void)
{ //EnterCriticalSection( _get_reporting_critical_section() );
  { // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
        (lstrlen((LPCTSTR)lpMsgBuf) + 40) * sizeof(TCHAR));
    StringCchPrintf((LPTSTR)lpDisplayBuf,
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("Failed with error %d: %s"),
        dw, lpMsgBuf);

    // spam formated string to listeners
    {
      fprintf(stderr,"%s",lpDisplayBuf);
    }

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
  }
  //LeaveCriticalSection( _get_reporting_critical_section() );
}
#endif

TicTocTimer tictoc_g_last;

static TicTocTimer tic(void)
{ TicTocTimer t = {0,0};
#ifdef TICTOC_HAVE_POSIX_TIMER
  struct timespec rate,time;
  clock_getres(TICTOC_CLOCKID,&rate);
  clock_gettime(TICTOC_CLOCKID,&time);
  t.rate = 1000000000LL*rate.tv_nsec; // in Hz
  t.last = time.tv_sec*1000000000LL+time.tv_nsec;
#endif
#ifdef TICTOC_HAVE_MACH_TIMER
  mach_timebase_info_data_t rate_nsec;
  mach_timebase_info(&rate_nsec);
  t.rate = 1000000000LL * rate_nsec.numer / rate_nsec.denom;
  t.last = mach_absolute_time();
#endif
#ifdef TICTOC_HAVE_WIN32_TIMER
  { LARGE_INTEGER tmp;

    tictoc_Guarded_Assert_WinErr( QueryPerformanceFrequency( &tmp ) );
    t.rate = tmp.QuadPart;
    tictoc_Guarded_Assert_WinErr( QueryPerformanceCounter  ( &tmp ) );
    t.last = tmp.QuadPart;
  }
#endif

#ifdef DEBUG_TIC_TOC_TIMER
  //Guarded_Assert( t.rate > 0 );
  debug("Tic() timer frequency: %llu Hz\r\n"
        "           resolution: %g ns\r\n"
        "               counts: %llu\r\n",t.rate,
                                        1e9/(double)t.rate,
                                        t.last);
#endif
  tictoc_g_last = t;
  return t;
}

// - returns time since last in seconds
// - updates timer
static double toc(TicTocTimer *t)
{ uint64_t now;
  double delta;
  if(!t) t=&tictoc_g_last;
#ifdef TICTOC_HAVE_POSIX_TIMER
  { struct timespec time;
    clock_gettime(TICTOC_CLOCKID,&time);
    now = time.tv_sec*1000000000LL+time.tv_nsec;
  }
#endif
#ifdef TICTOC_HAVE_MACH_TIMER
  now = mach_absolute_time();
#endif
#ifdef TICTOC_HAVE_WIN32_TIMER
  { LARGE_INTEGER tmp;
    tictoc_Guarded_Assert_WinErr( QueryPerformanceCounter( &tmp ) );
    now = tmp.QuadPart;
  }
#endif
  delta = ( now - t->last) / (double)t->rate;
  t->last = now;
  return delta;
}


#ifdef __cplusplus
}
#endif
#endif
