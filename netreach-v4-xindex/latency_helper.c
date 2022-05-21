#include <algorithm>

#include "latency_helper.h"
#include "helper.h"

void dump_latency(std::vector<double> latency_list, std::string label) {
	if (latency_list.size() == 0) return;

	double sum_latency = 0.0;
	for (size_t i = 0; i < latency_list.size(); i++) {
		sum_latency += latency_list[i];
	}
	double min_latency = 0.0, max_latency = 0.0, avg_latency = 0.0, tail90_latency = 0.0, tail99_latency = 0.0, median_latency = 0.0;
	std::sort(latency_list.begin(), latency_list.end());
	min_latency = latency_list[0];
	max_latency = latency_list[latency_list.size()-1];
	tail90_latency = latency_list[latency_list.size()*0.9];
	tail99_latency = latency_list[latency_list.size()*0.99];
	median_latency = latency_list[latency_list.size()/2];
	avg_latency = sum_latency / double(latency_list.size());
	COUT_THIS("[Latency Statistics of " << label << " ]");
	COUT_THIS("| min | max | avg | 90th | 99th | median | sum |");
	COUT_THIS("| " << min_latency << " | " << max_latency << " | " << avg_latency << " | " << tail90_latency \
			<< " | " << tail99_latency << " | " << median_latency << " | " << sum_latency << " |");
}
