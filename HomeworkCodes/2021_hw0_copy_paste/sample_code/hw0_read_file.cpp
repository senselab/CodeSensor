#include <fstream>


int main() {
	std::ifstream ifs("input.txt");
	std::ofstream ofs("output.txt");

	ofs << ifs.rdbuf();

	ifs.close();
	ofs.close();

	return 0;
}