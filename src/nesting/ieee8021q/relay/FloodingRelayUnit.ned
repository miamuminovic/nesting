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
// Whenever this implementation of a ~RelayUnit receives a packet on an in-
// port, it sends out duplicated packets on every out-port except the one
// with equal index to the in-port.
//
// @see ~RelayUnit
//
simple FloodingRelayUnit like RelayUnit {
    parameters:
        @display("i=block/switch");
        @class(FloodingRelayUnit);
        int numberOfPorts;
        bool verbose = default(false);
    gates:
        input in[numberOfPorts];
        output out[numberOfPorts];
        input managementIn[];
        output managementOut[];
}
