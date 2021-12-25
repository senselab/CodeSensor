#include <fstream>

#include "data.hpp"


void solve(tTestData* test_data)
{
	std::ofstream ofs("output.txt");
	if (!ofs.is_open()) return;

	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 5; ++j) {
			ofs << test_data->data[i][j] << ' ';
		}
		ofs << '\n';
	}

	ofs.close();

	return;
}