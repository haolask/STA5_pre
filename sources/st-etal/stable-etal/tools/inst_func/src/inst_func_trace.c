//!
//!  \file      inst_func_trace.c
//!  \brief     <i><b>Instrumentation calls for entry and exit to functions profiling with compilation switch -finstrument-functions.</b></i>
//!  \details   Instrumentation calls for entry and exit to functions profiling with compilation switch -finstrument-functions. Just after function entry and just before function exit, the following profiling functions will be called with the address of the current function and its call site.
//!  \author    David Pastor
//!

#include <stdio.h>
#include <time.h>

static FILE *fp_trace;
static struct timespec t_process;
#ifdef __FDPIC__
static FILE *fp_cmd_pipe;
static unsigned int relocation_r_xs_start_address, relocation_r_xs_stop_address;
#endif /* __FDPIC__ */

void __attribute__ ((constructor))trace_begin (void){
    fp_trace = fopen("trace.out", "w");

#ifdef __FDPIC__
    /* get executable relocated .text start address */
    if (fp_trace != NULL) {
        fp_cmd_pipe = popen("cat /proc/maps|grep /etal|grep r-xs|sed -e 's/\\([0-9a-f]*-[0-9a-f]*\\).*/\\1/'", "r");
        if (fp_cmd_pipe != NULL) {
            fscanf(fp_cmd_pipe, "%x-%x", &relocation_r_xs_start_address, &relocation_r_xs_stop_address);
            fprintf(fp_trace, "r 0x%x 0x%x %d %d\n", relocation_r_xs_start_address, relocation_r_xs_stop_address, 0, 0);
            pclose(fp_cmd_pipe);
        }
    }
#endif /* __FDPIC__ */
}

void __attribute__ ((destructor))trace_end (void){
    if(fp_trace != NULL) {
        fclose(fp_trace);
    }
}

void __cyg_profile_func_enter (void *func,  void *caller){
#ifdef __FDPIC__
    register void *lr asm("lr");
#endif /* __FDPIC__ */

    if(fp_trace != NULL) {
        fprintf(fp_trace, "e %p %p %ld %ld\n",
#ifdef __FDPIC__
                lr,
#else
                func,
#endif /* __FDPIC__ */
                caller, (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_process) == 0)?t_process.tv_nsec:0, t_process.tv_sec);
    }
}

void __cyg_profile_func_exit (void *func, void *caller){
#ifdef __FDPIC__
    register void *lr asm("lr");
#endif /* __FDPIC__ */

    if(fp_trace != NULL) {
        fprintf(fp_trace, "x %p %p %ld %ld\n",
#ifdef __FDPIC__
                lr,
#else
                func,
#endif /* __FDPIC__ */
                caller, (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_process) == 0)?t_process.tv_nsec:0, t_process.tv_sec);
    }
}

