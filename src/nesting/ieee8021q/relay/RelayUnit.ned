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

package nesting.ieee8021q.relay;

//
// Implementations of this module interface should perform relaying of packets
// between input-, output- and management-connections of an ethernet switch.
//
moduleinterface RelayUnit {
    parameters:
        @display("i=block/switch");
        int numberOfPorts;
        bool verbose;
    gates:
        input in[numberOfPorts];
        output out[numberOfPorts];
        input managementIn[];
        output managementOut[];
}
