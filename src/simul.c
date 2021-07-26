/*
 * Simulator of "Poste Italiane" system
 * 
 * authors: A. Chillotti    (M. 0299824)
 *          C. Cuffaro      (M. 0299838)      
 *          S. Tiberi       (M. 0299908)
 * 
 * A.Y. 2020/2021
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Simulation libraries */
#include "rngs.h"                       /* Multi-stream generator */
#include "rvgs.h"                       /* Random variate generators */

/* Uncomment the following line to enable debug prints to verify the system */
//#define VERIFY
/* Uncomment the following line to enable stationary simulation */
//#define STATIONARY
//#define STOP_BATCH (256)

#define START 0.0                       /* Initial time */
#ifdef STATIONARY
    #define B (960.0)                     /* Length of a single batch */
    #define K (64)                        /* Number of batches */
    #define STOP (B * K)                /* Conceptual infinity time for the end of the simulation */
#else
    #define STOP (480.0)                /* Terminal ("close the door") time */
    #define ENSEMBLE_SIZE 300           /* Number of simulation replies */                       
#endif

#define TERM_INIT_SEED 9
#define STAT_INIT_SEED 12345

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

#define M 4
#define NUMBER_OF_QUEUES 6              /* Number of queues (UNICA_OP + PAGAM_PREL + SPED_RIT) */
#define NUMBER_OF_GP_QUEUES 4           /* Number of queues (only SPED_RIT) */

#define P_BP 0.25                       /* Probability of taking a ticket "BancoPosta" */

#define P_UO 0.5                        /* Probability of taking a ticket "Unica Operazione" */
#define P_PP 0.35                       /* Probability of taking a ticket "Pagamenti & Prelievi" */
#define P_SR 0.15                       /* Probability of taking a ticket "Spedizioni & Ritiri" */

#define LAMBDA (3125.0 / 12812)         /* Average arrival rate */

#define MU_UO (41.0 / 510)              /* Average service rate for "Unica Operazione" */
#define MU_PP (41.0 / 765)              /* Average service rate for "Pagamenti & Prelievi" */
#define MU_SR (1.0 / 15)                /* Average service rate for "Spedizioni & Ritiri" */

#define IDLE -1                         /* The server is idle */
#define UO_BP 0                         /* The server is processing a ticket "Unica Operazione BancoPosta" */
#define PP_BP 1                         /* The server is processing a ticket "Pagamenti & Prelievi BancoPosta" */
#define UO_STD 2                        /* The server is processing a ticket "Unica Operazione Standard" */
#define PP_STD 3                        /* The server is processing a ticket "Pagamenti & Prelievi Standard" */
#define SR_BP 4                         /* The server is processing a ticket "Spedizioni & Ritiri BancoPosta" */
#define SR_STD 5                        /* The server is processing a ticket "Spedizioni & Ritiri Standard" */

#define ARR_EVENT_TYPE 0                /* Next event is an arrival */
#define GEN_COMPL_EVENT_TYPE 1          /* Next event is a completion of a general server */
#define DED_COMPL_EVENT_TYPE 2          /* Next event is a completion of a dedicated server */


typedef struct event_list {
    double arrivals[NUMBER_OF_QUEUES];  /* Next occurrence of an arrival for each flow */
    double gen_completions[M-1];        /* Next occurrence of a completion for each server */       
    double ded_completion;              /* Next occurrence of a completion for the dedicated server */            
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
    double n[NUMBER_OF_QUEUES];         /* Requests in service */
} statistics_t;

/* System Status */
int customers[NUMBER_OF_QUEUES];
int gen_status[M-1];
int ded_status;

/* Event List */
event_list_t *events;

/* Simulation Clock */
times_t *t;

#ifdef STATIONARY
int batch_index = 0;                    /* Current number of batch */
#endif

/* Prototypes */
int                             integers_sum(int *, int);
double                          doubles_sum(double *, int);
double                          min_from_array(double *, int, int *);
double                          next_event(int *, int *);
void                            GetArrival(int);
double                          GetService(int);
time_integrated_populations_t * init_tip(void);
void                            init_event_list(void);
void                            init_times(void);
int *                           get_in_service_per_type(void);
void                            update_tip(time_integrated_populations_t *);
int                             get_max_prio_queue_not_empty(void);
int                             get_idle_server_gp(void);
void                            toggle_server_status(int);
void                            next_assignment_ded_server(void);
void                            print_update(int, int, int);
void                            print_report(int *, time_integrated_populations_t *);
statistics_t *                  load_statistics(time_integrated_populations_t *, int *);
int                             has_to_continue(int *);
statistics_t *                  simulation_run(void);


/*
 * Sum of the elements belonging to an array of integers
 * 
 * @param items:
 *      Array of integers
 * @param len: 
 *      Array length
 * @return:
 *      The sum of the elements in the array
 */
int integers_sum(int *items, int len) 
{
    int sum = 0;

    for (int i = 0; i < len; ++i)
        sum += items[i];
    
    return (sum);
}

/*
 * Sum of the elements belonging to an array of doubles
 * 
 * @param items:
 *      Array of doubles
 * @param len: 
 *      Array length
 * @return:
 *      The sum of the elements in the array
 */
double doubles_sum(double *items, int len) 
{
    double sum = 0;

    for (int i = 0; i < len; ++i)
        sum += items[i];
    
    return (sum);
}

/*
 * Minimum element from an array of doubles
 * 
 * @param items:
 *      Array of doubles
 * @param len: 
 *      Array length
 * @param *index:
 *      Pointer to save index of the minimum
 * @return:
 *      The minimum element of the array
 */
double min_from_array(double *items, int len, int *index)
{
    int min_index = 0;
    double min_val = items[0];

    for (int i = 1; i < len; ++i) {
        if (items[i] < min_val) {
            min_val = items[i];
            min_index = i;
        }
    }

    *index = min_index;
    return (min_val);
}

/*
 * Occurrence of the next event
 * 
 * @param *min_event_type:
 *      Pointer to save the type of the next event
 * @param *min_index: 
 *      Pointer to save the array index in the events list of the next event
 * @return:
 *      Time occurence of the next event
 */
double next_event(int *min_event_type, int *min_index)
{
    int min_arrival_index, min_gen_compl_index;
    double min_arrival, min_gen_compl;

    min_arrival = min_from_array(events->arrivals, NUMBER_OF_QUEUES, &min_arrival_index);
    min_gen_compl = min_from_array(events->gen_completions, M-1, &min_gen_compl_index);

    if (min_arrival < min_gen_compl) {                  
        if (min_arrival < events->ded_completion) {     /* Next event is an arrival */
            *min_event_type = ARR_EVENT_TYPE;
            *min_index = min_arrival_index;
            return min_arrival;
        } else {                                        /* Next event is an completion of the dedicated server */
            *min_event_type = DED_COMPL_EVENT_TYPE;
            *min_index = 0;
            return events->ded_completion;
        }
    } else {
        if (min_gen_compl < events->ded_completion) {   /* Next event is an completion of a general server */
            *min_event_type = GEN_COMPL_EVENT_TYPE;
            *min_index = min_gen_compl_index;
            return min_gen_compl;
        } else {                                        /* Next event is an completion of the dedicated server */
            *min_event_type = DED_COMPL_EVENT_TYPE;     
            *min_index = 0;
            return events->ded_completion;
        }        
    }
}

/*
 * Generate next arrival of flow i 
 * 
 * @param type:
 *      i-th flow
 */
void GetArrival(int type)
{
    switch (type)
    {
    case UO_BP:
        SelectStream(UNICA_OP_BP_ARR_STREAM);
        events->arrivals[type] += Exponential(1 / (P_UO * P_BP * LAMBDA));
        break;
    case UO_STD:
        SelectStream(UNICA_OP_STD_ARR_STREAM);
        events->arrivals[type] += Exponential(1 / (P_UO * (1 - P_BP) * LAMBDA));
        break;
    case PP_BP:
        SelectStream(PAGAM_PREL_BP_ARR_STREAM);
        events->arrivals[type] += Exponential(1 / (P_PP * P_BP * LAMBDA));
        break;
    case PP_STD:
        SelectStream(PAGAM_PREL_STD_ARR_STREAM);
        events->arrivals[type] += Exponential(1 / (P_PP * (1 - P_BP) * LAMBDA));
        break;
    case SR_BP:
        SelectStream(SPED_RIT_BP_ARR_STREAM);
        events->arrivals[type] += Exponential(1 / (P_SR * P_BP * LAMBDA));
        break;
    case SR_STD:
        SelectStream(SPED_RIT_STD_ARR_STREAM);
        events->arrivals[type] += Exponential(1 / (P_SR * (1 - P_BP) * LAMBDA));
        break;
    default:
        abort();
    }
} 

/*
 * Next service time of type i 
 * 
 * @param type:
 *      i-th type
 * @return:
 *      Service time for a ticket of type i
 */
double GetService(int type) 
{
    switch(type) {
    case UO_BP:
    case UO_STD:
        SelectStream(UNICA_OP_SERV_STREAM);
        return (Exponential(1 / MU_UO));
    case PP_BP:
    case PP_STD:
        SelectStream(PAGAM_PREL_SERV_STREAM);
        return (Exponential(1 / MU_PP));
    case SR_BP:
    case SR_STD:
        SelectStream(SPED_RIT_SERV_STREAM);
        return (Exponential(1 / MU_SR));
    default: 
        abort();
    }
}

/*
 * Initialize time_integrated_populations structure
 *
 * @return:
 *      Pointer to a structure
 */
time_integrated_populations_t *init_tip(void)
{
    time_integrated_populations_t *p = malloc(sizeof(time_integrated_populations_t));
    if (p == NULL)
        abort();

    memset(p, 0x0, sizeof(time_integrated_populations_t));

    return (p);
}

/*
 * Initialize system status
 */
void init_status(void)
{
    int i;

    for(i = 0; i < NUMBER_OF_QUEUES; ++i)
        customers[i] = 0;
    for (i = 0; i < (M - 1); ++i)
        gen_status[i] = IDLE;
        
    ded_status = IDLE;

    /* Uncomment these lines to start with not empty center */
    // int initial_customers[NUMBER_OF_QUEUES] = {1, 1, 2, 3, 1, 2};
    // for (i = 0; i < NUMBER_OF_QUEUES; ++i)
    //     customers[i] = initial_customers[i];
}

/*
 * Initialize event list
 */
void init_event_list(void)
{

    int i;
    events = malloc(sizeof(event_list_t));
    if (events == NULL)
        abort();

    memset(events, 0x0, sizeof(event_list_t));

    /* Uncomment these lines if you want to start with an empty center 
     * (Standard behaviuour - should be uncommented)
     */
    for (i = 0; i < NUMBER_OF_QUEUES; ++i) 
        GetArrival(i);

    /* Uncomment these lines in validation phase (sec. 7.2) */
    // for (int i = NUMBER_OF_GP_QUEUES; i < NUMBER_OF_QUEUES; ++i)
    //     events->arrivals[i] = INFTY;

    /* Uncomment these lines in validation phase (sec. 7.3) */
    // for (int i = 0; i < NUMBER_OF_GP_QUEUES; ++i)
    //     events->arrivals[i] = INFTY;

    for (i = 0; i < M - 1; ++i) 
        events->gen_completions[i] = INFTY;
    
    events->ded_completion = INFTY;
}

/*
 * Initialize times structure
 */
void init_times(void) 
{
    t = malloc(sizeof(times_t));
    if (t == NULL)
        abort();
    memset(t, 0x0, sizeof(times_t));

    t->current = START;
}

/*
 * Number of ticket in service for each flow
 *
 * @return:
 *      Array of integer that contains those numbers
 */
int *get_in_service_per_type(void)
{
    int *in_service_per_type;
    in_service_per_type = calloc(NUMBER_OF_QUEUES, sizeof(int));
    if (in_service_per_type == NULL)
        abort();

    for (int i = 0; i < M-1; ++i) {
        if(gen_status[i] != IDLE)
            in_service_per_type[gen_status[i]]++;
    }
    if (ded_status != IDLE)
        in_service_per_type[ded_status]++;

    return (in_service_per_type);
}

/*
 * Update time integrated populations
 *
 * @param *area:
 *      Pointer to a structure time_integrated_populations
 */
void update_tip(time_integrated_populations_t *area)
{
    int *in_service_per_type = get_in_service_per_type();

    for(int i = 0; i < NUMBER_OF_QUEUES; ++i) {
        if (customers[i] > 0) {
            area->customers[i] += (t->next - t->current) * customers[i];
            area->queue[i]   += (t->next - t->current) * (customers[i] - in_service_per_type[i]);
            area->service[i] += (t->next - t->current) * in_service_per_type[i];
        }
    }
}

/*
 * Index of a priority queue not empty in the system
 *
 * @return:
 *      Index of the queue (customers array) if exists, otherwise -1
 */
int get_max_prio_queue_not_empty(void)
{
    int prio_queue = 0;
    int *in_service_per_type = get_in_service_per_type();

    while (prio_queue < NUMBER_OF_GP_QUEUES) {
        if (customers[prio_queue] > in_service_per_type[prio_queue])
            break;
        ++prio_queue;
    }

    return ((prio_queue == NUMBER_OF_GP_QUEUES) ? -1 : prio_queue);
}

/*
 * Index of first idle general server in the system
 *
 * @return:
 *      Index of the server
 */
int get_idle_server_gp(void)
{
    for (int i = 0; i < M-1; ++i) {
        if (gen_status[i] == IDLE)
            return (i);
    }
    return (-1);
}

/*
 * Assign a job to a server
 *
 * @param index;
 *      Server index to assign job to
 */
void toggle_server_status(int index)
{
    int max_prio_queue_not_empty = get_max_prio_queue_not_empty();
    if (max_prio_queue_not_empty != -1) {                                                           /* There is a queue not empty */
        if (index != -1) {                                                                          /* The server is general */
            gen_status[index] = max_prio_queue_not_empty;
            events->gen_completions[index] = t->current + GetService(max_prio_queue_not_empty);
        } else {                                                                                    /* The server is dedicated */
            ded_status = max_prio_queue_not_empty;
            events->ded_completion = t->current + GetService(max_prio_queue_not_empty);
        }
    } else {                                                                                        /* There isn't a queue not empty */
        if (index != -1) {                                                                          /* The server is general */
            gen_status[index] = IDLE;
            events->gen_completions[index] = INFTY;
        } else {                                                                                    /* The server is dedicated */
            ded_status = IDLE;
            events->ded_completion = INFTY;
        }
    }
}

/*
 * Assign a job (Spedizioni & Ritiri [BancoPosta/Standard]) to the dedicated server
 */
void next_assignment_ded_server(void)
{
    int *in_service = get_in_service_per_type();
    if (customers[SR_BP] > in_service[SR_BP]) {
        ded_status = SR_BP;
        events->ded_completion = t->current + GetService(SR_BP);
    } else if (customers[SR_STD] > in_service[SR_STD]) {
        ded_status = SR_STD;
        events->ded_completion = t->current + GetService(SR_STD);
    } else {
        toggle_server_status(-1);
    }
}

/*
 * Print an update at each iteration of the simulation run (VERIFY MODE only)
 * 
 * @param event_type:
 *      Type of the generated event
 * @param event_index:
 *      Index of array dependant to specific event type
 * @param service_completed:
 *      Type of last completed service (event_type != ARRIVALS)
 */
void print_update(int event_type, int event_index, int service_completed)
{
    int dummy;
    int j;
    char *tmp;

    char *types_of_tickets[NUMBER_OF_QUEUES] = {
        "\'Unica Operazione BancoPosta\'", "\'Pagamenti & Prelievi BancoPosta\'", "\'Unica Operazione Standard\'", 
        "\'Pagamenti & Prelievi Standard\'", "\'Spedizioni & Ritiri BancoPosta\'", "\'Spedizioni & Ritiri Standard\'"
    };

    double min_arrival = min_from_array(events->arrivals, NUMBER_OF_QUEUES, &dummy);
    double min_completion = min_from_array(events->gen_completions, M-1, &dummy);

    if (event_type == ARR_EVENT_TYPE) {
        printf("Current event: ARRIVAL %s\n\n\n", types_of_tickets[event_index]);
    } else if (event_type == GEN_COMPL_EVENT_TYPE) {
        printf("Current event: COMPLETION OF A GEN. SERVER (id: %d)\n\tservice completed = %s\n\n", 
            event_index, types_of_tickets[service_completed]);
    } else {
        printf("Current event: COMPLETION OF A DED. SERVER\n\tservice completed = %s\n\n", 
            types_of_tickets[service_completed]);      
    }

    printf("Current time                        = %12.6lf\n", t->current);
    printf("Next arrival time                   = %12.6lf%s\n", 
        min_arrival, 
        (min_arrival == INFTY) ? " (INFTY)"  : "");
    printf("Next completion of general server   = %12.6lf%s\n", 
        min_completion, 
        ((min_completion == INFTY) ? " (INFTY)"  : "")
    );
    printf("Next completion of dedicated server = %12.6lf%s\n\n",
        events->ded_completion,
        ((events->ded_completion == INFTY) ? " (INFTY)" : "")
    );

    for (j = 0; j < NUMBER_OF_QUEUES; j++)
        printf("Customers of %-33s = %3d\n", types_of_tickets[j], customers[j]);

    printf("\nTotal customers                                = %3d\n\n", integers_sum(customers, NUMBER_OF_QUEUES));

    for (j = 0; j < M-1; j++) {
        tmp = (gen_status[j] == -1) ? "IDLE" : types_of_tickets[gen_status[j]];
        printf("General server %d:\n\tStatus          = %s \n", j, tmp);
        printf("\tNext completion = %lf\n\n", events->gen_completions[j]);
    }

    tmp = (ded_status == -1) ? "IDLE" : types_of_tickets[ded_status];
    printf("Dedicated server:\n\tStatus          = %s \n", tmp);
    printf("\tNext completion = %lf\n\n", events->ded_completion);

    printf("===========================================================\n");
}

/*
 * Print a report at the end of the simulation run
 *
 * @param *number_of_completions:
 *      Array of completions
 * @param *area:
 *      Pointer to time_integrated_populations structure
 */
void print_report(int *number_of_completions, time_integrated_populations_t *area) 
{
    int tot_completions = integers_sum(number_of_completions, NUMBER_OF_QUEUES);
    double area_customers = doubles_sum(area->customers, NUMBER_OF_QUEUES);
    double area_queue = doubles_sum(area->queue, NUMBER_OF_QUEUES);
    double area_service = doubles_sum(area->service, NUMBER_OF_QUEUES);
    char *types_of_service[NUMBER_OF_QUEUES] = {
        "UNICA_OP_BP", "PAGAM_PREL_BP", "UNICA_OP_STD", 
        "PAGAM_PREL_STD", "SPED_RIT_BP", "SPED_RIT_STD"
    };
        
    printf("\n+--------------------+\n| SIMULATION RESULTS |\n+--------------------+\n");

    for(int j = 0; j < NUMBER_OF_QUEUES; j++) 
        printf("Completions of %-15s = %3d\n", types_of_service[j], number_of_completions[j]);
    
    printf("\nTotal completions              = %3d\n\n", tot_completions);

    // printf("Average interarrival time      = %10.6f\n", t->last / tot_completions);
    printf("Average wait                   = %10.6f\n", area_customers / tot_completions);
    printf("Average delay                  = %10.6f\n", area_queue / tot_completions);
    printf("Average service time           = %10.6f\n", area_service / tot_completions);
    printf("Average # in the node          = %10.6f\n", area_customers / t->current);
    printf("Average # in the queue         = %10.6f\n", area_queue / t->current);
    printf("Average # in service           = %10.6f\n", area_service / t->current);
    printf("Utilization                    = %10.6f\n", (area_service / M) / t->current);

    printf("\n");
    for (int j = 0; j < NUMBER_OF_QUEUES; j++) {
        printf("Average interarrival time %s = %lf\n", types_of_service[j], t->last[j] / number_of_completions[j]);
        printf("Average delay %s = %lf\n\n", types_of_service[j], area->queue[j] / number_of_completions[j]);
    }
}

/*
 * Evaluate statics from a single simulation run
 *
 * @param *area:
 *      Pointer to time_integrated_populations structure
 * @param *number_of_completions:
 *      Array of completions
 * @return
 *      Pointer to a statistics structure
 */
statistics_t *load_statistics(time_integrated_populations_t *area, int *number_of_completions) 
{
    statistics_t *stat;
    
    stat = malloc(sizeof(statistics_t));
    if (stat == NULL)
        abort();
    memset(stat, 0x0, sizeof(statistics_t));

    for (int i = 0; i < NUMBER_OF_QUEUES; ++i) {
        if (number_of_completions[i] != 0) {
            stat->r[i] = t->last[i] / number_of_completions[i];
            stat->w[i] = area->customers[i] / number_of_completions[i];
            stat->d[i] = area->queue[i] / number_of_completions[i];
            stat->s[i] = area->service[i] / number_of_completions[i];
        }
#ifdef STATIONARY
        stat->l[i] = area->customers[i] / (t->current - ((batch_index - 1) * B));
        stat->q[i] = area->queue[i] / (t->current - ((batch_index - 1) * B));
        stat->n[i] = area->service[i] / (t->current - ((batch_index - 1) * B));
#else
        stat->l[i] = area->customers[i] / t->current;
        stat->q[i] = area->queue[i] / t->current;
        stat->n[i] = area->service[i] / t->current;
#endif
    }

    return (stat);
}

/*
 * Check if there is a flow that has not yet passed stop
 * 
 * @param flags:
 *      Boolean mask used to store the result of condition (!events->arrivals[next_event_index] > STOP)
 * @return
 *      * 1 if exists at least an entry of flags that is 1
 *      * 0 otherwise
 */
int has_to_continue(int *flags)
{
    int result = flags[0];
    for (int i = 1; i < NUMBER_OF_QUEUES; ++i) 
        result = result || flags[i];
    
    return (result);
}

/*
 * Executes a single run of a simulation
 *
 * @return:
 *      Pointer to statistics collected 
 */
statistics_t *simulation_run(void) 
{
    int number_of_completions[NUMBER_OF_QUEUES];
    time_integrated_populations_t *area;
    int idle_server_index;
    int continue_simul[NUMBER_OF_QUEUES] = {[0 ... NUMBER_OF_QUEUES - 1] = 1};
    int next_event_type;
    int next_event_index;
    int current_state;
#ifdef STATIONARY
    statistics_t *stat;
#endif
        
    memset(number_of_completions, 0x0, NUMBER_OF_QUEUES * sizeof(int));
    init_times();
    init_event_list();
    init_status();
    area = init_tip();

    /* Uncomment these lines in validation phase (sec. 7.2) */
    // for (int i = NUMBER_OF_GP_QUEUES; i < NUMBER_OF_QUEUES; ++i)
    //     continue_simul[i] = 0;

    /* Uncomment these lines in validation phase (sec. 7.3) */
    // for (int i = 0; i < NUMBER_OF_GP_QUEUES; ++i)
    //     continue_simul[i] = 0;

#ifdef STATIONARY   
    while (has_to_continue(continue_simul)) { 

        double p0 = (P_BP * P_UO)/(P_UO + P_PP);
        double p1 = (P_BP * P_PP)/(P_UO + P_PP);
        double p2 = ((1-P_BP) * P_UO)/(P_UO + P_PP);
        double p3 = ((1-P_BP) * P_PP)/(P_UO + P_PP);

        if (t->current > B * (batch_index + 1)) {
            batch_index++;
            stat = load_statistics(area, number_of_completions);
            //printf("%d,%lf\n", batch_index, stat->d[3]);
            printf("%lf\n", (p0*stat->d[0] + p1*stat->d[1] + p2*stat->d[2] + p3*stat->d[3]));
            //printf("%lf\n", (p0*stat->w[0] + p1*stat->w[1] + p2*stat->w[2] + p3*stat->w[3]));
            //printf("%lf\n", (stat->n[0] + stat->n[1] + stat->n[2] + stat->n[3]) / M);
            //printf("%lf\n", P_BP*stat->d[4] + (1-P_BP)*stat->d[5]);

            /* Uncomment these lines to plot stationary delay */
            // if (STOP_BATCH == 1) 
            //     printf("%lf\n", (p0*stat->d[0] + p1*stat->d[1] + p2*stat->d[2] + p3*stat->d[3]));
            //     //printf("%lf\n", P_BP*stat->d[4] + (1-P_BP)*stat->d[5]);

            free(stat);
            memset(number_of_completions, 0x0, NUMBER_OF_QUEUES * sizeof(int));
            free(area);
            area = init_tip();
            update_tip(area);

            if (batch_index == STOP_BATCH)
                break;            
        }
#else
	while (has_to_continue(continue_simul) || (integers_sum(customers, NUMBER_OF_QUEUES) > 0)) {
#endif
        t->next = next_event(&next_event_type, &next_event_index);
        
#ifndef FLUSH
        if (t->next > STOP)
            break;
#endif
        update_tip(area);
        t->current = t->next;

        if (next_event_type == ARR_EVENT_TYPE) {                       /* ARRIVO */
            customers[next_event_index]++;

            GetArrival(next_event_index);
            
            if (events->arrivals[next_event_index] > STOP) {
                continue_simul[next_event_index] = 0;
                events->arrivals[next_event_index] = INFTY;
                t->last[next_event_index] = t->current;
            }

            if ((next_event_index != SR_BP) && (next_event_index != SR_STD)) {
                if ((idle_server_index = get_idle_server_gp()) != -1 || ded_status == IDLE)
                    toggle_server_status(idle_server_index);
            } else {
                if (ded_status == IDLE) {
                    ded_status = next_event_index;
                    events->ded_completion = t->current + GetService(next_event_index);
                }
            }            

        } else if (next_event_type == GEN_COMPL_EVENT_TYPE) {       /* COMPLETAMENTO SERVER GP */
            current_state = gen_status[next_event_index];
            number_of_completions[current_state]++;
            toggle_server_status(next_event_index);
            customers[current_state]--;

        } else {                                                    /* COMPLETAMENTO SERVER DEDICATO */
            current_state = ded_status;
            number_of_completions[current_state]++;
            next_assignment_ded_server();
            customers[current_state]--;
        }

#ifdef VERIFY
        print_update(next_event_type, next_event_index, current_state);
#endif
    }

    //print_report(number_of_completions, area);
    return (load_statistics(area, number_of_completions));
}

int main(void) 
{
#ifdef STATIONARY
    PlantSeeds(STAT_INIT_SEED);
    simulation_run();
#else
    statistics_t *stat;
    
    PlantSeeds(TERM_INIT_SEED);

    double p0 = (P_BP * P_UO)/(P_UO + P_PP);
    double p1 = (P_BP * P_PP)/(P_UO + P_PP);
    double p2 = ((1-P_BP) * P_UO)/(P_UO + P_PP);
    double p3 = ((1-P_BP) * P_PP)/(P_UO + P_PP); 

    for (int j = 0; j < ENSEMBLE_SIZE; ++j) {
        stat = simulation_run(); 
        //printf("%lf\n", stat->n[4] + stat->n[5]);
        //printf("%lf\n", stat->d[INDEX]);
        //printf("%lf\n", (stat->n[0] + stat->n[1] + stat->n[2] + stat->n[3]) / M);
        printf("%lf\n", (p0*stat->d[0] + p1*stat->d[1] + p2*stat->d[2] + p3*stat->d[3]));
        //printf("%lf\n", P_BP*stat->w[4] + (1-P_BP)*stat->w[5]);
        free(stat);
    }
#endif
    
    return (0);
}

/* bel comandino proprio */
// echo src/simul.c | entr /bin/bash -c "make && ./bin/simul | ./bin/estimate >> plots/data/d0-stat.csv"