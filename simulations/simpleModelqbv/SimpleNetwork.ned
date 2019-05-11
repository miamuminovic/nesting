//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package nesting.simulations.simpleModelqbv;

import inet.examples.ospfv2.fulltest.C;
import nesting.node.ethernet.VlanEtherHostFullLoad;
import nesting.node.ethernet.VlanEtherHostQ;
import nesting.node.ethernet.VlanEtherHostSched;
import nesting.node.ethernet.VlanEtherSwitchPreemptable;


//
// TODO auto-generated type
//
network SimpleNetwork
{
    @display("bgb=392,282");
    submodules:
        switch: VlanEtherSwitchPreemptable {
            parameters:
            	@display("p=194,119");
            gates:
                ethg[3];
        }
        ST: VlanEtherHostSched {
            @display("p=76,62");
        }
        Sink: VlanEtherHostQ {
            @display("p=319,120");
        }
        BE: VlanEtherHostFullLoad {
            @display("p=76,170");
            
        }
    connections:
        ST.ethg <--> C <--> switch.ethg[0];
        switch.ethg[2] <--> C <--> Sink.ethg;
        BE.ethg <--> C <--> switch.ethg[1];
}
