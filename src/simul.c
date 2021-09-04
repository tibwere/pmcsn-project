/*
 * Simulator of "Poste Italiane" system
 * 
 * authors: A. Chillotti    (M. 0299824)
 *          C. Cuffaro      (M. 0299838)      
 *          S. Tiberi       (M. 0299908)
 * 
 * A.Y. 2020/2021
 */

/* Simulation libraries */
#include "rngs.h"                       /* Multi-stream generator */
#include "rvgs.h"                       /* Random variate generators */
#include "utils.h"                      /* Simulator utilities */

/* Prototypes */
void    print_update(int, int, int);
int     get_max_prio_queue_not_empty(void);
void    toggle_server_status(int);
void    next_assignment_ded_server(void);
void    simulation_run(statistics_t**);

/*
 * Index of a priority queue not empty in the system
 *
 * @return:
 *      Index of the queue (customers array) if exists, otherwise -1
 */
int get_max_prio_queue_not_empty(void)
{
    int prio_queue = 0;

    while (prio_queue < NUMBER_OF_GP_QUEUES) {
        if (customers[prio_queue] > in_service[prio_queue])
            break;
        ++prio_queue;
    }

    return ((prio_queue == NUMBER_OF_GP_QUEUES) ? -1 : prio_queue);
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
            gen_status[index] = BUSY;
            events->gen_completions[index]->time = t->current + GetService(max_prio_queue_not_empty);
            events->gen_completions[index]->user_class = max_prio_queue_not_empty;
        } else {                                                                                    /* The server is dedicated */
            ded_status = BUSY;
            events->ded_completion->time = t->current + GetService(max_prio_queue_not_empty);
            events->ded_completion->user_class = max_prio_queue_not_empty;
        }
        in_service[max_prio_queue_not_empty]++;
    } else {                                                                                        /* There isn't a queue not empty */
        if (index != -1) {                                                                          /* The server is general */
            gen_status[index] = IDLE;
            events->gen_completions[index]->time = INFTY;
            events->gen_completions[index]->user_class = NONE;
        } else {                                                                                    /* The server is dedicated */
            ded_status = IDLE;
            events->ded_completion->time = INFTY;
            events->ded_completion->user_class = NONE;
        }
    }
}

/*
 * Assign a job (Spedizioni & Ritiri [BancoPosta/Standard]) to the dedicated server
 */
void next_assignment_ded_server(void)
{
    if (customers[SR_BP] > in_service[SR_BP]) {
        ded_status = BUSY;
        in_service[SR_BP]++;
        events->ded_completion->time = t->current + GetService(SR_BP);
        events->ded_completion->user_class = SR_BP;
    } else if (customers[SR_STD] > in_service[SR_STD]) {
        ded_status = BUSY;
        in_service[SR_STD]++;
        events->ded_completion->time = t->current + GetService(SR_STD);
        events->ded_completion->user_class = SR_STD;
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
    int ticket;
    char *tmp;

    char *types_of_tickets[NUMBER_OF_QUEUES] = {
        "\'Unica Operazione BancoPosta\'", "\'Pagamenti & Prelievi BancoPosta\'", "\'Unica Operazione Standard\'", 
        "\'Pagamenti & Prelievi Standard\'", "\'Spedizioni & Ritiri BancoPosta\'", "\'Spedizioni & Ritiri Standard\'"
    };

    double min_arrival = next_arr_event(events->arrivals, NUMBER_OF_QUEUES, &dummy);
    double min_completion = next_compl_event(events->gen_completions, M-1, &dummy);

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
        events->ded_completion->time,
        ((events->ded_completion->time == INFTY) ? " (INFTY)" : "")
    );

    for (j = 0; j < NUMBER_OF_QUEUES; j++)
        printf("Customers of %-33s = %3d\n", types_of_tickets[j], customers[j]);

    printf("\nTotal customers                                = %3d\n\n", integers_sum(customers, NUMBER_OF_QUEUES));

    for (j = 0; j < M-1; j++) {
        ticket = events->gen_completions[j]->user_class;
        tmp = (ticket == NONE) ? "IDLE" : types_of_tickets[ticket];
        printf("General server %d:\n\tStatus          = %s \n", j, tmp);
        printf("\tNext completion = %lf\n\n", events->gen_completions[j]->time);
    }

    ticket = events->ded_completion->user_class;
    tmp = (ticket == NONE) ? "IDLE" : types_of_tickets[ticket];
    printf("Dedicated server:\n\tStatus          = %s \n", tmp);
    printf("\tNext completion = %lf\n\n", events->ded_completion->time);

    printf("=============================================================\n");
}

/*
 * Executes a single run of a simulation
 *
 * @return:%6.2
 *      Pointer to statistics collected 
 */
void simulation_run(statistics_t **stat) 
{
    int number_of_completions[NUMBER_OF_QUEUES];
    time_integrated_populations_t *area;
    int idle_server_index;
    int continue_simul[NUMBER_OF_QUEUES] = {[0 ... NUMBER_OF_QUEUES - 1] = 1};
    int next_event_type;
    int next_event_index;
    int current_state;
#ifdef STATIONARY
    statistics_t *stationary_stat;
#endif
        
    memset(number_of_completions, 0x0, NUMBER_OF_QUEUES * sizeof(int));
    init_times();
    init_event_list();
    init_status();
    area = init_tip();

    /* Uncomment these lines in validation phase (sec. 7.2) */
    //for (int i = NUMBER_OF_GP_QUEUES; i < NUMBER_OF_QUEUES; ++i)
    //   continue_simul[i] = 0;

    /* Uncomment these lines in validation phase (sec. 7.3) */
    //for (int i = 0; i < NUMBER_OF_GP_QUEUES; ++i)
    //   continue_simul[i] = 0;

#ifdef STATIONARY   
    batch_index = 0;

    while (has_to_continue(continue_simul)) { 

        double p0 = (P_BP * P_UO)/(P_UO + P_PP);
        double p1 = (P_BP * P_PP)/(P_UO + P_PP);
        double p2 = ((1-P_BP) * P_UO)/(P_UO + P_PP);
        double p3 = ((1-P_BP) * P_PP)/(P_UO + P_PP);

        if (t->current > B * (batch_index + 1)) {
            batch_index++;
            stationary_stat = load_statistics(area, number_of_completions);
            //printf("%d,%lf\n", batch_index, stationary_stat->d[3]);
            //printf("%lf\n", (p0*stationary_stat->d[0] + p1*stationary_stat->d[1] + p2*stationary_stat->d[2] + p3*stationary_stat->d[3]));
            //printf("%lf\n", (p0*stationary_stat->w[0] + p1*stationary_stat->w[1] + p2*stationary_stat->w[2] + p3*stationary_stat->w[3]));
            //printf("%lf\n", (stationary_stat->y[0] + stationary_stat->y[1] + stationary_stat->y[2] + stationary_stat->y[3]) / M);
            //printf("%lf\n", P_BP*stationary_stat->d[4] + (1-P_BP)*stationary_stat->d[5]);
            printf("%lf\n", P_BP*stationary_stat->w[4] + (1-P_BP)*stationary_stat->w[5]);

            /* Uncomment these lines to plot stationary delay */
            // if (STOP_BATCH == 1) 
            //     printf("%lf\n", (p0*stationary_stat->d[0] + p1*stationary_stat->d[1] + p2*stationary_stat->d[2] + p3*stationary_stat->d[3]));
            //     //printf("%lf\n", P_BP*stationary_stat->d[4] + (1-P_BP)*stationary_stat->d[5]);

            free(stationary_stat);
            memset(number_of_completions, 0x0, NUMBER_OF_QUEUES * sizeof(int));
            free(area);
            area = init_tip();
            update_tip(area);

            /* Uncomment these lines to analyze growing stats */
            // if (batch_index == STOP_BATCH)
            //     break;            
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

        if (next_event_type == ARR_EVENT_TYPE) {                    /* Arrival event occurrence */
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
                    ded_status = BUSY;
                    in_service[next_event_index] ++;
                    events->ded_completion->time = t->current + GetService(next_event_index);
                    events->ded_completion->user_class = next_event_index;
                }
            }            

        } else if (next_event_type == GEN_COMPL_EVENT_TYPE) {       /* Event occurrence of completion of general server */
            current_state = events->gen_completions[next_event_index]->user_class;
            number_of_completions[current_state]++;
            toggle_server_status(next_event_index);
            customers[current_state]--;
            in_service[current_state]--;

        } else {                                                    /* Event occurrence of completion of dedicated server */
            current_state = events->ded_completion->user_class;
            number_of_completions[current_state]++;
            next_assignment_ded_server();
            customers[current_state]--;
            in_service[current_state]--;
        }

#ifdef VERIFY
        print_update(next_event_type, next_event_index, current_state);
#endif
    }

    if (stat != NULL) {
        *stat = load_statistics(area, number_of_completions);
#ifdef VERIFY
        dump_statistics(*stat, number_of_completions);
#endif
    }
    
    /* Free allocated dynamic memory */
    free(t);
    for (int i = 0; i < M-1; ++i)
        free(events->gen_completions[i]);
    free(events->ded_completion);
    free(events);
    free(area);
}

int main(void) 
{
#ifdef STATIONARY
    PlantSeeds(STAT_INIT_SEED);
    simulation_run(NULL);
#else
    statistics_t *stat;
    
    PlantSeeds(TERM_INIT_SEED);

    // double p0 = (P_BP * P_UO)/(P_UO + P_PP);
    // double p1 = (P_BP * P_PP)/(P_UO + P_PP);
    // double p2 = ((1-P_BP) * P_UO)/(P_UO + P_PP);
    // double p3 = ((1-P_BP) * P_PP)/(P_UO + P_PP); 

    for (int j = 0; j < ENSEMBLE_SIZE; ++j) {
        simulation_run(&stat); 
        //printf("%lf\n", stat->n[4] + stat->n[5]);
        printf("%lf\n", stat->d[3]);
        //printf("%lf\n", (stat->n[0] + stat->n[1] + stat->n[2] + stat->n[3]) / M);
        //printf("%lf\n", (p0*stat->d[0] + p1*stat->d[1] + p2*stat->d[2] + p3*stat->d[3]));
        //printf("%lf\n", P_BP*stat->w[4] + (1-P_BP)*stat->w[5]);
        free(stat);
    }
#endif
    
    return (0);
}