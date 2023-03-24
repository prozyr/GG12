#include <omnetpp.h>
using namespace omnetpp; //for Omnet++ ver. 5

class Server : public cSimpleModule
{
  private:
    cQueue queue;	            //the queue of jobs; it is assumed that the first job in the queue is the one being serviced
	cMessage *departure;        //special message; it reminds about the end of service and the need for job departure
	simtime_t departure_time;   //time of the next departure
	// histograms
	cDoubleHistogram hist_overflow, hist_over_aveD;
	cLongHistogram hist_over_aveL;
	// variables
	simtime_t time_overflow, time_start_buffer, t;
	int start;
	cMessage *arrival_time;
	struct statistic
	{
		int N, deleted, accepted, deleted_in_one_row;
		double answer;	// Size of queue
	}queueStat;
	bool del, ZLICZANIE;
	double follow1,follow2;
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msgin);

};

Define_Module(Server);

void Server::initialize()
{
	departure = new cMessage("Departure");

	queueStat = {10,0,0,0,0.0}; start = 0;
	// arrival_time = new cMessage("ar_time");
	// t = 1;
	del = false;
	ZLICZANIE = false;
	// scheduleAt(simTime()+t,arrival_time);
	// Ad.2 OVERFLOW
	hist_overflow.setName("BUFFER OVERFLOW DISTRIBUTION");
	hist_overflow.setRange(0,30);
	hist_overflow.setNumCells(200);
	// Ad.3 LOSSES
	hist_over_aveL.setName("BUFFER AVE. OVER. LOSSES DIST. Long");
	hist_over_aveL.setRange(0,10);

	hist_over_aveD.setName("BUFFER AVE. OVER. LOSSES DIST. Double");
	hist_over_aveD.setRange(0,200);
	hist_over_aveD.setNumCells(200);
	// hist_over_aveL.setNumCells(200);

	WATCH(follow1);
	WATCH(follow2);
}


void Server::handleMessage(cMessage *msgin)  //two types of messages may arrive: a job from the source, or the special message initiating job departure
{
	// if (msgin==arrival_time)
	// {
	// 	scheduleAt(simTime()+t,arrival_time);
	// }
    // else
	if (msgin==departure)   //job departure
	{
		if (!(queue.getLength() <= queueStat.N)&&del)
		{
			hist_overflow.collect(simTime() - (((cMessage*)queue.front())->getTimestamp()));
			hist_over_aveL.collect(queueStat.deleted_in_one_row);
			EV << "DELETED IN ONE ROW: " << queueStat.deleted_in_one_row << "\n";
			follow1 = hist_overflow.getMean(); follow2 = hist_over_aveL.getMean();
			queueStat.deleted_in_one_row = 0;
		}


		cMessage *msg = (cMessage *)queue.pop();    //remove the finished job from the head of the queue
		send(msg,"out");                            //depart the finished job
		EV << "DEPARTURE queue: " << queue.getLength();
		if (!queue.isEmpty())                         //if the queue is not empty, initiate the next service, i.e. schedule the next departure event in the future
		{
			departure_time=simTime()+par("service_time");
	        scheduleAt(departure_time,departure);
		}
		if (queue.getLength() == 0 && !ZLICZANIE)
		{
			time_start_buffer = simTime();
			ZLICZANIE = true;
		}
	}
	else if (queue.getLength() <= queueStat.N)                   //job arrival
	{
		if (!(queue.getLength() <= queueStat.N)&&del)
		{
			hist_overflow.collect(simTime() - (((cMessage*)queue.front())->getTimestamp()));
			hist_over_aveL.collect(queueStat.deleted_in_one_row);
			EV << "DELETED IN ONE ROW: " << queueStat.deleted_in_one_row << "\n";
			follow1 = hist_overflow.getMean(); follow2 = hist_over_aveL.getMean();
			queueStat.deleted_in_one_row = 0;
		}
		EV << "ARRIVAL queue: " << queue.getLength();
		// hist_overflow.collect(queueStat.answer); // Ad2
		if (queue.isEmpty())  //if the queue is empty, the job that has just arrived has to be served immediately, i.e. the departure event of this job has to be scheduled in the future
		{
			departure_time=simTime()+par("service_time");
            scheduleAt(departure_time,departure);
		}
		if(queue.getLength() == queueStat.N-1 && ZLICZANIE){
            hist_over_aveD.collect(simTime()-time_start_buffer);
            ZLICZANIE = false;
        }
		queueStat.accepted ++;
		msgin->setTimestamp();
		queue.insert(msgin); //insert the job at the end of the queue
	}
	else
	{
		del = true;
		EV << "DELETE queue: " << queue.getLength();
		queueStat.deleted_in_one_row++;
		queueStat.deleted ++;
		delete msgin;
	}
	queueStat.answer = (double)queueStat.deleted/(queueStat.deleted + queueStat.accepted);
}