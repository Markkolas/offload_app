// Module interface for mobility sim
//moduleinterface IMobility
//{
//	parameters:
//	    int posx;
//	    int posy;
//	    double velocity;
//	    double heading;
//}
//
//simple RandomPivot like IMobility
//{
//    parameters:
//	    int posx = default(0);
//	    int posy = default(0);
//	    double velocity = default(0);
//	    double heading = default(0);
//	    int pivotRadius = default(1);
//}

simple SimpleUser
{
    parameters:
        @signal[OffDelay](type="double");
        @signal[QueueLength](type="int");
        @signal[QueueFull](type="int");
        @statistic[offDelayStat](title="Offloading delay"; source=OffDelay; record=vector,mean,stats);
	    @statistic[QueueLengthStat](title="Qeueue length"; source=QueueLength; record=vector,mean);
	    @statistic[QueueFullStat](title="Queue full events"; source=QueueFull; record=vector);

        double tasksPerSecond = default(10);

	    int CPU_CYCLES = default(300); //MHz
	    int QUEUE_SIZE = default(10000); //Tasks
	
	    int MIN_TASK_S = default(1000); //Bytes
	    int MAX_TASK_S = default(10000); //Bytes
    gates:
        input in;
        output out;
}

simple SimpleServer
{
    parameters:
        @signal[QueueLength](type="int");
        @signal[QueueFull](type="int");
	    @statistic[QueueLengthStat](title="Qeueue length"; source=QueueLength; record=vector,mean);
	    @statistic[QueueFullStat](title="Queue full events"; source=QueueFull; record=vector);
	
	    int CPU_CYCLES = default(300); //MHz
	    int N_CORES = default(1);
	    int QUEUE_SIZE = default(10000); //Tasks
	    int MAX_CLIENTS = default(32);
	gates:
	    input in[];
	    output out[];
}

channel DynDelayCh extends ned.DatarateChannel
{
	parameters:
	    // Extended modules use the C++ of the base by default
	    // If a NED module inherits both the NED and C++ class,
	    // use @class directive instead:
	    @class(DynDelayCh);
	    volatile double DynDelay @unit(s) = default(100ms);
	
	    datarate = default(1Mbps);
	    delay = default(10ms);
}

network SimpleNet
{
    parameters:
        int N_USERS = default(1);
	submodules:
	    user[N_USERS]: SimpleUser;
	    server: SimpleServer;
	
	connections:
	    for i=0..N_USERS-1{
	    	user[i].out --> DynDelayCh --> server.in++;
	  		user[i].in <-- DynDelayCh <-- server.out++;
	    }

}