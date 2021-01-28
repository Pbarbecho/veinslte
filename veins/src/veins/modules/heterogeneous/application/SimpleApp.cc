//
// Copyright (C) 2006-2016 Florian Hagenauer <hagenauer@ccs-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "SimpleApp.h"

Define_Module(SimpleApp);

void SimpleApp::initialize(int stage) {
	if (stage == 0) {
		debug = par("debug").boolValue();
		infoLogging = par("infoLogging");
		toDecisionMaker = findGate("toDecisionMaker");
		fromDecisionMaker = findGate("fromDecisionMaker");

		cModule *tmpMobility = getParentModule()->getSubmodule("veinsmobility");
		Veins::TraCIMobility* mobility = dynamic_cast<Veins::TraCIMobility *>(tmpMobility);
		ASSERT(mobility);
		sumoId = mobility->getExternalId();

        APP_filename = par("APP_filename").stdstringValue();
		/* Don't schedule at the same time to prevent synchronization */
		scheduleAt(simTime() + uniform(0, 1), new cMessage("Send"));
	}
}


std::string SimpleApp::getRandomDestination() {
    /* choose a random other car to send the message to */
    TraCIScenarioManager* manager = TraCIScenarioManagerAccess().get();
    std::map<std::string, cModule*> hosts = manager->getManagedHosts();
    std::map<std::string, cModule*>::iterator it = hosts.begin();
    std::advance(it, intrand(hosts.size()));
    if (hosts.size() <= 1){std::cerr<<simTime()<<"  Just one car in the simulation !!!!!!!!!!"<<endl;}
    std::string dst_add = it->first;
    //std::string destination("node[" + dst_add + "]");
    return dst_add;
}


void SimpleApp::handleMessage(cMessage *msg) {
	if (msg->isSelfMessage()) {
	    /*CREO MENSAJE EN EL CAR HACIA OTRO CAR O HACIA EL SERVER VIA LTE*/

	    /*
		 * Send a message to a random node in the network. Note that only the most necessary values
		 * are set. Size of the message have to be set according to the real message (aka your used
		 * .msg file). The values here are just a temporary placeholder.
		 */

	    /*Get a random node to send the message*/
        cModule *tmpMobility = getParentModule()->getSubmodule("veinsmobility");
        Veins::TraCIMobility* mobility = dynamic_cast<Veins::TraCIMobility *>(tmpMobility);
        std::string current_node_id = mobility->getExternalId();
        std::string destination = getRandomDestination();
        /*=========================================================================================*/


        /*Check if the destination is the current node*/
        if (current_node_id != destination){

            /*=========================================================================================*/
            /*======================== SEND MESSAGE  TO NODE VIA DSRC =================================*/
            /*=========================================================================================*/

            std::string destinationID("node[" + destination + "]"); // destination afdddres is later check as node[x]

            /* get current euclidean distance from current vehicle to enb */
            Coord curr_veh_pos = mobility->getCurrentPosition();
            /*Fixed enb position (200,200,0)*/
            Coord eNB;eNB.x = 200;eNB.y = 200;eNB.z = 0;
            TraCIScenarioManager* manager = TraCIScenarioManagerAccess().get();
            double dst_to_enb = manager->getCommandInterface()->getDistance(curr_veh_pos,eNB,false);
            /*=========================================================================================*/

            /*Prepare the message to send*/
            HeterogeneousMessage *testMessage = new HeterogeneousMessage();
            /*Decide type of message*/
            //if (dst_to_enb <= 200){testMessage->setNetworkType(LTE);}
            //else{testMessage->setNetworkType(DSRC);}
            testMessage->setNetworkType(DSRC);
            testMessage->setName("Heterogeneous Test Message");
            testMessage->setByteLength(10);
            INFO_ID("TX MSG TO " << destinationID << " from  node["<<sumoId<<"]  D:"<<dst_to_enb<<"  NT:"<<testMessage->getNetworkType());
            testMessage->setDestinationAddress(destinationID.c_str());
            /* Finish the message and send it */
            testMessage->setSourceAddress(sumoId.c_str());

            /* Pablo capture send msg - Use sorce and destination ! recipient and sender not defined fields*/
            CaptureMSG("car", "tx",sumoId,
                    testMessage->getNetworkType(),
                    testMessage->getSourceAddress(),
                    testMessage->getDestinationAddress(),
                    testMessage->getTreeId());
            /*=========================================================================================*/

            /*Send message to lower layer DesionMaker*/
            send(testMessage, toDecisionMaker);

        }

        /*=========================================================================================*/
        /*======================== SEND MESSAGE  TO SERVIE VIA LTE =================================*/
        /*=========================================================================================*/

        /*
         * PABLO CAMBIO ESTO A QUE SIEMPRE ENVIE AL NODE VIA DSRC Y AL SERVER LTE
         * At 25% of the time send also a message to the main server. This message is sent via LTE
         * and is then simply handed to the decision maker.
        */

        //if(dblrand() < 0.25){
        INFO_ID("TX MSG TO SERVER");
        HeterogeneousMessage* serverMessage = new HeterogeneousMessage();
        serverMessage->setName("Server Message Test");
        serverMessage->setByteLength(10);
        serverMessage->setNetworkType(LTE); //SIEMPRE AL SERVER POR LTE
        serverMessage->setDestinationAddress("server");
        serverMessage->setSourceAddress(sumoId.c_str());
        send(serverMessage, toDecisionMaker);

        /* Pablo capture send msg - Use sorce and destination ! recipient and sender not defined fields*/
        CaptureMSG("car", "tx",sumoId,
                serverMessage->getNetworkType(),
                serverMessage->getSourceAddress(),
                serverMessage->getDestinationAddress(),
                serverMessage->getTreeId());

        /*=========================================================================================*/
        //}

        /*Agenda un nuevo mensaje en un segundo*/
        scheduleAt(simTime() + 1, new cMessage("Send"));
        /*=========================================================================================*/

	} else {
	    /*=========================================================================================*/
	    /*RECIBO MENSAJE EN EL CAR - SE CONTABILIZAN LOS MSGs RECIBIDOS CAR Y EN EL SERVERB*/
	    /*=========================================================================================*/
	    HeterogeneousMessage *testMessage = dynamic_cast<HeterogeneousMessage *>(msg);
		INFO_ID("RX MSG " << " from  node[" << testMessage->getSourceAddress() <<"]  NT:"<<testMessage->getNetworkType());



		/* Pablo capture RECEIVED msg */
		CaptureMSG("car","rx", sumoId,
                testMessage->getNetworkType(),
                testMessage->getSourceAddress(),
                testMessage->getDestinationAddress(),
                testMessage->getTreeId());
	    /*=========================================================================================*/
	}
}


void SimpleApp::CaptureMSG(std::string node, std::string state, std::string nodeid, int type, std::string source, std::string destination, int msgID)
{
    MSG_file.open(APP_filename, std::ios::out | std::ios::app);                                    //para leer datos ios::in
    if (MSG_file.is_open()){
        MSG_file<<node<<","<<state<<","<<nodeid<<","<<type<<"," << source <<","<<destination<<","<<msgID<<","<< simTime().dbl()<<'\n';
        MSG_file.close();
    }
    else std::cerr << "ERROR NO SE PUEDE ABRIR EL ARCHIVO"<< endl;
}
