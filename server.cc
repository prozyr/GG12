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
	cLongHistogram overflowBufferPeriod;
	//cDoubleHistogram overflowBufferPeriod;
	cDoubleHistogram timeToOverflowBuffer;
	cDoubleHistogram burstRatio;
	double noOverflow=0;
	simtime_t overflowTime,start;
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
	overflowBuffer.setRange(0,20);
	overflowBuffer.setNumCells(200);

	overflowBufferPeriod.setName("overflowPeriod");
	overflowBufferPeriod.setRange(0,20);
	//overflowBufferPeriod.setNumCells(200);

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

		if(queue.getLength() < N && check == 1){

			overflowTime = (simTime()-(((cMessage *)queue.front())->getTimestamp()));
			//overflowDouble = (simTime().dbl()-(((cMessage *)queue.front())->getTimestamp()).dbl());

			//EV << overflowTime << " OVERFLOWTIME\n";
			//EV << overflowDouble << " DOUBLEOVERFLOWTIME\n";
			overflowBuffer.collect(overflowTime);

			//if(oneTimeDel!=0){
				//period = (double)(oneTimeDel)/(overflowTime.dbl());
				//simtime_t NewPeriod = (Pomiar.dbl())/(double)(oneTimeDel);
				//EV << overflowTime.dbl() << " DOUBLE RZUTOWANIE";
				//EV << period << " PERIOD\n";
			//	EV << NewPeriod << " NewPeriod\n";
			//	AvgBufferLoss = deleted/OverflowCounter;
				//overflowBufferPeriod.collect(oneTimeDel);
				//overflowBufferPeriod.collect(period);
			//}
			//	EV << OverflowCounter << " OVERFLOWCOUNTER\n";
				OverflowCounter++;
				check = 0;
				oneTimeDel=0;
		}

		if(queue.getLength()>=N && noOverflow == 1){

			timeToOverflow = (simTime()-start+par("service_time"));

			timeToOverflowBuffer.collect(timeToOverflow);

			noOverflow = 0;
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

		if(queue.getLength()==N && check==0)
		{

			check = 1;
			//EV << " START\n";
			//Pomiar = simTime();

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

		accepted++;
	}
	else {

		delete msgin;
		deleted++;
		if(check==1){
		oneTimeDel++;
		EV << oneTimeDel << " TEMP DEL\n";
		}


	}

	L = deleted/(accepted+deleted);


		if(getMeanPossible == 1){
		G = overflowBufferPeriod.getMean();
		K = 1/(1-L);
		B = G/K;

		EV << K << " K\n";
		EV << G << " G\n";
		EV << B << " B\n";
		//burstRatio.collect(B);
		}
		//EV << L << " L\n";
	//EV << accepted << " ACCEPTED\n";
	//EV << deleted << " DELETED\n";

	if(oneTimeDel>20)
	EV << " !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
}








