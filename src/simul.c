#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "rngs.h"                       /* the multi-stream generator       */
#include "rvgs.h"                       /* random variate generators        */

#define DEBUG

#define START 0.0               /* initial time                     */
#define STOP (60.0 * (60 / 7))  /* terminal (close the door) time   */
#define INFTY (100.0 * STOP)    /* must be much larger than STOP    */

#define UNICA_OP_BP_ARR_STREAM 0
#define PAGAM_PREL_BP_ARR_STREAM 1
#define UNICA_OP_STD_ARR_STREAM 2
#define PAGAM_PREL_STD_ARR_STREAM 3
#define SPED_RIT_BP_ARR_STREAM 4
#define SPED_RIT_STD_ARR_STREAM 5

#define UNICA_OP_SERV_STREAM 6
#define PAGAM_PREL_SERV_STREAM 7
#define SPED_RIT_SERV_STREAM 8

#define M 3
#define NUMBER_OF_QUEUE 6
#define NUMBER_OF_GP_QUEUE 4

#define P_BP 0.25

#define P_UO 0.5
#define P_PP 0.35
#define P_SR 0.15

#define LAMBDA (24500.0 / 115281)

#define MU_UO (41.0 / 340)
#define MU_PP (41.0 / 510)
#define MU_SR (41.0 / 680)

#define IDLE -1
#define UO_BP 0
#define PP_BP 1
#define UO_STD 2
#define PP_STD 3
#define SR_BP 4
#define SR_STD 5

#define ARR_EVENT_TYPE 0
#define GEN_COMPL_EVENT_TYPE 1
#define DED_COMPL_EVENT_TYPE 2


typedef struct event_list {
    double arrivals[NUMBER_OF_QUEUE]; 
    double gen_completions[M-1]; 
    double ded_completion;
    double next;                      
    double current;
    double last;                    
} event_list_t; 

struct time_integrated_populations {
    double customers[NUMBER_OF_QUEUE];  
    double queue[NUMBER_OF_QUEUE];      
    double service[NUMBER_OF_QUEUE];   
};

/* System Status */
int customers[NUMBER_OF_QUEUE] = {[0 ... NUMBER_OF_QUEUE - 1] = 0};
int gen_status[M-1] = {[0 ... M - 2] = IDLE};
int ded_status = IDLE;

/* Event List */
event_list_t *t;


int integers_sum(int *items, int len) {
    int sum = 0;

    for (int i = 0; i < len; ++i)
        sum += items[i];
    
    return sum;
}

double doubles_sum(double *items, int len) {
    double sum = 0;

    for (int i = 0; i < len; ++i)
        sum += items[i];
    
    return sum;
}

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
    return min_val;
}

double next_event(int *min_event_type, int *min_index)
{
    int min_arrival_index, min_gen_compl_index;
    double min_arrival, min_gen_compl;

    min_arrival = min_from_array(t->arrivals, NUMBER_OF_QUEUE, &min_arrival_index);
    min_gen_compl = min_from_array(t->gen_completions, M-1, &min_gen_compl_index);

    //printf("min_arr = %lf, min_compl = %lf")

    if (min_arrival < min_gen_compl) {
        if (min_arrival < t->ded_completion) {
            *min_event_type = ARR_EVENT_TYPE;
            *min_index = min_arrival_index;
            return min_arrival;
        } else {
            *min_event_type = DED_COMPL_EVENT_TYPE;
            *min_index = 0;
            return t->ded_completion;
        }
    } else {
        if (min_gen_compl < t->ded_completion) {
            *min_event_type = GEN_COMPL_EVENT_TYPE;
            *min_index = min_gen_compl_index;
            return min_gen_compl;
        } else {
            *min_event_type = DED_COMPL_EVENT_TYPE;
            *min_index = 0;
            return t->ded_completion;
        }        
    }
}

double GetArrival(int type)
{
    static double arrivals[NUMBER_OF_QUEUE] = {START};

    switch (type)
    {
    case UO_BP:
        SelectStream(UNICA_OP_BP_ARR_STREAM);
        arrivals[type] += Exponential(1 / (P_UO * P_BP * LAMBDA));
        break;
    case UO_STD:
        SelectStream(UNICA_OP_STD_ARR_STREAM);
        arrivals[type] += Exponential(1 / (P_UO * (1 - P_BP) * LAMBDA));
        break;
    case PP_BP:
        SelectStream(PAGAM_PREL_BP_ARR_STREAM);
        arrivals[type] += Exponential(1 / (P_PP * P_BP * LAMBDA));
        break;
    case PP_STD:
        SelectStream(PAGAM_PREL_STD_ARR_STREAM);
        arrivals[type] += Exponential(1 / (P_PP * (1 - P_BP) * LAMBDA));
        break;
    case SR_BP:
        SelectStream(SPED_RIT_BP_ARR_STREAM);
        arrivals[type] += Exponential(1 / (P_SR * P_BP * LAMBDA));
        break;
    case SR_STD:
        SelectStream(SPED_RIT_STD_ARR_STREAM);
        arrivals[type] += Exponential(1 / (P_SR * (1 - P_BP) * LAMBDA));
        break;
    default:
        abort();
    }

    return (arrivals[type]);
} 

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

struct time_integrated_populations *init_tip(void)
{
    struct time_integrated_populations *p = malloc(sizeof(struct time_integrated_populations));
    if (p == NULL)
        return NULL;
    memset(p, 0x0, sizeof(struct time_integrated_populations));

    return (p);
}

void init_event_list(void)
{

    int i;
    t = malloc(sizeof(event_list_t));
    if (t == NULL)
        abort();
    memset(t, 0x0, sizeof(event_list_t));

    t->current = START;

    for (i = 0; i < NUMBER_OF_QUEUE; ++i) 
        t->arrivals[i] = GetArrival(i);

    for (i = 0; i < M - 1; ++i) 
        t->gen_completions[i] = INFTY;
    
    t->ded_completion = INFTY;
}

int *get_in_service_per_type(void)
{
    int *in_service_per_type;
    in_service_per_type = calloc(NUMBER_OF_QUEUE, sizeof(int));

    for (int i = 0; i < M-1; ++i) {
        if(gen_status[i] != IDLE)
            in_service_per_type[gen_status[i]]++;
    }
    if (ded_status != IDLE)
        in_service_per_type[ded_status]++;

    return (in_service_per_type);
}

void update_tip(struct time_integrated_populations *area)
{
    int *in_service_per_type = get_in_service_per_type();

    for(int i = 0; i < NUMBER_OF_QUEUE; ++i) {
        if (customers[i] > 0) {
            area->customers[i] += (t->next - t->current) * customers[i];
            area->queue[i]   += (t->next - t->current) * (customers[i] - in_service_per_type[i]);
            area->service[i] += (t->next - t->current) * in_service_per_type[i];
        }
    }
}

int get_max_prio_queue_not_empty(void)
{
    int prio_queue = 0;
    int *in_service_per_type = get_in_service_per_type();

    while (prio_queue < NUMBER_OF_GP_QUEUE) {
        if (customers[prio_queue] > in_service_per_type[prio_queue])
            break;
        ++prio_queue;
    }

    return ((prio_queue == NUMBER_OF_GP_QUEUE) ? -1 : prio_queue);
}

int get_idle_server_gp(void)
{
    for (int i = 0; i < M-1; ++i) {
        if (gen_status[i] == IDLE)
            return (i);
    }
    return (-1);
}

void toggle_server_status(int index)
{
    int max_prio_queue_not_empty = get_max_prio_queue_not_empty();
    if (max_prio_queue_not_empty != -1) {
        if (index != -1) {
            gen_status[index] = max_prio_queue_not_empty;
            t->gen_completions[index] = t->current + GetService(max_prio_queue_not_empty);
        } else {
            ded_status = max_prio_queue_not_empty;
            t->ded_completion = t->current + GetService(max_prio_queue_not_empty);
        }
    } else {
        if (index != -1) {
            gen_status[index] = IDLE;
            t->gen_completions[index] = INFTY;
        } else {
            ded_status = IDLE;
            t->ded_completion = INFTY;
        }
    }
}

void next_assignment_ded_server(void)
{
    int *in_service = get_in_service_per_type();
    if (customers[SR_BP] > in_service[SR_BP]) {
        ded_status = SR_BP;
        t->ded_completion = t->current + GetService(SR_BP);
    } else if (customers[SR_STD] > in_service[SR_STD]) {
        ded_status = SR_STD;
        t->ded_completion = t->current + GetService(SR_STD);
    } else {
        toggle_server_status(-1);
    }
}

void print_update(void)
{
    int dummy;
    int j;
    char *tmp;
    char *types_of_service[NUMBER_OF_QUEUE] = {
        "UNICA_OP_BP", "PAGAM_PREL_BP", "UNICA_OP_STD", 
        "PAGAM_PREL_STD", "SPED_RIT_BP", "SPED_RIT_STD"
    };

    printf("Current time                 = %12.6lf\n", t->current);
    printf("Next arrival time            = %12.6lf\n", min_from_array(t->arrivals, NUMBER_OF_QUEUE, &dummy));
    printf("Next gp_completion           = %12.6lf\n", min_from_array(t->gen_completions, M-1, &dummy));
    printf("Next ded_completion          = %12.6lf\n\n", t->ded_completion);
    
    for (j = 0; j < NUMBER_OF_QUEUE; j++)
        printf("Customers of %-15s = %5d\n", types_of_service[j], customers[j]);

    printf("\nTotal customers              = %5d\n\n", integers_sum(customers, NUMBER_OF_QUEUE));

    for (j = 0; j < M-1; j++) {
        tmp = (gen_status[j] == -1) ? "IDLE" : types_of_service[gen_status[j]];
        printf("Gp_Server[%d] Status = %s \n", j + 1, tmp);
        printf("             Next   = %lf\n", t->gen_completions[j]);
    }

    tmp = (ded_status == -1) ? "IDLE" : types_of_service[ded_status];
    printf("Ded_Server   Status = %s \n", tmp);
    printf("             Next   = %lf\n", t->ded_completion);

    printf("===========================================\n");
    //getchar();
}

void print_report(int *number_of_completions, struct time_integrated_populations *area) 
{
    int tot_completions = integers_sum(number_of_completions, NUMBER_OF_QUEUE);
    double area_customers = doubles_sum(area->customers, NUMBER_OF_QUEUE);
    double area_queue = doubles_sum(area->queue, NUMBER_OF_QUEUE);
    double area_service = doubles_sum(area->service, NUMBER_OF_QUEUE);
    char *types_of_service[NUMBER_OF_QUEUE] = {
        "UNICA_OP_BP", "PAGAM_PREL_BP", "UNICA_OP_STD", 
        "PAGAM_PREL_STD", "SPED_RIT_BP", "SPED_RIT_STD"
    };
        
    printf("\n+--------------------+\n| SIMULATION RESULTS |\n+--------------------+\n");

    for(int j = 0; j < NUMBER_OF_QUEUE; j++) 
        printf("Completions of %-15s = %3d\n", types_of_service[j], number_of_completions[j]);
    
    printf("\nTotal completions              = %3d\n\n", tot_completions);

    printf("Average interarrival time      = %10.6f\n", t->last / tot_completions);
    printf("Average wait                   = %10.6f\n", area_customers / tot_completions);
    printf("Average delay                  = %10.6f\n", area_queue / tot_completions);
    printf("Average service time           = %10.6f\n", area_service / tot_completions);
    printf("Average # in the node          = %10.6f\n", area_customers / t->current);
    printf("Average # in the queue         = %10.6f\n", area_queue / t->current);
    printf("Average # in service           = %10.6f\n", area_service / t->current);
    printf("Utilization                    = %10.6f\n", (area_service / M) / t->current);
}

int has_to_continue(int *flags)
{
    int result = flags[0];
    for (int i = 1; i < NUMBER_OF_QUEUE; ++i) 
        result = result || flags[i];
    
    return result;
}

int main(void) 
{
    int number_of_completions[NUMBER_OF_QUEUE];
    struct time_integrated_populations *area;
    int idle_server_index;
    int continue_simul[NUMBER_OF_QUEUE] = {1};
    int next_event_type;
    int next_event_index;
    int current_state;

    PlantSeeds(0);

    memset(number_of_completions, 0x0, NUMBER_OF_QUEUE * sizeof(int));
    init_event_list();
    area = init_tip();

    for (int i = 0; i < M-1; ++i) {
        printf("Gen_Status[%d] = %d\n", i, gen_status[i]);
    } 
   
    while (has_to_continue(continue_simul) || (integers_sum(customers, NUMBER_OF_QUEUE) > 0)) {
        t->next = next_event(&next_event_type, &next_event_index);
        update_tip(area);
        t->current = t->next;

#ifdef DEBUG
        print_update();
#endif

        if (next_event_type == ARR_EVENT_TYPE) {                       /* ARRIVO */
            customers[next_event_index]++;

            t->arrivals[next_event_index] = GetArrival(next_event_index);
            
            if (t->arrivals[next_event_index] > STOP) {
                continue_simul[next_event_index] = 0;
                t->arrivals[next_event_index] = INFTY;

                if (!has_to_continue(continue_simul))
                    t->last = t->current;
            }

            if ((next_event_index != SR_BP) && (next_event_index != SR_STD)) {
                if ((idle_server_index = get_idle_server_gp()) != -1 || ded_status == IDLE)
                    toggle_server_status(idle_server_index);
            } else {
                if (ded_status == IDLE) {
                    ded_status = next_event_index;
                    ded_status = t->current + GetService(next_event_index);
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
    }

    print_report(number_of_completions, area);

  return (0);
}


