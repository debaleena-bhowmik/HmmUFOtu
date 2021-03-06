/*******************************************************************************
 * This file is part of HmmUFOtu, an HMM and Phylogenetic placement
 * based tool for Ultra-fast taxonomy assignment and OTU organization
 * of microbiome sequencing data with species level accuracy.
 * Copyright (C) 2017  Qi Zheng
 *
 * HmmUFOtu is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HmmUFOtu is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with AlignerBoost.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
/*
 * JC69.h
 *  JC69 DNA Substitution Model
 *  Created on: Mar 8, 2017
 *      Author: zhengqi
 */

#ifndef SRC_JC69_H_
#define SRC_JC69_H_

#include <cmath>
#include "DNASubModel.h"

namespace EGriceLab {
namespace HmmUFOtu {

class JC69: public DNASubModel {
public:
	/* destructor, do nothing */
	virtual ~JC69() { }

	/* member methods */
	virtual string modelType() const {
		return name;
	}

	virtual Vector4d getPi() const {
		return pi;
	}

	/**
	 * get the Prob matrix given branch length and optionally rate factor
	 * @override  the base class function
	 */
	virtual Matrix4d Pr(double v) const;

	/**
	 * Get the substitution distance given the observed fraction of differences (p-distance) using this model
	 * @override  the base class function
	 */
	virtual double subDist(const Matrix4d& D, double N) const;

	/**
	 * read in content from input stream
	 * will set badbit if anything went wrong
	 * @override  base class method
	 */
	virtual istream& read(istream& in);

	/**
	 * write this model to given output stream
	 * @override  base class method
	 */
	virtual ostream& write(ostream& out) const;

	/**
	 * train model parameters using given sets of observed base transition and frequency counts
	 * @override  base class method
	 */
	virtual void trainParams(const vector<Matrix4d>& Pv, const Vector4d& f)
	{ }

	/**
	 * copy this object and return the new object's address
	 * @override  base class method
	 */
	virtual JC69* clone() const {
		return new JC69(*this);
	}


private:

	static const string name;
	static const Vector4d pi;
};

inline Matrix4d JC69::Pr(double v) const {
	Matrix4d P = Matrix4d::Constant((1 - ::exp(-4 * v / 3)) / 4);
	P.diagonal().setConstant((1 + 3 * ::exp(-4 * v / 3)) / 4);
	return P;
}

inline double JC69::subDist(const Matrix4d& D, double N) const {
	if(N == 0)
		return 0;
	double p = (D.sum() - D.diagonal().sum()) / N; /* p-distance */
	return - 3.0 / 4.0 * ::log(1.0 - 4.0 / 3.0 * p);
}

} /* namespace HmmUFOtu */
} /* namespace EGriceLab */

#endif /* SRC_JC69_H_ */
