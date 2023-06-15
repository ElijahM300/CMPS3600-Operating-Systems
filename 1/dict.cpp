//
//dict program
#include <iostream>
#include <fstream>
#include <string>
using namespace std;
const char fname[] = "/usr/share/dict/cracklib-small";
const int MAX_WORD_LENGTH = 100;

int main() {
	ifstream fin;
	fin.open(fname);
	if (fin.fail()) {
		cout << "ERROR: opening file for input\n";
		return 0;
	}
	int count = 0;
	string line;
	fin >> line;
	while (!fin.eof()) {
		++count;
		fin >> line;
	}
	cout << count << endl;
	string *arr = new string[count];
	int n=0;
	fin.clear();
	fin.seekg(ios::beg);
	fin >> line;
	while (!fin.eof()) {
		arr[n++] = line;
		fin >> line;
	}
	//
	for (int i=0; i<count; i++)
		cout << arr[i] << " ";
	cout << endl;
	fin.close();
	cout << endl;
	return 0;
}


















