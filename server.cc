#include <omnetpp.h>
using namespace omnetpp; //for Omnet++ ver. 5

class Server : public cSimpleModule
{		
  private:
    cQueue queue;	            //the queue of jobs; it is assumed that the first job in the queue is the one being serviced
	cMessage *departure;        //special message; it reminds about the end of service and the need for job departure
	simtime_t departure_time;   //time of the next departure
	int N;
	double accepted,deleted,OverflowCounter;
	double L;
	cDoubleHistogram overflowBuffer;
	//cLongHistogram overflowBufferPeriod;
	cDoubleHistogram overflowBufferPeriod;
	cDoubleHistogram timeToOverflowBuffer;
	//cDoubleHistogram burstRatio;
	double noOverflow=0;
	simtime_t overflowTime,start,poczatek;
	simtime_t timeToOverflow;
	double period,overflowDouble;
	double getMeanPossible=0;
	int check;
	double K,G,B;
	double oneTimeDel;
	double AvgBufferLoss;
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msgin);

};

Define_Module(Server);


void Server::initialize()
{	
	departure = new cMessage("Departure");
	overflowBuffer.setName("overflow");
	overflowBuffer.setRange(0,3);
	overflowBuffer.setNumCells(300);

	overflowBufferPeriod.setName("overflowPeriod");
	overflowBufferPeriod.setRange(0,4);
	overflowBufferPeriod.setNumCells(1000);

	timeToOverflowBuffer.setName("timetooverflow");
	timeToOverflowBuffer.setRange(0,400);
	timeToOverflowBuffer.setNumCells(4000);

	//burstRatio.setName("burst");
	//burstRatio.setRange(0,20);
	//burstRatio.setNumCells(200);


	N=10;
	accepted=0;
	deleted=0;
	B = 0;
	L=0;

	check=0;
	oneTimeDel=0;
	OverflowCounter=1;
	overflowTime=0;
	AvgBufferLoss;
	WATCH(L);
}


void Server::handleMessage(cMessage *msgin)  //two types of messages may arrive: a job from the source, or the special message initiating job departure
{		
    if (msgin==departure)   //job departure
	{
		
		if(oneTimeDel != 0){

			overflowBufferPeriod.collect(oneTimeDel);
			oneTimeDel = 0;
			getMeanPossible=1;
		}

		
		if(queue.getLength()>=N && noOverflow == 1){

			timeToOverflow = (simTime()-start+par("service_time"));
		}
		
		cMessage *msg = (cMessage *)queue.pop();    //remove the finished job from the head of the queue

		if(queue.getLength() == N-1){

			//EV << simTime() << " SIMTIME";
			//EV << poczatek << " POCZATEK";
			//overflowTime = (simTime()-(((cMessage *)queue.front())->getTimestamp()));
			//overflowTime = (simTime()-poczatek);
			//overflowDouble = (simTime().dbl()-(((cMessage *)queue.front())->getTimestamp()).dbl());

			//overflowBuffer.collect(overflowTime);

			//overflowBuffer.collect(overflowTime);

				OverflowCounter++;
				check = 0;
		}



		send(msg,"out");                            //depart the finished job
		if (!queue.isEmpty())                         //if the queue is not empty, initiate the next service, i.e. schedule the next departure event in the future
		{
			departure_time=simTime()+par("service_time");
	        scheduleAt(departure_time,departure);
		}
	}
	else if(queue.getLength() <= N)                    //job arrival  (double)par("buffer_N")
	{	

		if(queue.getLength()==N && check==0)
		{
		
			check = 1;
		}	

		
		msgin->setTimestamp();

		if (queue.isEmpty())  //if the queue is empty, the job that has just arrived has to be served immediately, i.e. the departure event of this job has to be scheduled in the future
		{

			if(noOverflow == 0){

			noOverflow = 1;
			start  = simTime();
			//start = (((cMessage *)queue.front())->getTimestamp());
		}


			departure_time=simTime()+par("service_time");
            scheduleAt(departure_time,departure);
		}

		


		queue.insert(msgin); //insert the job at the end of the queue
		
		if(queue.getLength()==N){

		//poczatek = simTime();

		overflowBuffer.collect(departure_time - simTime());

		}

		accepted++;
	}
	else {

		delete msgin;
		deleted++;
	//	if(check==1){
		oneTimeDel++;
	//	EV << oneTimeDel << " TEMP DEL\n";
	//	}

		
	}

	L = deleted/(accepted+deleted);


		if(getMeanPossible == 1){	
		G = overflowBufferPeriod.getMean();
		K = (1)/(1-L);
		B = G/K;

	//	EV << K << " K\n";
	//	EV << G << " G\n";
	//	EV << B << " B\n";
		//burstRatio.collect(B);
		}
		//EV << L << " L\n";
		//EV << accepted << " ACCEPTED\n";
		//EV << deleted << " DELETED\n";

	if(overflowTime>20){
	EV << overflowTime << " OVERFLOW\n";
	EV << " !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
	}
}








