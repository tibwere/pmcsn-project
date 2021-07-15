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

#define ARRIVAL_TYPE_STREAM 9

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


/* Global variables (system status) */
int customers[NUMBER_OF_QUEUE];
struct server_info **gp_servers;
struct server_info *dedicated_server;

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

double min (double a, double b, double c)
{
    if (a < b) {
        if (a < c) {
            return a;
        } else {
            return c;
        }
    } else {
        if (b < c) {
            return b;
        } else {
            return c;
        }
    }
}

double GetArrival(int *type_ptr)
{
    static double arrival = START;
    SelectStream(ARRIVAL_TYPE_STREAM);
    double choose = Uniform(0, 1);

	if (choose <= 0.125) {
        SelectStream(UNICA_OP_BP_ARR_STREAM);
        *type_ptr = UO_BP;
	} else if (choose <= 0.5) {
        SelectStream(UNICA_OP_STD_ARR_STREAM);
        *type_ptr = UO_STD;
	} else if (choose <= 0.5875) {
        SelectStream(PAGAM_PREL_BP_ARR_STREAM);
        *type_ptr = PP_BP;
	} else if (choose <= 0.85) {
        SelectStream(PAGAM_PREL_STD_ARR_STREAM);
        *type_ptr = PP_STD;	
	} else if (choose <= 0.8875) {
        SelectStream(SPED_RIT_BP_ARR_STREAM);
        *type_ptr = SR_BP;
	} else {
        SelectStream(SPED_RIT_STD_ARR_STREAM);
        *type_ptr = SR_STD;
	}
    arrival += Exponential(1 / LAMBDA);

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

void init_gp_servers(void)
{
    gp_servers = calloc(M-1, sizeof(struct server_info *));

    for (int i = 0; i < M-1; ++i) {
        if ((gp_servers[i] = malloc(sizeof(struct server_info))) == NULL)
            abort();
        gp_servers[i]->status = IDLE;
        gp_servers[i]->next = INFTY;
    }
}

void init_ded_server(void)
{
    dedicated_server = malloc(sizeof(struct server_info));

    dedicated_server->status = IDLE;
    dedicated_server->next = INFTY;
}

int total_customers(void)
{
    int total = 0;
    for(int i = 0; i < NUMBER_OF_QUEUE; ++i)
        total += customers[i];

    return (total);
}

int *get_in_service_per_type(void)
{
    int *in_service_per_type;
    in_service_per_type = calloc(NUMBER_OF_QUEUE, sizeof(int));

    for (int i = 0; i < M-1; ++i) {
        if(gp_servers[i]->status != IDLE)
            in_service_per_type[gp_servers[i]->status]++;
    }
    if (dedicated_server->status != IDLE)
        in_service_per_type[dedicated_server->status]++;

    return (in_service_per_type);
}

void update_tip(struct time_integrated_populations *area, struct times *t)
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

int next_completion_gp(void)
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
        if (gp_servers[i]->status == IDLE)
            return (i);
    }

    return (-1);
}

void toggle_server_status(int index, double current)
{
    int max_prio_queue_not_empty = get_max_prio_queue_not_empty();
    if (max_prio_queue_not_empty != -1) {
        if (index != -1) {
            gp_servers[index]->status = max_prio_queue_not_empty;
            gp_servers[index]->next = current + GetService(max_prio_queue_not_empty);
        } else {
            dedicated_server->status = max_prio_queue_not_empty;
            dedicated_server->next = current + GetService(max_prio_queue_not_empty);
        }
    } else {
        if (index != -1) {
            gp_servers[index]->status = IDLE;
            gp_servers[index]->next = INFTY;
        } else {
            dedicated_server->status = IDLE;
            dedicated_server->next = INFTY;
        }
    }
}

void next_assignment_ded_server(double current)
{
    int *in_service = get_in_service_per_type();
    if (customers[SR_BP] > in_service[SR_BP]) {
        dedicated_server->status = SR_BP;
        dedicated_server->next = current + GetService(SR_BP);
    } else if (customers[SR_STD] > in_service[SR_STD]) {
        dedicated_server->status = SR_STD;
        dedicated_server->next = current + GetService(SR_STD);
    } else {
        toggle_server_status(-1, current);
    }
}

void print_update(struct times *t, int next_server_index)
{
    char *types_of_service[NUMBER_OF_QUEUE] = {"UNICA_OP_BP", "PAGAM_PREL_BP", "UNICA_OP_STD", "PAGAM_PREL_STD",
        "SPED_RIT_BP", "SPED_RIT_STD"};
    printf("Current time                 = %12.6lf\n", t->current);
    printf("Next arrival time            = %12.6lf\n", t->arrival);
    printf("Next gp_completion           = %12.6lf\n", gp_servers[next_server_index]->next);
    printf("Next ded_completion          = %12.6lf\n\n", dedicated_server->next);
    int j;
    for (j = 0; j < NUMBER_OF_QUEUE; j++)
        printf("Customers of %-15s = %5d\n", types_of_service[j], customers[j]);

    printf("\nTotal customers              = %5d\n\n", integers_sum(customers, NUMBER_OF_QUEUE));

    char *tmp;
    for (j = 0; j < M-1; j++) {
        tmp = (gp_servers[j]->status == -1) ? "IDLE" : types_of_service[gp_servers[j]->status];
        printf("Gp_Server[%d] Status = %s \n", j + 1, tmp);
        printf("             Next   = %lf\n", gp_servers[j]->next);
    }

    tmp = (dedicated_server->status == -1) ? "IDLE" : types_of_service[dedicated_server->status];
    printf("Ded_Server   Status = %s \n", tmp );
    printf("             Next   = %lf\n", dedicated_server->next);

    printf("===========================================\n");
}

void print_report(int *number_of_completions, struct time_integrated_populations *area, struct times *t) 
{
    int tot_completions = integers_sum(number_of_completions, NUMBER_OF_QUEUE);
    double area_customers = doubles_sum(area->customers, NUMBER_OF_QUEUE);
    double area_queue = doubles_sum(area->queue, NUMBER_OF_QUEUE);
    double area_service = doubles_sum(area->service, NUMBER_OF_QUEUE);
    char *types_of_service[NUMBER_OF_QUEUE] = {"UNICA_OP_BP", "PAGAM_PREL_BP", "UNICA_OP_STD", "PAGAM_PREL_STD",
        "SPED_RIT_BP", "SPED_RIT_STD"};
        
    printf("\n+--------------------+\n| SIMULATION RESULTS |\n+--------------------+\n");

    for(int j = 0; j < NUMBER_OF_QUEUE; j++) {
        printf("Completions of %-15s = %3d\n", types_of_service[j], number_of_completions[j]);
    }
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

int main(void) 
{
    int number_of_completions[NUMBER_OF_QUEUE];
    struct times *t;
    struct time_integrated_populations *area;
    int previous_arrival_type;
    int current_arrival_type;
    int idle_server_index;
    
    memset(number_of_completions, 0x0, NUMBER_OF_QUEUE * sizeof(int));
    memset(customers, 0x0, NUMBER_OF_QUEUE * sizeof(int));

    PlantSeeds(0);
  
    t = init_times(&current_arrival_type);
    init_gp_servers();
    init_ded_server();
    area = init_tip();
   
    while ((t->arrival < STOP) || (total_customers() > 0)) {
        int next_server_index = next_completion_gp();
        t->next = min(t->arrival, gp_servers[next_server_index]->next, dedicated_server->next);
        update_tip(area, t);
        t->current = t->next;
#ifdef DEBUG 
        print_update(t, next_server_index);
#endif
        if (t->current == t->arrival) { /* ARRIVO */
            customers[current_arrival_type]++;
            previous_arrival_type = current_arrival_type;
            t->arrival = GetArrival(&current_arrival_type);
            
            if (t->arrival > STOP) {
                t->last = t->current;
                t->arrival = INFTY;
            }
            if (previous_arrival_type != SR_BP && previous_arrival_type != SR_STD) {
                if ((idle_server_index = get_idle_server_gp()) != -1 || dedicated_server->status == IDLE)
                    toggle_server_status(idle_server_index, t->current);
            } else {
                if (dedicated_server->status == IDLE) {
                    dedicated_server->status = previous_arrival_type;
                    dedicated_server->next = t->current + GetService(previous_arrival_type);
                }
            }
        } else if (t->current == gp_servers[next_server_index]->next) {    /* COMPLETAMENTO SERVER GP */
            int current_state = gp_servers[next_server_index]->status;
            number_of_completions[current_state]++;
            toggle_server_status(next_server_index, t->current);
            customers[current_state]--;
        } else {    /* COMPLETAMENTO SERVER DEDICATO */
            int current_state = dedicated_server->status;
            number_of_completions[current_state]++;
            next_assignment_ded_server(t->current);
            customers[current_state]--;
        }
    }

    print_report(number_of_completions, area, t);

  return (0);
}


