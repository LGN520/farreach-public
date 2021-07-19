# Result

## Preliminary Exp of microbench

- Latency
	+ Tofino + XIndex
		* Read-only
			- `./prepare; ./client`
			- `./prepare; ./server`
			- Result: 19852 op/s = 50.37us
		* 90% read - 10% insert
			- `./prepare -b 0.1; ./client -a 0.9 -b 0.1`
			- `./prepare; ./server`
			- Result: 19857 op/s = 50.36us
		* 80% read - 10% insert - 10% remove
			- `./prepare -b 0.1; ./client -a 0.8 -b 0.1 -c 0.1`
			- `./prepare; ./server`
			- Result: 19867 op/s = 50.33us
		* 70% read - 10% insert - 10% remove - 10% update
			- `./prepare -b 0.1; ./client -a 0.7 -b 0.1 -c 0.1 -d 0.1`
			- `./prepare; ./server`
			- Result: 19841 op/s = 50.4us
		* 60% read - 10% insert - 10% remove - 10% update - 10% scan
			- `./prepare -b 0.1; ./client -a 0.6 -b 0.1 -c 0.1 -d 0.1 -e 0.1`
			- `./prepare; ./server`
			- Result: 19843 op/s = 50.39us
	+ Tofino-based NetBuffer + XIndex
