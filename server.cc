#include <omnetpp.h>
using namespace omnetpp; //for Omnet++ ver. 5

class Server : public cSimpleModule
{		
  private:
    cQueue queue;	            //the queue of jobs; it is assumed that the first job in the queue is the one being serviced
	cMessage *departure;        //special message; it reminds about the end of service and the need for job departure
	simtime_t departure_time;   //time of the next departure
	int N;
	double accepted,deleted;
	double L;
	cDoubleHistogram overflowBuffer;
	//cLongHistogram overflowBufferPeriod;
	cDoubleHistogram overflowBufferPeriod;
	simtime_t start=0;
	simtime_t overflowTime;
	double period,overflowDouble;
	int check;
	int oneTimeDel;
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msgin);

};

Define_Module(Server);


void Server::initialize()
{	
	departure = new cMessage("Departure");
	overflowBuffer.setName("overflow");
	overflowBuffer.setRange(0,20);
	overflowBuffer.setNumCells(200);

	overflowBufferPeriod.setName("overflowPeriod");
	overflowBufferPeriod.setRange(0,20);
	overflowBufferPeriod.setNumCells(200);

	N=10;
	accepted=0;
	deleted=0;
	L=0;
	check=0;
	oneTimeDel=0;
	WATCH(L);
}


void Server::handleMessage(cMessage *msgin)  //two types of messages may arrive: a job from the source, or the special message initiating job departure
{		
    if (msgin==departure)   //job departure
	{
		
		if(queue.getLength() == N-1 && check==1){

			overflowTime = (simTime()-(((cMessage *)queue.front())->getTimestamp()));
			overflowDouble = (simTime().dbl()-(((cMessage *)queue.front())->getTimestamp()).dbl());

			EV << overflowTime << " OVERFLOWTIME\n";
			//EV << overflowDouble << " DOUBLEOVERFLOWTIME\n";
			overflowBuffer.collect(overflowTime);

			if(oneTimeDel!=0){
				period = (overflowTime.dbl())/(double)(oneTimeDel);
				//EV << overflowTime.dbl() << " DOUBLE RZUTOWANIE";
				EV << period << " PERIOD\n";
				overflowBufferPeriod.collect(period);
			}
		
				check = 1;
				oneTimeDel=0;
		}

		cMessage *msg = (cMessage *)queue.pop();    //remove the finished job from the head of the queue
		send(msg,"out");                            //depart the finished job
		if (!queue.isEmpty())                         //if the queue is not empty, initiate the next service, i.e. schedule the next departure event in the future
		{
			departure_time=simTime()+par("service_time");
	        scheduleAt(departure_time,departure);
		}
	}
	else if(queue.getLength() <= N)                    //job arrival  (double)par("buffer_N")
	{	

		if(queue.getLength()==N && check==0){
		
			check = 1;
			EV << " START\n";
		
		}	


		msgin->setTimestamp();

		if (queue.isEmpty())  //if the queue is empty, the job that has just arrived has to be served immediately, i.e. the departure event of this job has to be scheduled in the future
		{
			departure_time=simTime()+par("service_time");
            scheduleAt(departure_time,departure);
		}


		queue.insert(msgin); //insert the job at the end of the queue
		accepted++;
	}
	else {

		delete msgin;
		deleted++;
		if(check==1){
		oneTimeDel++;
		}

	}

	L = deleted/(accepted+deleted);
	
	//EV << L << " L\n";
	//EV << accepted << " ACCEPTED\n";
	//EV << deleted << " DELETED\n";
	//EV << oneTimeDel << " TEMP DEL";

	if(overflowTime>1000)
	EV << " !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
}








