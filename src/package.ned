simple User
{
    gates:
        input in;
        output out;
}

simple Server
{
	gates:
	    input in;
	    output out;
}

network SimpleNet
{
	submodules:
	    user: User;
	    server: Server;
	
	connections:
	    user.out --> { delay = 100ms; } --> server.in;
	    user.in <-- { delay = 100ms; } <-- server.out;
}