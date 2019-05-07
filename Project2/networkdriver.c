/*
	Tyler Green	tgreen2		Cis 415 Project 2
	
	This is my work but is heavily influenced by the
	pseudo code provided from lab
*/

#include "packetdescriptor.h"
#include "destination.h"
#include "pid.h"
#include "freepacketdescriptorstore.h"
#include "freepacketdescriptorstore__full.h"
#include "networkdevice.h"
#include "pthread.h"
#include "diagnostics.h"
#include "queue.h"
#include "BoundedBuffer.h"

NetworkDevice *dev = NULL;
FreePacketDescriptorStore *fpd = NULL;
BoundedBuffer *send = NULL;
BoundedBuffer *recv[MAX_PID+1];

void blocking_send_packet(PacketDescriptor *pd)
{
	
	send->blockingWrite(send, (void *)pd);
}

int  nonblocking_send_packet(PacketDescriptor *pd)
{
	int ret;
	ret = send->nonblockingWrite(send, (void *)pd);
	return ret;
}

/* These calls hand in a PacketDescriptor for dispatching */
/* The nonblocking call must return promptly, indicating whether or */
/* not the indicated packet has been accepted by your code          */
/* (it might not be if your internal buffer is full) 1=OK, 0=not OK */
/* The blocking call will usually return promptly, but there may be */
/* a delay while it waits for space in your buffers.                */
/* Neither call should delay until the packet is actually sent      */

void blocking_get_packet(PacketDescriptor **pd, PID pid)
{
	recv[pid]->blockingRead(recv[pid], (void **)pd);
}

int  nonblocking_get_packet(PacketDescriptor **pd, PID pid)
{
	int ret;
	ret = recv[pid]->nonblockingRead(recv[pid], (void **)pd);
	return ret;
}

static void *send_thread()
{	
	int result, i;
	PacketDescriptor *pd;
	while(1)
	{
		send->blockingRead(send, (void **)&pd);
		result = 0;
		i = 0;
		for (i = 0; i <= 36; ++i)
		{
			result = dev->sendPacket(dev,pd);
			if(result == 0)
			{
				DIAGNOSTICS("Packet Succesfully Sent after %d times\n", i);
				break;
			}
		}
		fpd->nonblockingPut(fpd, pd);
	}
	return NULL;
}

static void *receive_thread()
{
	PacketDescriptor *temp;
	PacketDescriptor *pd;
	PID p;
	
	fpd->blockingGet(fpd, &pd);
	initPD(pd);
	dev->registerPD(dev, pd);
	while(1)
	{
		dev->awaitIncomingPacket(dev);
		if (fpd->nonblockingGet(fpd, &temp) == 1)
		{
			p = getPID(pd);
			/*if write is unsuccessful*/
			if ((recv[p]->nonblockingWrite(recv[p], (void *)pd)) == 0)
				fpd->nonblockingPut(fpd, pd);
			pd = temp;
		}

		initPD(pd);
		dev->registerPD(dev, pd);
	}
	return NULL;
}

/* These represent requests for packets by the application threads */
/* The nonblocking call must return promptly, with the result 1 if */
/* a packet was found (and the first argument set accordingly) or  */
/* 0 if no packet was waiting.                                     */
/* The blocking call only returns when a packet has been received  */
/* for the indicated process, and the first arg points at it.      */
/* Both calls indicate their process number and should only be     */
/* given appropriate packets. You may use a small bounded buffer   */
/* to hold packets that haven't yet been collected by a process,   */
/* but are also allowed to discard extra packets if at least one   */
/* is waiting uncollected for the same PID. i.e. applications must */
/* collect their packets reasonably promptly, or risk packet loss. */

void init_network_driver(NetworkDevice *nd, void *mem_start, 
			 unsigned long mem_length,
                         FreePacketDescriptorStore **fpds_ptr)
{
	int s, i;
	pthread_t sender;
	pthread_t receiver;

	enable_diagnostics();
	s = MAX_PID;/*fpd->size(fpd);*/
	dev = nd;
	fpd = FreePacketDescriptorStore_create(mem_start, mem_length);
	send = BoundedBuffer_create(s);
	for (i = 0; i <= MAX_PID; ++i)
	{
		recv[i] = BoundedBuffer_create(s);
	}
	*fpds_ptr = fpd;
		
	pthread_create(&sender, NULL, &send_thread, NULL);
	pthread_create(&receiver, NULL, &receive_thread, NULL);

}
/* Called before any other methods, to allow you to initialise */
/* data structures and start any internal threads.             */ 
/* Arguments:                                                  */
/*   nd: the NetworkDevice that you must drive,                */
/*   mem_start, mem_length: some memory for PacketDescriptors  */
/*   fpds_ptr: You hand back a FreePacketDescriptorStore into  */
/*             which you have put the divided up memory        */
/* Hint: just divide the memory up into pieces of the right size */
/*       passing in pointers to each of them                     */ 

