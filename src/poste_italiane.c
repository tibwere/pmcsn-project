/* ------------------------------------------------------------------------- 
 * This program is a next-event simulation of a single-server FIFO service
 * node using Exponentially distributed interarrival times and Erlang 
 * distributed service times (i.e., a M/E/1 queue).  The service node is 
 * assumed to be initially idle, no arrivals are permitted after the 
 * terminal time STOP, and the node is then purged by processing any 
 * remaining jobs in the service node.
 *
 * Name            : ssq4.c  (Single Server Queue, version 4)
 * Author          : Steve Park & Dave Geyer
 * Language        : ANSI C
 * Latest Revision : 11-09-98
 * ------------------------------------------------------------------------- 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "rngs.h"                      /* the multi-stream generator */
#include "rvgs.h"                      /* random variate generators  */

#define START         0.0              /* initial time                   */
#define STOP      20000.0              /* terminal (close the door) time */
#define INFTY   (100.0 * STOP)      /* must be much larger than STOP  */

#define UNICA_OP_BP_ARR_STREAM 0
#define PAGAM_PREL_BP_ARR_STREAM 1
#define SPED_RIT_BP_ARR_STREAM 2
#define UNICA_OP_STD_ARR_STREAM 3
#define PAGAM_PREL_STD_ARR_STREAM 4
#define SPED_RIT_STD_ARR_STREAM 5
#define UNICA_OP_SERV_STREAM 6
#define PAGAM_PREL_SERV_STREAM 7
#define SPED_RIT_SERV_STREAM 8

#define M 4
#define NUMBER_OF_QUEUE 6
#define P_BP 0.25

#define P_UO 0.5
#define P_PP 0.35
#define P_SR 0.15

#define LAMBDA 8750 / 115281

#define MU_UO 53 / 400
#define MU_PP 53 / 600
#define MU_SR 53 / 800


enum type_of_ticket
{
	UNICA_OP_BP,
	UNICA_OP_STD,
	PAGAM_PREL_BP,
	PAGAM_PREL_STD,
	SPED_RIT_BP,
	SPED_RIT_STD
};

struct times {
    double arrival;                 /* next arrival time                   */
    double completion;              /* next completion time                */
    double current;                 /* current time                        */
    double next;                    /* next (most imminent) event time     */
    double last;                    /* last arrival time                   */
};
	
struct populations {
    double node;                    /* time integrated number in the node  */
    double queue[NUMBER_OF_QUEUE];                /* time integrated number in the queue */
    double service[M];              /* time integrated number in service   */
};


   double Min(double a, double c)
/* ------------------------------
 * return the smaller of a, b
 * ------------------------------
 */
{ 
  if (a < c)
    return (a);
  else
    return (c);
}


double GetArrival(enum type_of_ticket type)
{
    static double arrival = START;

    switch(type) {
    case UNICA_OP_BP:
        SelectStream(UNICA_OP_BP_ARR_STREAM);
        arrival += Exponential(P_BP * P_UO * LAMBDA);
        break;
    case UNICA_OP_STD:
        SelectStream(UNICA_OP_STD_ARR_STREAM);
        arrival += Exponential((1 - P_BP) * P_UO * LAMBDA);
        break;
    case PAGAM_PREL_BP:
        SelectStream(PAGAM_PREL_BP_ARR_STREAM);
        arrival += Exponential(P_BP * P_PP * LAMBDA);
        break;
    case PAGAM_PREL_STD:
        SelectStream(PAGAM_PREL_STD_ARR_STREAM);
        arrival += Exponential((1 - P_BP) * P_PP * LAMBDA);
        break;
    case SPED_RIT_BP:
        SelectStream(SPED_RIT_BP_ARR_STREAM);
        arrival += Exponential(P_BP * P_SR * LAMBDA);
        break;
    case SPED_RIT_STD:
        SelectStream(SPED_RIT_STD_ARR_STREAM);
        arrival += Exponential((1 - P_BP) * P_SR * LAMBDA);
        break;
    }

    return (arrival);
} 


double GetService(enum type_of_ticket type) 
{
    switch(type) {
    case UNICA_OP_BP:
    case UNICA_OP_STD:
        SelectStream(UNICA_OP_SERV_STREAM);
        return (Exponential(MU_UO));
    case PAGAM_PREL_BP:
    case PAGAM_PREL_STD:
        SelectStream(PAGAM_PREL_SERV_STREAM);
        return (Exponential(MU_PP));
    case SPED_RIT_BP:
    case SPED_RIT_STD:
        SelectStream(SPED_RIT_SERV_STREAM);
        return (Exponential(MU_SR));
    }
}  


struct populations *init_populations(void)
{
    struct populations *p = malloc(sizeof(struct populations));
    if (p == NULL)
        return NULL;
    memset(p, 0x0, sizeof(struct populations));

    return p;
}


struct times *init_times(void)
{
    struct times *t = malloc(sizeof(struct times));
    if (t == NULL)
        return NULL;
    memset(t, 0x0, sizeof(struct times));

    t->current = START;
    t->arrival = GetArrival();
    t->completion = INFTY;

    return t;
}


int main(void) 
{
    long index  = 0;                  /* used to count departed jobs         */
    long number = 0;                  /* number in the node                  */
    struct times *t;
    struct populations *p;

    PlantSeeds(0);

    t = init_times();
    a = init_populations();

  while ((t.arrival < STOP) || (number > 0)) {
    t.next          = Min(t.arrival, t.completion);  /* next event time   */
    if (number > 0)  {                               /* update integrals  */
      area.node    += (t.next - t.current) * number;
      area.queue   += (t.next - t.current) * (number - 1);
      area.service += (t.next - t.current);
    }
    t.current       = t.next;                    /* advance the clock */

    if (t.current == t.arrival)  {               /* process an arrival */
      number++;
      t.arrival     = GetArrival();
      if (t.arrival > STOP)  {
        t.last      = t.current;
        t.arrival   = INFTY;
      }
      if (number == 1)
        t.completion = t.current + GetService();
    }

    else {                                        /* process a completion */
      index++;
      number--;
      if (number > 0)
        t.completion = t.current + GetService();
      else
        t.completion = INFTY;
    }
  } 

  printf("\nfor %ld jobs\n", index);
  printf("   average interarrival time = %6.2f\n", t.last / index);
  printf("   average wait ............ = %6.2f\n", area.node / index);
  printf("   average delay ........... = %6.2f\n", area.queue / index);
  printf("   average service time .... = %6.2f\n", area.service / index);
  printf("   average # in the node ... = %6.2f\n", area.node / t.current);
  printf("   average # in the queue .. = %6.2f\n", area.queue / t.current);
  printf("   utilization ............. = %6.2f\n", area.service / t.current);

  return (0);
}
