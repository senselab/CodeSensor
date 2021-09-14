#include <cstdio>
#include <vector>
#include <algorithm>
#include <fstream>

#include "data.hpp"


void solve(tTestData* test_data)
{
	std::ofstream ofs("output.txt");
	if (!ofs.is_open()) return;

	int ar[20000];
	for (int i = 0; i < test_data->cnt; ++i) {
		std::copy(test_data->data[i], test_data->data[i] + 20000, ar);
		std::sort(ar, ar + 20000);
		
		for (int j = 0; j < 20000; ++j) {
			ofs << ar[j] << ' ';
		}
		ofs << '\n';
	}

	ofs.close();

	return;
}