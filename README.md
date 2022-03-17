This set of processes is meant to be run on the same machine. First, extract `qjump_client.c`, `qjump_host.c`, and `Makefile` to the same directory. Then, run the `make` command in your terminal to compile `qjump_client.c` and `qjump_host.c` into the executable binaries `qjump_client` and `qjump_host`, respectively.

To run the simulator of QJump, first run `qjump_host` through the command `./qjump_host` in the directory where the binary is located. Then, on the same machine (most likely by using a new terminal window), run `qjump_client` through `./qjump_client`. It will record the latency of each message sent, printing out when a new minimum or maximum latency value is reached.

You can compare this to a situation where there is no queue jumping enabled by running the above steps, except when starting the `qjump_host`, add the argument `--no-jump`, so the start command will look like `./qjump_host --no-jump`. The client will be run in the same way.

GitHub repository link: https://github.com/natanlidukhover/CS-740-AS2