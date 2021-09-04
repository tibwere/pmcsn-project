#pragma once

#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Uncomment the following line to enable debug prints to verify the system */
//#define VERIFY
/* Uncomment the following line to enable stationary simulation */
//#define STATIONARY
/* Uncomment the following line to analyze growing stats */
//#define STOP_BATCH (64)

#define START 0.0                       /* Initial time */
#ifdef STATIONARY
    #define B (960.0)                   /* Length of a single batch */
    #define K (64)                      /* Number of batches */
    #define STOP (B * K)                /* Conceptual infinity time for the end of the simulation */
#else
    #define STOP (480.0)                /* Terminal ("close the door") time */
    #define ENSEMBLE_SIZE 300           /* Number of simulation replies */                       
#endif

#define TERM_INIT_SEED 9				/* Initial seed for terminating simulation */
#define STAT_INIT_SEED 12345			/* Initial seed for steady-state simulation */

#define FLUSH                           /* Flush the system after close the door */

#define INFTY (100.0 * STOP)            /* Impossible occurrence of an event (must be much larger than STOP) */

#define UNICA_OP_BP_ARR_STREAM 0        /* Stream for "Unica Operazione Banco Posta" [ARRIVALS] */
#define PAGAM_PREL_BP_ARR_STREAM 1      /* Stream for "Pagamenti & Prelievi Banco Posta" [ARRIVALS] */
#define UNICA_OP_STD_ARR_STREAM 2       /* Stream for "Unica Operazione Standard" [ARRIVALS] */
#define PAGAM_PREL_STD_ARR_STREAM 3     /* Stream for "Pagamenti & Prelievi Standard" [ARRIVALS] */ 
#define SPED_RIT_BP_ARR_STREAM 4        /* Stream for "Spedizioni & Ritiri Banco Posta" [ARRIVALS] */
#define SPED_RIT_STD_ARR_STREAM 5       /* Stream for "Spedizioni & Ritiri Standard" [ARRIVALS] */

#define UNICA_OP_SERV_STREAM 6          /* Stream for "Unica Operazione" [SERVICES] */
#define PAGAM_PREL_SERV_STREAM 7        /* Stream for "Pagamenti & Prelievi" [SERVICES] */
#define SPED_RIT_SERV_STREAM 8          /* Stream for "Spedizioni & Ritiri" [SERVICES] */

#define M 3
#define NUMBER_OF_QUEUES 6              /* Number of queues (UNICA_OP + PAGAM_PREL + SPED_RIT) */
#define NUMBER_OF_GP_QUEUES 4           /* Number of queues (without SPED_RIT) */

#define P_BP 0.25                       /* Probability of taking a ticket "BancoPosta" */

#define P_UO 0.5                        /* Probability of taking a ticket "Unica Operazione" */
#define P_PP 0.35                       /* Probability of taking a ticket "Pagamenti & Prelievi" */
#define P_SR 0.15                       /* Probability of taking a ticket "Spedizioni & Ritiri" */

#define LAMBDA (3125.0 / 12812)         /* Average arrival rate */

#define MU_UO (1.0 / 7)              /* Average service rate for "Unica Operazione" */
#define MU_PP (1.0 / 14)              /* Average service rate for "Pagamenti & Prelievi" */
#define MU_SR (1.0 / 10)                /* Average service rate for "Spedizioni & Ritiri" */

#define IDLE 0                          /* The server is idle */
#define BUSY 1                          /* The server is busy */

#define NONE -1                         /* The server is not processing a ticket */
#define UO_BP 0                         /* The server is processing a ticket "Unica Operazione BancoPosta" */
#define PP_BP 1                         /* The server is processing a ticket "Pagamenti & Prelievi BancoPosta" */
#define UO_STD 2                        /* The server is processing a ticket "Unica Operazione Standard" */
#define PP_STD 3                        /* The server is processing a ticket "Pagamenti & Prelievi Standard" */
#define SR_BP 4                         /* The server is processing a ticket "Spedizioni & Ritiri BancoPosta" */
#define SR_STD 5                        /* The server is processing a ticket "Spedizioni & Ritiri Standard" */

#define ARR_EVENT_TYPE 0                /* Next event is an arrival */
#define GEN_COMPL_EVENT_TYPE 1          /* Next event is a completion of a general server */
#define DED_COMPL_EVENT_TYPE 2          /* Next event is a completion of a dedicated server */

typedef struct compl_event {
    double time;
    int user_class;
} compl_event_t;

typedef struct event_list {
    double arrivals[NUMBER_OF_QUEUES];  /* Next occurrence of an arrival for each flow */
    compl_event_t *gen_completions[M-1];/* Next occurrence of a completion for each server */       
    compl_event_t *ded_completion;      /* Next occurrence of a completion for the dedicated server */            
} event_list_t; 

typedef struct times {
    double next;                        /* Occurrence of the next event */                                         
    double current;                     /* System clock */
    double last[NUMBER_OF_QUEUES];      /* Last arrival time for each flow */ 
} times_t;

typedef struct time_integrated_populations {
    double customers[NUMBER_OF_QUEUES]; /* "Area" of customers [unita' di misura: (total customers) * time] */ 
    double queue[NUMBER_OF_QUEUES];     /* "Area" of queues [unita' di misura: (customers in queue) * time] */       
    double service[NUMBER_OF_QUEUES];   /* "Area" of service [unita' di misura: (customers in service) * time] */
} time_integrated_populations_t;

typedef struct statistics {
    double r[NUMBER_OF_QUEUES];         /* Interarrival times */
    double w[NUMBER_OF_QUEUES];         /* Waits */
    double d[NUMBER_OF_QUEUES];         /* Delays */
    double s[NUMBER_OF_QUEUES];         /* Services */
    double l[NUMBER_OF_QUEUES];         /* Requests in the node */
    double q[NUMBER_OF_QUEUES];         /* Requests in the queues */ 
    double y[NUMBER_OF_QUEUES];         /* Requests in service */
} statistics_t;


/* System Status */
int customers[NUMBER_OF_QUEUES];
int in_service[NUMBER_OF_QUEUES];
int gen_status[M-1];
int ded_status;

/* Event List */
event_list_t *events;

/* Simulation Clock */
times_t *t;

#ifdef STATIONARY
int batch_index;                        /* Current number of batch */
#endif

/* Prototypes */
int                             integers_sum(int *, int);
double                          doubles_sum(double *, int);
double                          next_arr_event(double *, int, int *);
double                          next_compl_event(compl_event_t **, int, int *);
double                          next_event(int *, int *);
void                            GetArrival(int);
double                          GetService(int);
time_integrated_populations_t * init_tip(void);
void                            init_event_list(void);
void                            init_times(void);
void                            init_status(void);
void                            update_tip(time_integrated_populations_t *);
int                             get_idle_server_gp(void);
statistics_t *                  load_statistics(time_integrated_populations_t *, int *);
int                             has_to_continue(int *);
int                             get_max_prio_queue_not_empty(void);
void                            toggle_server_status(int);
void                            next_assignment_ded_server(void);
#endif
