simple SimpleUser
{
    parameters:
        @signal[OffDelay](type="double");
        @statistic[offDelayStat](title="Offloading delay"; source=OffDelay; record=vector,mean,stats);

        double tasksPerSecond = default(10);
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
	    @statistic[QueueFullStat](title="Queue full events"; source=QueueFull; record=vector,count);
	
	    int CPU_CYCLES = default(100);
	    int N_CORES = default(1);
	    int QUEUE_SIZE = default(100);
	gates:
	    input in;
	    output out;
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
	submodules:
	    user: SimpleUser;
	    server: SimpleServer;
	
	connections:
	    user.out --> DynDelayCh --> server.in;
	    user.in <-- DynDelayCh <-- server.out;
}