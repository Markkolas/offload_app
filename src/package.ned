simple SimpleUser
{
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

network SimpleNet
{
	submodules:
	    user: SimpleUser;
	    server: SimpleServer;
	
	connections:
	    user.out --> { delay = 100ms; } --> server.in;
	    user.in <-- { delay = 100ms; } <-- server.out;
}