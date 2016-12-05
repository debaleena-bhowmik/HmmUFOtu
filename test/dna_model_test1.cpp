/*
 * dna_model_test1.cpp
 *
 *  Created on: Sep 22, 2016
 *      Author: zhengqi
 */

#include <iostream>
#include <fstream>
#include "MSA.h"
#include "PhyloTree.h"
#include "DNASubModel.h"
#include "GTR.h"

using namespace std;
using namespace EGriceLab;

int main(int argc, const char* argv[]) {
	if(argc != 4) {
		cerr << "Usage:  " << argv[0] << " TREE-INFILE MSA-INFILE OUTFILE" << endl;
		return -1;
	}

	ifstream msaIn(argv[2]);
	ofstream out(argv[3]);
	if(!msaIn.is_open()) {
		cerr << "Unable to open " << argv[2] << endl;
		return -1;
	}

	if(!out.is_open()) {
		cerr << "Unable to write to " << argv[3] << endl;
		return -1;
	}

	MSA msa;
	if(msa.load(msaIn))
		cerr << "MSA database loaded" << endl;
	else {
		cerr << "Unable to load MSA database" << endl;
		return -1;
	}

	EGriceLab::PT tree;
	int nRead = tree.readTree(argv[1], "newick", msa);
	if(nRead != -1)
		cerr << "Read in PhyloTree with " << nRead << " assigned seq" << endl;
	else {
		cerr << "Unable to read PhyloTree" << endl;
		return -1;
	}

	/* train an GTR model */
	GTR model;
	model.trainParams(tree);

	cerr << "GTR model trained" << endl;
	out << model;
}
