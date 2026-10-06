// Module interface for mobility sim
moduleinterface IMobile
{
	parameters:
	    int posx;
	    int posy;
	    double velocity;
	    double heading;
	
	gates:
	    input in;
	    output out;
}

simple SimpleUser like IMobile //TODO: Compound modules are needed. But first lets do it with only a simple mod.
{
    parameters:
        @signal[OffDelay](type="double");
        @signal[QueueLength](type="int");
        @signal[QueueFull](type="int");
        @statistic[offDelayStat](title="Offloading delay"; source=OffDelay; record=vector,mean,stats);
	    @statistic[QueueLengthStat](title="Qeueue length"; source=QueueLength; record=vector,mean);
	    @statistic[QueueFullStat](title="Queue full events"; source=QueueFull; record=vector);

        double tasksGenPeriod @unit(s) = default(100ms);

	    int CPU_CYCLES = default(300); //MHz
	    int QUEUE_SIZE = default(10000); //Tasks
	
	    int MIN_TASK_S = default(1000); //Bytes
	    int MAX_TASK_S = default(10000); //Bytes
	
	    double P_OFFLOAD = default(1); // From 0 to 1
	
	    volatile double traffic_gen = default(exponential(1));
	
	    int posx = default(0);
	    int posy = default(0);
	    double velocity = default(0);
	    double heading = default(0);
	    int pivotRadius = default(200);
	
	    double txPower @unit(W) = default(100mW);
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
	
	    int CPU_CYCLES = default(2400); //MHz
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
	    volatile double DynDelay @unit(s) = default(0ms);
	
	    datarate = default(1Mbps);
}

network SimpleNet
{
    parameters:
        int N_USERS = default(1);
	submodules:
	    user[N_USERS]: <default("SimpleUser")> like IMobile;
	    server: SimpleServer;
	
	connections:
	    for i=0..N_USERS-1{
	    	user[i].out --> DynDelayCh --> server.in++;
	  		user[i].in <-- DynDelayCh <-- server.out++;
	    }
}