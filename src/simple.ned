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
	gates:
	    input in;
	    output out;
}

channel DynDelayCh extends ned.DatarateChannel
{
	parameters:
	    // Extended modules use the C++ of the base by default
	    // If a NED module inherits bith the NED and C++ class,
	    // use this instead
	    @class(DynDelayCh);
	    volatile double DynDelay @unit(s) = default(100ms);
	
	    datarate = 1Mbps;
	    delay = 10ms;
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