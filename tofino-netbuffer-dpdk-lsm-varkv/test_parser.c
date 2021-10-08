#include "ycsb/parser.h"

int main(int argc, char **argv) {
	char filename[256] = "tmp.out";
	Parser parser(filename);
	ParserIterator iter = parser.begin();
	while (true) {
		COUT_THIS("type: " << int(iter.type()) <<" key: "<<iter.key().to_string()<<" val: "<<iter.val().to_string())
		if (!iter.next()) {
			break;
		}
	}
}
