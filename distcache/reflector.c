#include <stdio.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h> // for signal and raise

// CPU affinity
#define _GNU_SOURCE
#include <sched.h>
#include <pthread.h>

#include "helper.h"
#include "socket_helper.h"

#include "common_impl.h"

char reflector_role[256];

char reflector_ip_for_switchos[256];
short reflector_dp2cpserver_port = -1;
short reflector_cp2dpserver_port = -1;
char reflector_cp2dp_dstip[256];
char switchos_ip[256];
int server_physical_idx = -1;

bool volatile reflector_running = false;
std::atomic<size_t> reflector_ready_threads(0);
size_t reflector_expected_ready_threads = 2;

#include "reflector_impl.h"

bool volatile killed = false;
void kill(int signum);

cpu_set_t nonserverworker_cpuset; // [server_cores, total_cores-1] for all other threads

bool validate_reflector_ip();

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Usage: ./reflector spine/leaf\n");
		exit(-1);
	}

	parse_ini("config.ini");
	parse_control_ini("control_type.ini");

  	memcpy(reflector_role, argv[1], strlen(argv[1]));

	// update global variables based on reflector.role
	if (strcmp(reflector_role, "spine") == 0) {
		memcpy(reflector_ip_for_switchos, spine_reflector_ip_for_switchos, strlen(spine_reflector_ip_for_switchos));
		reflector_dp2cpserver_port = spine_reflector_dp2cpserver_port;
		reflector_cp2dpserver_port = spine_reflector_cp2dpserver_port;
		memcpy(reflector_cp2dp_dstip, spine_reflector_cp2dp_dstip, strlen(spine_reflector_cp2dp_dstip));
		memcpy(switchos_ip, spineswitchos_ip, strlen(spineswitchos_ip));
	}
	else if (strcmp(reflector_role, "leaf") == 0) {
		memcpy(reflector_ip_for_switchos, leaf_reflector_ip_for_switchos, strlen(leaf_reflector_ip_for_switchos));
		reflector_dp2cpserver_port = leaf_reflector_dp2cpserver_port;
		reflector_cp2dpserver_port = leaf_reflector_cp2dpserver_port;
		memcpy(reflector_cp2dp_dstip, leaf_reflector_cp2dp_dstip, strlen(leaf_reflector_cp2dp_dstip));
		memcpy(switchos_ip, leafswitchos_ip, strlen(leafswitchos_ip));
	}
	else {
		printf("Invalid reflector role: %s which should be spine/leaf\n", reflector_role);
		exit(-1);
	}

	// check if reflector_ip_for_switch is one local ip in current physical machine
	if (!validate_reflector_ip()) {
		printf("[ERROR] you should launch reflector_for_%s in correct physical machine with IP %s\n", reflector_role, reflector_ip_for_switchos);
		exit(-1);
	}

	// find physical server if any based on reflector_ip_for_switchos
	for (int i = 0; i < server_physical_num; i++) {
		if (strcmp(reflector_ip_for_switchos, server_ips[i]) == 0) {
			server_physical_idx = i;
			break;
		}
	}

	// if deployed in physical server -> avoid from affecting server.workers
	if (server_physical_idx != -1) {
		CPU_ZERO(&nonserverworker_cpuset);
		for (int i = server_worker_corenums[server_physical_idx]; i < server_total_corenums[server_physical_idx]; i++) {
			CPU_SET(i, &nonserverworker_cpuset);
		}
		//int ret = sched_setaffinity(0, sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
		pthread_t main_thread = pthread_self();
		int ret = pthread_setaffinity_np(main_thread, sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
		if (ret) {
			printf("[Error] fail to set affinity of server.main; errno: %d\n", errno);
			exit(-1);
		}
	}

	// (1) prepare phase

	signal(SIGTERM, SIG_IGN); // Ignore SIGTERM for subthreads
	prepare_reflector();

	// (2) transaction phase

	pthread_t reflector_cp2dpserver_thread;
	pthread_t reflector_dp2cpserver_thread;
	// launch reflector.cp2dpserver
	int ret = pthread_create(&reflector_cp2dpserver_thread, nullptr, run_reflector_cp2dpserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error of launching reflector.cp2dpserver: " << ret);
	}
	if (server_physical_idx != -1) { // if deployed in physical server
		ret = pthread_setaffinity_np(reflector_cp2dpserver_thread, sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
		if (ret) {
			printf("Error of setaffinity for reflector.cp2dpserver; errno: %d\n", errno);
			exit(-1);
		}
	}

	// launch reflector.dp2cpserver
	ret = pthread_create(&reflector_dp2cpserver_thread, nullptr, run_reflector_dp2cpserver, nullptr);
	if (ret) {
		COUT_N_EXIT("Error of launching reflector.dp2cpserver: " << ret);
	}
	if (server_physical_idx != -1) { // if deployed in physical server
		ret = pthread_setaffinity_np(reflector_dp2cpserver_thread, sizeof(nonserverworker_cpuset), &nonserverworker_cpuset);
		if (ret) {
			printf("Error of setaffinity for reflector.dp2cpserver; errno: %d\n", errno);
			exit(-1);
		}
	}

	while (reflector_ready_threads < reflector_expected_ready_threads) sleep(1);
	
	reflector_running = true;
	printf("[reflector_for_%s] all threads ready\n", reflector_role);

	signal(SIGTERM, kill); // Set for main thread (kill -15)

	while (!killed) {
		sleep(1);
	}

	reflector_running = false;

	void *status;
	printf("wait for reflector.cp2dpserver\n");
	int rc = pthread_join(reflector_cp2dpserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error: unable to join reflector.cp2dpserver " << rc);
	}
	printf("reflector.cp2dpserver finish\n");

	printf("wait for reflector.dp2cpserver\n");
	rc = pthread_join(reflector_dp2cpserver_thread, &status);
	if (rc) {
		COUT_N_EXIT("Error: unable to join reflector.dp2cpserver " << rc);
	}
	printf("reflector.dp2cpserver finish\n");

	// (3) free phase
	close_reflector();
}

bool validate_reflector_ip() {
	struct ifaddrs *ifAddrStruct = NULL;
	void *tmpAddrPtr = NULL;

	char ip[256];
	memset(ip, 0, 256);

	bool result = false;

	// get addr info of all interfaces
	getifaddrs(&ifAddrStruct);
	struct ifaddrs *cur_ifaddrstruct_ptr = ifAddrStruct;

    while (cur_ifaddrstruct_ptr != NULL) {
		if (cur_ifaddrstruct_ptr->ifa_addr->sa_family==AF_INET) {
			tmpAddrPtr = &((struct sockaddr_in *)cur_ifaddrstruct_ptr->ifa_addr)->sin_addr;
			inet_ntop(AF_INET, tmpAddrPtr, ip, INET_ADDRSTRLEN);
			//printf("%s IP Address:%s\n", cur_ifaddrstruct_ptr->ifa_name, ip);
			if (strcmp(reflector_ip_for_switchos, ip) == 0) {
				result = true;
				break;
			}
		}
		cur_ifaddrstruct_ptr = cur_ifaddrstruct_ptr->ifa_next;
	}

	//free ifaddrs
	freeifaddrs(ifAddrStruct);
	return result;
}

void kill(int signum) {
	COUT_THIS("[transaction phase] receive SIGKILL!")
	killed = true;
}
