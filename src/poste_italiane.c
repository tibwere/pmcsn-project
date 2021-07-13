#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "rngs.h"                       /* the multi-stream generator       */
#include "rvgs.h"                       /* random variate generators        */

#define START         0.0               /* initial time                     */
#define STOP      20000.0               /* terminal (close the door) time   */
#define INFTY   (100.0 * STOP)          /* must be much larger than STOP    */

#define UNICA_OP_BP_ARR_STREAM 0
#define PAGAM_PREL_BP_ARR_STREAM 1
#define UNICA_OP_STD_ARR_STREAM 2
#define PAGAM_PREL_STD_ARR_STREAM 3
#define SPED_RIT_BP_ARR_STREAM 4
#define SPED_RIT_STD_ARR_STREAM 5

#define UNICA_OP_SERV_STREAM 6
#define PAGAM_PREL_SERV_STREAM 7
#define SPED_RIT_SERV_STREAM 8

#define ARRIVAL_TYPE_STREAM 9

#define M 4
#define NUMBER_OF_QUEUE 6

#define P_BP 0.25

#define P_UO 0.5
#define P_PP 0.35
#define P_SR 0.15

#define LAMBDA (8750.0 / 115281)

#define MU_UO (53.0 / 400)
#define MU_PP (53.0 / 600)
#define MU_SR (53.0 / 800)

#define IDLE -1
#define UO_BP 0
#define PP_BP 1
#define UO_STD 2
#define PP_STD 3
#define SR_BP 4
#define SR_STD 5

#define MIN(a,b) (((a)<(b))?(a):(b))

struct server_info
{
    int status;
    double next;
};

struct times {
    double arrival;                 /* next arrival time                   */
    double current;                 /* current time                        */
    double next;                    /* next (most imminent) event time     */
    double last;                    /* last arrival time                   */
};
	
struct time_integrated_populations {
    double customers[NUMBER_OF_QUEUE];  /* time integrated number in the node  */
    double queue[NUMBER_OF_QUEUE];      /* time integrated number in the queue */
    double service[NUMBER_OF_QUEUE];    /* time integrated number in service   */
};


double GetArrival(int *type_ptr)
{
    static double arrival = START;
    SelectStream(ARRIVAL_TYPE_STREAM);
    double choose = Uniform(0, 1);

	if (choose <= 0.125) {
        SelectStream(UNICA_OP_BP_ARR_STREAM);
        *type_ptr = UO_BP;
        arrival += Exponential(1 / (P_BP * P_UO * LAMBDA));
	} else if (choose <= 0.5) {
        SelectStream(UNICA_OP_STD_ARR_STREAM);
        *type_ptr = UO_STD;
        arrival += Exponential(1 / ((1 - P_BP) * P_UO * LAMBDA));
	} else if (choose <= 0.5875) {
        SelectStream(PAGAM_PREL_BP_ARR_STREAM);
        *type_ptr = PP_BP;
        arrival += Exponential(1 / (P_BP * P_PP * LAMBDA));	
	} else if (choose <= 0.85) {
        SelectStream(PAGAM_PREL_STD_ARR_STREAM);
        *type_ptr = PP_STD;
        arrival += Exponential(1 / ((1 - P_BP) * P_PP * LAMBDA));	
	} else if (choose <= 0.8875) {
        SelectStream(SPED_RIT_BP_ARR_STREAM);
        *type_ptr = SR_BP;
        arrival += Exponential(1  / (P_BP * P_SR * LAMBDA));
	} else {
        SelectStream(SPED_RIT_STD_ARR_STREAM);
        *type_ptr = SR_STD;
        arrival += Exponential(1 / ((1 - P_BP) * P_SR * LAMBDA));	
	}

    return (arrival);
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


struct times *init_times(int *type_ptr)
{
    struct times *t = malloc(sizeof(struct times));
    if (t == NULL)
        return NULL;
    memset(t, 0x0, sizeof(struct times));

    t->current = START;
    t->arrival = GetArrival(type_ptr);

    return (t);
}

struct server_info **init_server_info(void)
{
    struct server_info **info = calloc(M-1, sizeof(struct server_info *));

    for (int i = 0; i < M-1; ++i) {
        if ((info[i] = malloc(sizeof(struct server_info))) == NULL)
            abort();
        info[i]->status = IDLE;
        info[i]->next = INFTY;
    }

    return (info);
}

long total_customers(long *customers)
{
    long total = 0L;
    for(int i = 0; i < NUMBER_OF_QUEUE; ++i)
        total += customers[i];

    return (total);
}

int *get_in_service_per_type(struct server_info **gp_servers)
{
    int *in_service_per_type;
    in_service_per_type = calloc(NUMBER_OF_QUEUE, sizeof(int));

    for (int i = 0; i < M-1; ++i) {
        if(gp_servers[i]->status != IDLE)
            in_service_per_type[gp_servers[i]->status]++;
    }

    return (in_service_per_type);
}

void update_tip(struct time_integrated_populations *area, struct times *t, 
    long *customers, struct server_info **gp_servers)
{
    int *in_service_per_type = get_in_service_per_type(gp_servers);

    for(int i = 0; i < NUMBER_OF_QUEUE; ++i) {
        if (customers[i] > 0) {
            area->customers[i] += (t->next - t->current) * customers[i];
            area->queue[i]   += (t->next - t->current) * (customers[i] - in_service_per_type[i]);
            area->service[i] += (t->next - t->current) * in_service_per_type[i];
        }
    }
}

int next_completion_gp(struct server_info **gp_servers)
{
    struct server_info *min_server = gp_servers[0];
    int min = 0;

    for(int i = 1; i < M-1; ++i) {
        if(gp_servers[i]->next < min_server->next) {
            min_server = gp_servers[i];
            min = i;
        }
    }

    return (min);
}

int get_max_prio_queue_not_empty(struct server_info **gp_servers, long *customers)
{
    int prio_queue = 0;
    int *in_service_per_type = get_in_service_per_type(gp_servers);

    while (prio_queue < NUMBER_OF_QUEUE) {
        if (customers[prio_queue] > in_service_per_type[prio_queue])
            break;
        ++prio_queue;
    }

    return ((prio_queue == NUMBER_OF_QUEUE) ? -1 : prio_queue);
}

int get_idle_server_gp(struct server_info **gp_servers)
{
    for (int i = 0; i < M-1; ++i) {
        if (gp_servers[i]->status == IDLE)
            return (i);
    }

    return (-1);
}

void toggle_server_status(struct server_info **gp_servers, long *customers, int index, double current)
{
    int max_prio_queue_not_empty = get_max_prio_queue_not_empty(gp_servers, customers);
    if (max_prio_queue_not_empty != -1) {
        gp_servers[index]->status = max_prio_queue_not_empty;
        double expo = GetService(max_prio_queue_not_empty);
        gp_servers[index]->next = current + expo;
    } else {
        gp_servers[index]->status = IDLE;
        gp_servers[index]->next = INFTY;
    }
}


int main(void) 
{
    long number_of_completions[NUMBER_OF_QUEUE];
    long customers[NUMBER_OF_QUEUE];
    struct server_info **gp_servers;
    struct times *t;
    struct time_integrated_populations *area;
    int last_arrival_type;
    int idle_server_index;
    
    memset(number_of_completions, 0x0, NUMBER_OF_QUEUE * sizeof(long));
    memset(customers, 0x0, NUMBER_OF_QUEUE * sizeof(long));

    PlantSeeds(0);
  
    t = init_times(&last_arrival_type);
    gp_servers = init_server_info();
    area = init_tip();
    
    while ((t->arrival < STOP) || (total_customers(customers) > 0)) {
        int next_server_index = next_completion_gp(gp_servers);
        t->next = MIN(t->arrival, gp_servers[next_server_index]->next);
        update_tip(area, t, customers, gp_servers);
        t->current = t->next;
        if (t->current == t->arrival)  { /* ARRIVO */
            customers[last_arrival_type]++;
            t->arrival = GetArrival(&last_arrival_type);
            
            if (t->arrival > STOP)  {
                t->last = t->current;
                t->arrival = INFTY;
            }

            if ((idle_server_index = get_idle_server_gp(gp_servers)) != -1)
                toggle_server_status(gp_servers, customers, idle_server_index, t->current);

        } else {    /* COMPLETAMENTO */
            number_of_completions[gp_servers[next_server_index]->status]++;
            customers[gp_servers[next_server_index]->status]--;
            toggle_server_status(gp_servers, customers, next_server_index, t->current);
        }
  }

//   printf("\nfor %ld jobs\n", index);
//   printf("   average interarrival time = %6.2f\n", t.last / index);
//   printf("   average wait ............ = %6.2f\n", area.node / index);
//   printf("   average delay ........... = %6.2f\n", area.queue / index);
//   printf("   average service time .... = %6.2f\n", area.service / index);
//   printf("   average # in the node ... = %6.2f\n", area.node / t.current);
//   printf("   average # in the queue .. = %6.2f\n", area.queue / t.current);
//   printf("   utilization ............. = %6.2f\n", area.service / t.current);

  return (0);
}


