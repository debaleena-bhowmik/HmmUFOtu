/*
 * NT_IO_test1.cpp
 *
 *  Created on: Dec 2, 2016
 *      Author: zhengqi
 */

#include <iostream>
#include <boost/config/warning_disable.hpp>
#include "NewickTree.h"
using namespace std;

int main(int argc, const char* argv[]) {

	namespace qi = boost::spirit::qi;
	std::string str;
	EGriceLab::NT tree;

	EGriceLab::newick_grammar<std::string::const_iterator> grammar;

	cout << "Type in a newick tree or q to exit" << endl;

	while(std::getline(std::cin, str)) {
		if(str.empty() || str[0] == 'q' || str[0] == 'Q')
			break;

		// parse
		std::string::const_iterator iter = str.begin();
		std::string::const_iterator end = str.end();
		bool result = qi::phrase_parse(iter, end, grammar, qi::space, tree);

		if(result && iter == str.end()) {
			std::cout << "Parsing succeeded. Tree:" << endl <<
					tree << std::endl;
		}
		else {
			std::cout << "Parsing failed. Stopped at: " << std::string(iter, end) << std::endl;
			return -1;
		}
	}

	std::cout << "Bye... :-)\n\n";
	return 0;
}
