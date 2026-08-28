simple SimpleUser
{
    parameters:
        @signal[OffDelay](type="double");
        @statistic[offDelayStat](title="Offloading delay"; source=OffDelay; record=vector,mean,stats);
    gates:
        input in;
        output out;
}

simple SimpleServer
{
	gates:
	    input in;
	    output out;
}

channel DynDelayCh extends ned.DelayChannel
{
	parameters:
	    // Extended modules use the C++ of the base by default
	    // If a NED module inherits bith the NED and C++ class,
	    // use this instead
	    @class(DynDelayCh);
	    volatile double DynDelay @unit(s) = default(100ms);
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