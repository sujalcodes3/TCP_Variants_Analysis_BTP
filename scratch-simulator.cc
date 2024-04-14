/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ScratchSimulator");

void handler(int arg0, int arg1) {
    std::cout << "handler called with argument arg0=" << arg0 << " and arg1=" << arg1 << std::endl;
}

int
main(int argc, char* argv[])
{
    NS_LOG_UNCOND("Scratch Simulator");

    Simulator::Schedule(Seconds(10), &handler, 5, 1);
    Simulator::Schedule(Seconds(5), &handler, 1, 5);

    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
