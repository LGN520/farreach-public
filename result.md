# Result

## Preliminary Exp of microbench

- Latency
	* Read-only
		- `./prepare; ./client`
		- `./prepare; ./server`
		- XIndex: 19852 op/s = 50.37us
		- NetBuffer: 19874 op/s = 50.32us
	* 90% read - 10% insert
		- `./prepare -b 0.1; ./client -a 0.9 -b 0.1`
		- `./prepare; ./server`
		- XIndex: 19857 op/s = 50.36us
		- NetBuffer: 19833 op/s = 50.42us
	* 80% read - 10% insert - 10% remove
		- `./prepare -b 0.1; ./client -a 0.8 -b 0.1 -c 0.1`
		- `./prepare; ./server`
		- XIndex: 19867 op/s = 50.33us
		- NetBuffer: 19845 op/s = 50.39us
	* 70% read - 10% insert - 10% remove - 10% update
		- `./prepare -b 0.1; ./client -a 0.7 -b 0.1 -c 0.1 -d 0.1`
		- `./prepare; ./server`
		- XIndex: 19841 op/s = 50.4us
		- NetBuffer: 19780 op/s = 50.56us
	* 60% read - 10% insert - 10% remove - 10% update - 10% scan
		- `./prepare -b 0.1; ./client -a 0.6 -b 0.1 -c 0.1 -d 0.1 -e 0.1`
		- `./prepare; ./server`
		- XIndex: 19843 op/s = 50.39us
