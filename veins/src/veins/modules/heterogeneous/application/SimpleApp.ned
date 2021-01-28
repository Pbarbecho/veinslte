package org.car2x.veins.modules.heterogeneous.application;

//
// A simple application that just sends messages.
//
simple SimpleApp {
	parameters:
		bool debug = default(false);
		bool infoLogging = default(true);
		string APP_filename = default("/root/Documents/EPN/msgs.txt");
    gates:
        output toDecisionMaker;
        input  fromDecisionMaker;
}
