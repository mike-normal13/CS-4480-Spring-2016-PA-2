#include <stdio.h>
#include <stdlib.h>
#include <string.h> // added for memcpy
#include <assert.h> // may stay, may not

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

     Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

#define BIDIRECTIONAL 0    /* change to 1 if you're doing extra credit */
                           /* and write a routine called B_output */

/* a "msg" is the data unit passed from layer 5 (teachers code) to layer  */
/* 4 (students' code).  It contains the data (characters) to be delivered */
/* to layer 5 via the students transport level protocol entities.         */
     struct msg {
      char data[20];
    };

/* a packet is the data unit passed from layer 4 (students code) to layer */
/* 3 (teachers code).  Note the pre-defined packet structure, which all   */
/* students must follow. */
    struct pkt {
     int seqnum;
     int acknum;
     int checksum;
     char payload[20];
   };

//****************** My Globals *********************************************
int wait_seq_num = 0; //  flag indicating which seq number A/B should be waiting for  // assume only one var needed......
int ack_num = 0;  //  flag indicating which acknowledgment was received.
int A_to_B_in_transit = 0; // flag indicating that a message is currently in transit from the sender to the receiver. 
struct pkt saved_packet; //  save the last sent packet in case we need to resend it.
//****************** End of My Globals *********************************************

//*********************** My method prototypes ***********************************************
uint checksum(struct pkt* packet_addr, uint count);  // highly suspect!!!
//*********************** End of My method prototypes ***********************************************

//  TODO: part a requires implementing a Alternating Bit protocol...
/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
// THE NEXT 7 routines are "layer 4".

  //  A output(message), 
  //    where message is a structure of type msg, 
  //    containing data to be sent to the B-side. 
  //  This routine will be called whenever the upper layer at the sending side (A) has a message to send. 
  //  It is the job of your protocol to insure that the data in such a message is delivered in-order, 
  //    and correctly, 
  //    to the receiving side upper layer.
/* called from layer 5, passed the data to be sent to other side */
//  we assume that layer 5 does not make mistakes when it comes to sequence numbers.... **********
A_output(message)
  struct msg message; // 20 byte char array
  {
    printf("**** A_output's passed in message: %s\n", message.data);
    // local 'message' holder
    char p_load[20];
    //  local checksum holder
    uint temp_checksum = 0;

    memcpy(p_load, message.data, 20);

    printf("**** A_output's passed in message after memcpy: %s\n", message.data);

    printf("*****memcpy to p_load: %s\n", p_load);

     // make a packet
    struct pkt send_packet;  

    //  if turn is wait_seq_num == 0:
    if(wait_seq_num == 0)
    {
      //  Assemble packet
      send_packet.seqnum = wait_seq_num;
      send_packet.acknum = wait_seq_num;
      send_packet.checksum = 0;                     //************************************8         
      memcpy(send_packet.payload, p_load, 20);
      // calculate checksum
      temp_checksum = checksum(&send_packet, 36); // 4 + 4 + 4 + 20
      send_packet.checksum = temp_checksum;

      //  save the packet to global place holder
      memcpy(&saved_packet, &send_packet,  36);
      printf("***saved_packet's payload after memcpy: %s\n", saved_packet.payload);
    }
      //  if turn is wait_seq_num == 1:
    else
    {
      assert(wait_seq_num == 1);

              //  Assemble packet
      send_packet.seqnum = wait_seq_num;
      send_packet.acknum = wait_seq_num;
      send_packet.checksum = 0;                
      memcpy(send_packet.payload, p_load,  20);
        // calculate checksum
        temp_checksum = checksum(&send_packet, 36); // 4 + 4 + 4 + 20
        send_packet.checksum = temp_checksum;

        //  save the packet to global place holder
        memcpy(&saved_packet, &send_packet,  36);
      }

      printf("***********A_output sending packet with seq: %d, ack: %d, checksum: %d, and payload: %s\n", send_packet.seqnum, send_packet.acknum, send_packet.checksum, send_packet.payload);
      printf("***** size of sending payload: %ld\n", sizeof(send_packet.payload));

      //    send packet to layer 3
      tolayer3(0, send_packet);
      //    start timer    
      starttimer(0, 5.0);    //  suspect interval....
    }

B_output(message)  /* need be completed only for extra credit */
    struct msg message;
    {

    }

  //  A input(packet), 
  //    where packet is a structure of type pkt. 
  //  This routine will be called whenever a packet sent from the B-side 
  //    (i.e., as a result of a tolayer3() being done by a B-side procedure) arrives at the A-side. 
  //  packet is the (possibly corrupted) packet sent from the B-side.
/* called from layer 3, when a packet arrives for layer 4 */
    A_input(packet)
    struct pkt packet;
    {
      printf("@@@@@@@@@@@@@@@ entering A_input\n");
      char locl_payload[20];

      int locl_seqnum = packet.seqnum;
      int locl_acknum = packet.acknum;
      int locl_checksum = packet.checksum;
      memcpy(locl_payload, packet.payload,  20);

    //  set the received packet's checksum to zero so we can verify it
      packet.checksum = 0;

    //  return if packet is corrupt.
      if(locl_checksum != (checksum(&packet, 36)))
      {
        return;
      }

    //  Waiting for ACK 0 (3 cases):
    if(wait_seq_num == 0)      //  TODO: seq_num being 0 should mean we are waiting for AKC == 0....
    {
        //    Case 1: ACK == 1
      if(locl_acknum == 1)
      {
          return; //              DO nothing
        }
        //    Case 3: Packet is legit and ACK is 0
        else
        {
          assert(locl_acknum == 0);
          stoptimer(0);
          //  deliver data to layer 5
          //tolayer5(0, locl_payload);
          // set member wait_seq_num to 1
          wait_seq_num = 1;
          return;
        }
      }
      else
      {
        assert(wait_seq_num == 1);
        //    Case 1: ACK == 0
        if(locl_acknum == 0)
        {
          return; //              DO nothing
        }
        //    Case 3: Packet is legit and ACK is 0
        else
        {
          assert(locl_acknum == 1);
          stoptimer(0);
          //  deliver data to layer 5
          //tolayer5(0, locl_payload);
          // set member wait_seq_num to 0
          wait_seq_num = 0;
          return;
        }
      }
    }

  //  A timerinterrupt() 
  //  This routine will be called when A’s timer expires (thus generating a timer interrupt). 
  //  You can use this routine to control the retransmission of packets. 
  //  See starttimer() and stoptimer() below for how the timer is started and stopped.
/* called when A's timer goes off */
    A_timerinterrupt()
    {
      printf("$$$$$$$$$$ A_timerinterrupt was called\n");
      tolayer3(0, saved_packet);
      starttimer(0, 5.0);
    }  

  //  A init() This routine will be called once, 
  //    before any of your other A-side routines are called. 
  //  It can be used to do any required initialization.
/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
    A_init()
    {
      //bbbbbbbbbbbbbbbbbbbb�
      //bbbbbbbbbbbbbbbbbbbb�


  //  TODO: Not sure what to do here...
  //          How are we going to keep track of seq/ack numbers????
    }

  //  B input(packet), 
  //    where packet is a structure of type pkt. 
  //  This routine will be called whenever a packet sent from the A-side 
  //    (i.e., as a result of a tolayer3() being done by a A-side procedure) arrives at the B-side. 
  //  packet is the (possibly corrupted) packet sent from the A-side.
/* Note that with simplex transfer from a-to-B, there is no B_output() */
/* called from layer 3, when a packet arrives for layer 4 at B*/
    B_input(packet)
    struct pkt packet;
    {
      printf("^^^^^^^^^^^^^^^entering B_input\n");
    // acknowldegment packet 
    struct pkt ack_packet; // = NULL;
    //  acknowledgment packet's checksum
    uint temp_ack_packet_checksum = 0;

    // store packet fields
    int locl_seq_num = packet.seqnum;
    int locl_ack_num = packet.acknum;
    int locl_checksum = packet.checksum;
    char locl_payload[20]; 
    char ack_payload[20];
    memcpy(locl_payload, packet.payload, 20);

    printf("^^^^^^^B received packet with seq: %d, ack: %d, checksum: %d, and payload: %s\n", locl_seq_num, locl_ack_num, locl_checksum, packet.payload);

    // clear checksum to 0 in packet because that's what we did when we created the packet in the first place.
    packet.checksum = 0;

  //  State 0 (waiting for seq_num == 0 from below):
    if(wait_seq_num == 0)
    {
        // Parse the uncorrupted packet with seq_num ==0
        //  if the packet is not corrupt, and has seq_num == 0,
      if(packet.seqnum == 0 && (checksum(&packet, 36) == locl_checksum))
      {
          //  deliver data to layer 5,
        tolayer5(1, locl_payload);
          // switch global seq number 
        wait_seq_num = 1;

        ack_packet.seqnum = locl_seq_num;
        ack_packet.acknum = locl_ack_num;
        ack_packet.checksum = 0;
        memcpy(ack_packet.payload, ack_payload, 20);

          // derive checksum for acknowledgmet packet
        temp_ack_packet_checksum = checksum(&ack_packet, 36);
        ack_packet.checksum = temp_ack_packet_checksum;

          // send acknowledgement packet to A
        tolayer3(1, ack_packet);
        return;
        printf("*************B received packet with seq 0 and sent acknowledgement");
      }
        //  if, on the other hand, the packet is corrupt or has seq_num == 1
      else
      {
          //  assemble acknowledgment packet with seq_num == 1,
          //  send acknowledgement packet through layer 3
        ack_packet.seqnum = 1;
          ack_packet.acknum = 1;    // suspect..............***********
          ack_packet.checksum = 0;
          memcpy(ack_packet.payload, ack_payload, 20);

          // derive checksum for acknowledgmet packet
          temp_ack_packet_checksum = checksum(&ack_packet, 36);
          ack_packet.checksum = temp_ack_packet_checksum;

          // send acknowledgement packet to A
          tolayer3(1, ack_packet);
          return;
        }
      } 
      //  State 1 (waiting for seq_num == 1 from below):
      else
      {
        assert(wait_seq_num == 1);
        if(packet.seqnum == 1 && (checksum(&packet, 36) == locl_checksum))
        {
          //  deliver data to layer 5,
          tolayer5(1, locl_payload);
          // switch global seq number 
          wait_seq_num = 0;

          ack_packet.seqnum = locl_seq_num;
          ack_packet.acknum = locl_ack_num;
          ack_packet.checksum = 0;
          memcpy(ack_packet.payload, ack_payload, 20);

          // derive checksum for acknowledgmet packet
          temp_ack_packet_checksum = checksum(&ack_packet, 36);
          ack_packet.checksum = temp_ack_packet_checksum;

          // send acknowledgement packet to A
          tolayer3(1, ack_packet);
          return;
        }
        //  if, on the other hand, the packet is corrupt or has seq_num == 0
        else
        {
          //  assemble acknowledgment packet with seq_num == 0,
          //  send acknowledgement packet through layer 3
          ack_packet.seqnum = 0;
          ack_packet.acknum = 0;    // suspect..............***********
          ack_packet.checksum = 0;
          memcpy(ack_packet.payload, ack_payload, 20);

          // derive checksum for acknowledgmet packet
          temp_ack_packet_checksum = checksum(&ack_packet, 36);
          ack_packet.checksum = temp_ack_packet_checksum;

          // send acknowledgement packet to A
          tolayer3(1, ack_packet);
          return;
        }
      }
    }

/* called when B's timer goes off */
    B_timerinterrupt()
    {
  //  ????? I'm assuming we don't need this unless we do the extra credit...
    }

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
    B_init()
    {

    }

//***************************************** My Methods  *************************************
    void save_packet()
    {

    }

//uint checksum(struct pkt* packet_addr, uint count)  // highly suspect!!!
uint checksum(struct pkt* packet_addr, uint count)  // highly suspect!!!
{
  register uint sum = 0;

  // main summing loop
  while(count > 1)
  {
    sum = sum + (*((uint*) packet_addr) + 1);     // highly suspect!!
    //sum = sum + *((void*) packet_addr)++;
    //sum = sum + *((uint*) packet_addr)++;
    //sum = sum + *(packet_addr)++;
    count = count - 4;
  }

  //  Add left-over word, if any
  if(count > 0)
  {
    sum = sum + *((unsigned char*) packet_addr);
  }

  return(~sum);
}

void extract_packet()
{

}
//***************************************** End of My Methods  *************************************

/*****************************************************************
***************** NETWORK EMULATION CODE STARTS BELOW ***********
The code below emulates the layer 3 and below network environment:
  - emulates the tranmission and delivery (possibly with bit-level corruption
    and packet loss) of packets across the layer 3/4 interface
  - handles the starting/stopping of a timer, and generates timer
    interrupts (resulting in calling students timer handler).
  - generates message to be sent (passed from later 5 to 4)

THERE IS NOT REASON THAT ANY STUDENT SHOULD HAVE TO READ OR UNDERSTAND
THE CODE BELOW.  YOU SHOLD NOT TOUCH, OR REFERENCE (in your code) ANY
OF THE DATA STRUCTURES BELOW.  If you're interested in how I designed
the emulator, you're welcome to look at the code - but again, you should have
to, and you defeinitely should not have to modify
******************************************************************/

struct event {
   float evtime;           /* event time */
   int evtype;             /* event type code */
   int eventity;           /* entity where event occurs */
   struct pkt *pktptr;     /* ptr to packet (if any) assoc w/ this event */
 struct event *prev;
 struct event *next;
};
struct event *evlist = NULL;   /* the event list */

/* possible events: */
#define  TIMER_INTERRUPT 0  
#define  FROM_LAYER5     1
#define  FROM_LAYER3     2

#define  OFF             0
#define  ON              1
#define   A    0
#define   B    1



int TRACE = 1;             /* for my debugging */
int nsim = 0;              /* number of messages from 5 to 4 so far */ 
int nsimmax = 0;           /* number of msgs to generate, then stop */
float time = 0.000;
float lossprob;            /* probability that a packet is dropped  */
float corruptprob;         /* probability that one bit is packet is flipped */
float lambda;              /* arrival rate of messages from layer 5 */   
int   ntolayer3;           /* number sent into layer 3 */
int   nlost;               /* number lost in media */
int ncorrupt;              /* number corrupted by media*/

main()
{
 struct event *eventptr;
 struct msg  msg2give;
 struct pkt  pkt2give;

 int i,j;
 char c; 

 init();
 A_init();
 B_init();

 while (1) {
  eventptr = evlist;            /* get next event to simulate */
  if (eventptr==NULL)
   goto terminate;

  evlist = evlist->next;        /* remove this event from event list */
 if (evlist!=NULL)
   evlist->prev=NULL;
 if (TRACE>=2) {
   printf("\nEVENT time: %f,",eventptr->evtime);
   printf("  type: %d",eventptr->evtype);
   if (eventptr->evtype==0)
    printf(", timerinterrupt  ");
  else if (eventptr->evtype==1)
   printf(", fromlayer5 ");
 else
  printf(", fromlayer3 ");
printf(" entity: %d\n",eventptr->eventity);
}
        time = eventptr->evtime;        /* update time to next event time */
if (nsim==nsimmax)
	  break;                        /* all done with simulation */
  if (eventptr->evtype == FROM_LAYER5 ) {
            generate_next_arrival();   /* set up future arrival */
            /* fill in msg to give with string of same letter */    
    j = nsim % 26; 
    for (i=0; i<20; i++)  
     msg2give.data[i] = 97 + j;
   if (TRACE>2) {
     printf("          MAINLOOP: data given to student: ");
     for (i=0; i<20; i++) 
      printf("%c", msg2give.data[i]);
    printf("\n");
  }
  nsim++;
  if (eventptr->eventity == A) 
   A_output(msg2give);  
 else
   B_output(msg2give);  
}
else if (eventptr->evtype ==  FROM_LAYER3) {
  pkt2give.seqnum = eventptr->pktptr->seqnum;
  pkt2give.acknum = eventptr->pktptr->acknum;
  pkt2give.checksum = eventptr->pktptr->checksum;
  for (i=0; i<20; i++)  
    pkt2give.payload[i] = eventptr->pktptr->payload[i];
	if (eventptr->eventity ==A)      /* deliver packet by calling */
   	A_input(pkt2give);            /* appropriate entity */
  else
   B_input(pkt2give);
	free(eventptr->pktptr);          /* free the memory for packet */
}
else if (eventptr->evtype ==  TIMER_INTERRUPT) {
  if (eventptr->eventity == A) 
    A_timerinterrupt();
  else
    B_timerinterrupt();
}
else  {
  printf("INTERNAL PANIC: unknown event type \n");
}
free(eventptr);
}

terminate:
printf(" Simulator terminated at time %f\n after sending %d msgs from layer5\n",time,nsim);
}



init()                         /* initialize the simulator */
{
  int i;
  float sum, avg;
  float jimsrand();
  
  
  printf("-----  Stop and Wait Network Simulator Version 1.1 -------- \n\n");
  printf("Enter the number of messages to simulate: ");
  scanf("%d",&nsimmax);
  printf("Enter  packet loss probability [enter 0.0 for no loss]:");
  scanf("%f",&lossprob);
  printf("Enter packet corruption probability [0.0 for no corruption]:");
  scanf("%f",&corruptprob);
  printf("Enter average time between messages from sender's layer5 [ > 0.0]:");
  scanf("%f",&lambda);
  printf("Enter TRACE:");
  scanf("%d",&TRACE);

   srand(9999);              /* init random number generator */
   sum = 0.0;                /* test random number generator for students */
  for (i=0; i<1000; i++)
      sum=sum+jimsrand();    /* jimsrand() should be uniform in [0,1] */
   avg = sum/1000.0;
 if (avg < 0.25 || avg > 0.75) {
  printf("It is likely that random number generation on your machine\n" ); 
  printf("is different from what this emulator expects.  Please take\n");
  printf("a look at the routine jimsrand() in the emulator code. Sorry. \n");
  exit(-1);
}

ntolayer3 = 0;
nlost = 0;
ncorrupt = 0;

   time=0.0;                    /* initialize time to 0.0 */
   generate_next_arrival();     /* initialize event list */
}

/****************************************************************************/
/* jimsrand(): return a float in range [0,1].  The routine below is used to */
/* isolate all random number generation in one location.  We assume that the*/
/* system-supplied rand() function return an int in therange [0,mmm]        */
/****************************************************************************/
float jimsrand() 
{
  double mmm = 2147483647;   /* largest int  - MACHINE DEPENDENT!!!!!!!!   */
  float x;                   /* individual students may need to change mmm */ 
  x = rand()/mmm;            /* x should be uniform in [0,1] */
  return(x);
}  

/********************* EVENT HANDLINE ROUTINES *******/
/*  The next set of routines handle the event list   */
/*****************************************************/

generate_next_arrival()
{
 double x,log(),ceil();
 struct event *evptr;
   //    char *malloc();
 float ttime;
 int tempint;

 if (TRACE>2)
   printf("          GENERATE NEXT ARRIVAL: creating new arrival\n");
 
   x = lambda*jimsrand()*2;  /* x is uniform on [0,2*lambda] */
                             /* having mean of lambda        */
 evptr = (struct event *)malloc(sizeof(struct event));
 evptr->evtime =  time + x;
 evptr->evtype =  FROM_LAYER5;
 if (BIDIRECTIONAL && (jimsrand()>0.5) )
  evptr->eventity = B;
else
  evptr->eventity = A;
insertevent(evptr);
} 


insertevent(p)
struct event *p;
{
 struct event *q,*qold;

 if (TRACE>2) {
  printf("            INSERTEVENT: time is %lf\n",time);
  printf("            INSERTEVENT: future time will be %lf\n",p->evtime); 
}
   q = evlist;     /* q points to header of list in which p struct inserted */
   if (q==NULL) {   /* list is empty */
evlist=p;
p->next=NULL;
p->prev=NULL;
}
else {
  for (qold = q; q !=NULL && p->evtime > q->evtime; q=q->next)
    qold=q; 
        if (q==NULL) {   /* end of list */
  qold->next = p;
  p->prev = qold;
  p->next = NULL;
}
           else if (q==evlist) { /* front of list */
p->next=evlist;
p->prev=NULL;
p->next->prev=p;
evlist = p;
}
           else {     /* middle of list */
p->next=q;
p->prev=q->prev;
q->prev->next=p;
q->prev=p;
}
}
}

printevlist()
{
  struct event *q;
  int i;
  printf("--------------\nEvent List Follows:\n");
  for(q = evlist; q!=NULL; q=q->next) {
    printf("Event time: %f, type: %d entity: %d\n",q->evtime,q->evtype,q->eventity);
  }
  printf("--------------\n");
}



/********************** Student-callable ROUTINES ***********************/

/* called by students routine to cancel a previously-started timer */
stoptimer(AorB)                                                           //  **  we can call this
int AorB;  /* A or B is trying to stop timer */
{
 struct event *q,*qold;

 if (TRACE>2)
  printf("          STOP TIMER: stopping timer at %f\n",time);
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
for (q=evlist; q!=NULL ; q = q->next) 
  if ( (q->evtype==TIMER_INTERRUPT  && q->eventity==AorB) ) { 
       /* remove this event */
   if (q->next==NULL && q->prev==NULL)
             evlist=NULL;         /* remove first and only event on list */
          else if (q->next==NULL) /* end of list - there is one in front */
     q->prev->next = NULL;
          else if (q==evlist) { /* front of list - there must be event after */
   q->next->prev=NULL;
   evlist = q->next;
 }
           else {     /* middle of list */
 q->next->prev = q->prev;
 q->prev->next =  q->next;
}
free(q);
return;
}
printf("Warning: unable to cancel your timer. It wasn't running.\n");
}


starttimer(AorB,increment)                        //  **  we can call this
int AorB;  /* A or B is trying to stop timer */
float increment;
{

 struct event *q;
 struct event *evptr;
 //char *malloc();

 if (TRACE>2)
  printf("          START TIMER: starting timer at %f\n",time);
 /* be nice: check to see if timer is already started, if so, then  warn */
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
for (q=evlist; q!=NULL ; q = q->next)  
  if ( (q->evtype==TIMER_INTERRUPT  && q->eventity==AorB) ) { 
    printf("Warning: attempt to start a timer that is already started\n");
    return;
  }

/* create future event for when timer goes off */
  evptr = (struct event *)malloc(sizeof(struct event));
  evptr->evtime =  time + increment;
  evptr->evtype =  TIMER_INTERRUPT;
  evptr->eventity = AorB;
  insertevent(evptr);
} 


/************************** TOLAYER3 ***************/
tolayer3(AorB,packet)                                 //    **    we can call this
int AorB;  /* A or B is trying to stop timer */
struct pkt packet;
{
  //printf("<<<<<<<<<<<<<<<<<<entering tolayer3\n");
 struct pkt *mypktptr;
 struct event *evptr,*q;
 //char *malloc();
 float lastime, x, jimsrand();
 int i;


 ntolayer3++;

 /* simulate losses: */
 if (jimsrand() < lossprob)  {
  nlost++;
  if (TRACE>0)    
   printf("          TOLAYER3: packet being lost\n");
 return;
}  

/* make a copy of the packet student just gave me since he/she may decide */
/* to do something with the packet after we return back to him/her */ 
mypktptr = (struct pkt *)malloc(sizeof(struct pkt));
mypktptr->seqnum = packet.seqnum;
mypktptr->acknum = packet.acknum;
mypktptr->checksum = packet.checksum;
for (i=0; i<20; i++)
  mypktptr->payload[i] = packet.payload[i];
if (TRACE>2)  {
 printf("          TOLAYER3: seq: %d, ack %d, check: %d ", mypktptr->seqnum,
   mypktptr->acknum,  mypktptr->checksum);
 for (i=0; i<20; i++)
  printf("%c",mypktptr->payload[i]);
printf("\n");
}

/* create future event for arrival of packet at the other side */
evptr = (struct event *)malloc(sizeof(struct event));
  evptr->evtype =  FROM_LAYER3;   /* packet will pop out from layer3 */
  evptr->eventity = (AorB+1) % 2; /* event occurs at other entity */
  evptr->pktptr = mypktptr;       /* save ptr to my copy of packet */
/* finally, compute the arrival time of packet at the other end.
   medium can not reorder, so make sure packet arrives between 1 and 10
   time units after the latest arrival time of packets
   currently in the medium on their way to the destination */
   lastime = time;
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next) */
   for (q=evlist; q!=NULL ; q = q->next) 
    if ( (q->evtype==FROM_LAYER3  && q->eventity==evptr->eventity) ) 
      lastime = q->evtime;
    evptr->evtime =  lastime + 1 + 9*jimsrand();



 /* simulate corruption: */
    if (jimsrand() < corruptprob)  {
      ncorrupt++;
      if ( (x = jimsrand()) < .75)
       mypktptr->payload[0]='Z';   /* corrupt payload */
        else if (x < .875)
         mypktptr->seqnum = 999999;
       else
         mypktptr->acknum = 999999;
       if (TRACE>0)    
         printf("          TOLAYER3: packet being corrupted\n");
     }  

     if (TRACE>2)  
       printf("          TOLAYER3: scheduling arrival on other side\n");
     insertevent(evptr);

     //printf("<<<<<<<<<<<<<<<<leaving tolayer3\n");
   } 

   tolayer5(AorB,datasent)    //  **      **    **    we can call this
   int AorB;
   char datasent[20];
   {
    int i;  
    if (TRACE>2) {
     printf("          TOLAYER5: data received: ");
     for (i=0; i<20; i++)  
      printf("%c",datasent[i]);
    printf("\n");
  }
  
}
