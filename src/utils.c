/*
 * Simulation libraries
 * 
 * authors: A. Chillotti    (M. 0299824)
 *          C. Cuffaro      (M. 0299838)      
 *          S. Tiberi       (M. 0299908)
 * 
 * A.Y. 2020/2021
 */
 
#include "rngs.h"                       /* Multi-stream generator */
#include "rvgs.h"                       /* Random variate generators */
#include "utils.h"                      /* Simulator utilities */

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
 * Next arrival event
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
double next_arr_event(double *items, int len, int *index)
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
 * Next completion event
 * 
 * @param items:
 *      Array of completion structs
 * @param len: 
 *      Array length
 * @param *index:
 *      Pointer to save index of the minimum
 * @return:
 *      The minimum element of the array
 */
double next_compl_event(compl_event_t **items, int len, int *index)
{
    int min_index = 0;
    double min_val = items[0]->time;

    for (int i = 1; i < len; ++i) {
        if (items[i]->time < min_val) {
            min_val = items[i]->time;
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

    min_arrival = next_arr_event(events->arrivals, NUMBER_OF_QUEUES, &min_arrival_index);
    min_gen_compl = next_compl_event(events->gen_completions, M-1, &min_gen_compl_index);

    if (min_arrival < min_gen_compl) {                  
        if (min_arrival < events->ded_completion->time) {     /* Next event is an arrival */
            *min_event_type = ARR_EVENT_TYPE;
            *min_index = min_arrival_index;
            return min_arrival;
        } else {                                        /* Next event is an completion of the dedicated server */
            *min_event_type = DED_COMPL_EVENT_TYPE;
            *min_index = 0;
            return events->ded_completion->time;
        }
    } else {
        if (min_gen_compl < events->ded_completion->time) {   /* Next event is an completion of a general server */
            *min_event_type = GEN_COMPL_EVENT_TYPE;
            *min_index = min_gen_compl_index;
            return min_gen_compl;
        } else {                                        /* Next event is an completion of the dedicated server */
            *min_event_type = DED_COMPL_EVENT_TYPE;     
            *min_index = 0;
            return events->ded_completion->time;
        }        
    }
}

/*
 * Generate next arrival of flow i 
 * 
 * @param type:
 *      i-th flow
 */
void GetArrival(int class)
{
    switch (class)
    {
    case UO_BP:
        SelectStream(UNICA_OP_BP_ARR_STREAM);
        events->arrivals[class] += Exponential(1 / (P_UO * P_BP * LAMBDA));
        break;
    case UO_STD:
        SelectStream(UNICA_OP_STD_ARR_STREAM);
        events->arrivals[class] += Exponential(1 / (P_UO * (1 - P_BP) * LAMBDA));
        break;
    case PP_BP:
        SelectStream(PAGAM_PREL_BP_ARR_STREAM);
        events->arrivals[class] += Exponential(1 / (P_PP * P_BP * LAMBDA));
        break;
    case PP_STD:
        SelectStream(PAGAM_PREL_STD_ARR_STREAM);
        events->arrivals[class] += Exponential(1 / (P_PP * (1 - P_BP) * LAMBDA));
        break;
    case SR_BP:
        SelectStream(SPED_RIT_BP_ARR_STREAM);
        events->arrivals[class] += Exponential(1 / (P_SR * P_BP * LAMBDA));
        break;
    case SR_STD:
        SelectStream(SPED_RIT_STD_ARR_STREAM);
        events->arrivals[class] += Exponential(1 / (P_SR * (1 - P_BP) * LAMBDA));
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
double GetService(int class) 
{
    switch(class) {
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

    for(i = 0; i < NUMBER_OF_QUEUES; ++i) {
        customers[i] = 0;
        in_service[i] = 0;
    }
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

    for (i = 0; i < M - 1; ++i) {
        events->gen_completions[i] = malloc(sizeof(compl_event_t));
        if (events->gen_completions[i] == NULL)
            abort();
        memset(events->gen_completions[i], 0x0, sizeof(compl_event_t));
        events->gen_completions[i]->time = INFTY;
        events->gen_completions[i]->user_class = NONE;
    }

    events->ded_completion = malloc(sizeof(compl_event_t));
    if (events->ded_completion == NULL)
        abort();
    memset(events->ded_completion, 0x0, sizeof(compl_event_t));
    events->ded_completion->time = INFTY;
    events->ded_completion->user_class = NONE;
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
 * Update time integrated populations
 *
 * @param *area:
 *      Pointer to a structure time_integrated_populations
 */
void update_tip(time_integrated_populations_t *area)
{
    for(int i = 0; i < NUMBER_OF_QUEUES; ++i) {
        if (customers[i] > 0) {
            area->customers[i] += (t->next - t->current) * customers[i];
            area->queue[i]   += (t->next - t->current) * (customers[i] - in_service[i]);
            area->service[i] += (t->next - t->current) * in_service[i];
        }
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
        stat->y[i] = area->service[i] / (t->current - ((batch_index - 1) * B));
#else
        stat->l[i] = area->customers[i] / t->current;
        stat->q[i] = area->queue[i] / t->current;
        stat->y[i] = area->service[i] / t->current;
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

void dump_statistics(statistics_t *stat, int *number_of_completions) 
{
    char *user_classes[NUMBER_OF_QUEUES] = {
        "\'Unica Operazione BancoPosta\'", "\'Pagamenti & Prelievi BancoPosta\'", "\'Unica Operazione Standard\'", 
        "\'Pagamenti & Prelievi Standard\'", "\'Spedizioni & Ritiri BancoPosta\'", "\'Spedizioni & Ritiri Standard\'"
    };

    for (int i = 0; i < NUMBER_OF_QUEUES; ++i) {
        printf("Statistics for %s:\n", user_classes[i]);
        printf("   number of completions ... = %6d\n", number_of_completions[i]);
        printf("   average wait ............ = %6.2f\n", stat->w[i]);
        printf("   average delay ........... = %6.2f\n", stat->d[i]);
        printf("   average service time .... = %6.2f\n", stat->s[i]);
        printf("   average # in the node ... = %6.2f\n", stat->l[i]);
        printf("   average # in the queue .. = %6.2f\n", stat->q[i]);
        printf("   average # in service .... = %6.2f\n\n", stat->y[i]);
    }

    printf("Last completion time c_n = %6.2f\n\n", t->current);
}