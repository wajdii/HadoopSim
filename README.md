# Hadoop simulator

This program requires ns-3.15. This directory should be placed in `ns-3.15/examples/HadoopSim`.
ns-3 needs a patch to compile in C++ 11. Run `./ns3patch.sh` once to apply the patch.

## Running

Run HadoopSim
`./waf --run "HadoopSim 0 0 examples/HadoopSim/topo.json examples/HadoopSim/Trace 2 0"`

Run HadoopSim with debug option enabled
`./waf --run "HadoopSim 0 0 examples/HadoopSim/topo.json examples/HadoopSim/Trace 2 1 examples/HadoopSim/debug-output/"`

Debug HadoopSim using GDB
`./waf --run "HadoopSim" --command-template="gdb --args %s 0 0 examples/HadoopSim/topo.json examples/HadoopSim/Trace 2 0"`

Run NetSim unittest
`./waf --run HadoopNetSimUnitTest


