/*
 * PhyloTreeUnrooted.h
 *  An Unrooted Phylogenic Tree (PTUnrooted)
 *  A PTUnrooted can be evaluated from any node as its root and yields same cost
 *  as long as using a time-reversible DNA substitution model
 *  Internal Tree nodes are number indexed from 0 to N-1
 *  Created on: Dec 1, 2016
 *      Author: zhengqi
 */

#ifndef SRC_PHYLOTREEUNROOTED_H_
#define SRC_PHYLOTREEUNROOTED_H_

#include <string>
#include <vector>
#include <limits>
#include <iostream>
#include <stdexcept>
#include <cstddef>
#include <cstdlib>
#include <cassert>
#include <eigen3/Eigen/Dense>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/unordered_map.hpp>

#include "HmmUFOtuConst.h"
#include "ProgLog.h"
#include "StringUtils.h"
#include "SeqCommons.h"
#include "DigitalSeq.h"
#include "NewickTree.h"
#include "MSA.h"
#include "DNASubModel.h"

namespace EGriceLab {
using std::string;
using std::vector;
using std::map;
using std::istream;
using std::ostream;
using Eigen::Matrix4Xd;
using Eigen::Matrix4d;
using Eigen::RowVectorXd;
using boost::shared_ptr;
using boost::unordered_map;

class PhyloTreeUnrooted; /* forward declaration */

typedef PhyloTreeUnrooted PTUnrooted;

class PhyloTreeUnrooted {
public:
	/* nested types and enums */

	struct PhyloTreeUnrootedNode;
	typedef PTUnrooted::PhyloTreeUnrootedNode PTUNode;

	typedef shared_ptr<PTUNode> PTUNodePtr; /* use boost shared_ptr to hold node pointers */
	typedef shared_ptr<const PTUNode> PTUNodeConstPtr; /* use boost shared_ptr to hold node pointers */

	typedef shared_ptr<DNASubModel> ModelPtr; /* use boost shared_ptr to hold DNA Sub Model */

	/**
	 * A PTUnrooed node that stores its basic information and neighbors
	 */
	class PhyloTreeUnrootedNode {
		friend class PhyloTreeUnrooted;

	public:
		/* constructors */
		/**
		 * Default constructor, do nothing
		 */
		PhyloTreeUnrootedNode() : id(0), annoDist(0) {	}

		/**
		 * Construct a PTUNode with a given name and id
		 */
		explicit PhyloTreeUnrootedNode(long id, const string& name = "")
		: id(id), name(name), annoDist(0) {  }

		/**
		 * Construct a PTUNode with a given id, name, sequence, and optionally annotation and annotation-dist
		 */
		PhyloTreeUnrootedNode(long id, const string& name, const DigitalSeq& seq,
				const string& anno = "", double annoDist = 0)
		: id(id), name(name), seq(seq), anno(anno), annoDist(annoDist)
		{ }

		/**
		 * Construct a PTUNode with a given id, name, unobserved seq with given length, and optionally annotation and annotation-dist
		 */
		PhyloTreeUnrootedNode(long id, const string& name, size_t length,
				const string& anno = "", double annoDist = 0)
		: id(id), name(name), seq(SeqCommons::nuclAbc, name), anno(anno), annoDist(annoDist)
		{
			seq.append(length, DegenAlphabet::GAP_SYM);
		}

		/* Member methods */
		/* Getters and Setters */
		const string& getAnno() const {
			return anno;
		}

		double getAnnoDist() const {
			return annoDist;
		}

		long getId() const {
			return id;
		}

		const string& getName() const {
			return name;
		}

		const PTUNodePtr& getParent() const {
			return parent;
		}

		const DigitalSeq& getSeq() const {
			return seq;
		}

		/** test whether this node is named */
		bool isNamed() const {
			return !name.empty();
		}

		/** test whether this is a leave node */
		bool isLeaf() const {
			return neighbors.size() == 1;
		}

		/** test whether this is an internal node */
		bool isInternal() const {
			return neighbors.size() > 1;
		}

		/** test whether this is a root node */
		bool isRoot() const {
			return parent == NULL;
		}

		/** test whether this node is parent of another node */
		bool isParent(const PTUNodePtr& other) const {
			return other != NULL && this == other->parent.get();
		}

		/** test whether this node is child of another node */
		bool isChild(const PTUNodePtr& other) const {
			return parent == other;
		}

		/**
		 * test whether this is a tip node
		 * all children of a tip must be leaves
		 */
		bool isTip() const {
			if(isLeaf())
				return false;
			for(vector<PTUNodePtr>::const_iterator child = neighbors.begin(); child != neighbors.end(); ++child)
				if(isParent(*child) /* this is really a child */
						&& !(*child)->isLeaf())
					return false;
			return true;
		}

		/**
		 * get children of this node
		 * children nodes are neighbors excluding the parent
		 */
		vector<PTUNodePtr> getChildren() const {
			vector<PTUNodePtr> children (neighbors);
			children.erase(std::remove(children.begin(), children.end(), parent), children.end());
			return children;
		}

		/**
		 * get first child of this node
		 * return NULL if not exists
		 */
		PTUNodePtr firstChild() const {
			for(vector<PTUNodePtr>::const_iterator child = neighbors.begin(); child != neighbors.end(); ++child)
				if(isParent(*child)) // this is really a child
					return *child;
			return NULL;
		}

		/**
		 * get last child of this node
		 * return NULL if not exists
		 */
		PTUNodePtr lastChild() const {
			for(vector<PTUNodePtr>::const_reverse_iterator child = neighbors.rbegin(); child != neighbors.rend(); ++child)
				if(isParent(*child)) // this is really a child
					return *child;
			return NULL;
		}

		/**
		 * get first leaf as an offspring of this node
		 */
		const PTUNode* firstLeaf() const {
			const PTUNode* node = this; /* search from this node */
			while(!node->isLeaf())
				node = node->firstChild().get();
			return node;
		}

		/**
		 * get first leaf as an offspring of this node
		 */
		PTUNode* firstLeaf() {
			PTUNode* node = this; /* search from this node */
			while(!node->isLeaf())
				node = node->firstChild().get();
			return node;
		}

		/**
		 * get the last leaf as an offspring of this node
		 */
		const PTUNode* lastLeaf() const {
			const PTUNode* node = this; /* search from this node */
			while(!node->isLeaf())
				node = node->lastChild().get();
			return node;
		}

		/**
		 * get the last leaf as an offspring of this node
		 */
		PTUNode* lastLeaf() {
			PTUNode* node = this; /* search from this node */
			while(!node->isLeaf())
				node = node->lastChild().get();
			return node;
		}

		/**
		 * get a random leaf as an offspring of this node
		 * you need to call srand() in your main program
		 */
		const PTUNode* randomLeaf() const {
			const PTUNode* node = this; /* search from this node */
			while(!node->isLeaf()) {
				const vector<PTUNodePtr>& children = node->getChildren();
				node = children[rand() % children.size()].get();
			}
			return node;
		}

		/**
		 * get a random leaf as an offspring of this node
		 * you need to call srand() in your main program
		 */
		PTUNode* randomLeaf() {
			PTUNode* node = this; /* search from this node */
			while(!node->isLeaf()) {
				const vector<PTUNodePtr>& children = node->getChildren();
				node = children[rand() % children.size()].get();
			}
			return node;
		}

		int numNeighbors() const {
			return neighbors.size();
		}

	private:
		/**
		 * save this node information but not its edges to a binary file
		 */
		istream& load(istream& in);

		/**
		 * load the node information but not its edges from a binary file
		 */
		ostream& save(ostream& out) const;

		/* member fields */
		long id; /* a unique id for each node */
		string name; /* node name, need to be unique for database loading */
		DigitalSeq seq; /* sequence of this node */
		vector<PTUNodePtr> neighbors; /* pointers to neighbors */
		PTUNodePtr parent; /* pointer to parent node, set to null on default */

		string anno;
		double annoDist;
	};

	typedef unordered_map<PTUNodePtr, unordered_map<PTUNodePtr, Matrix4Xd> > CostMap;
	typedef unordered_map<PTUNodePtr, unordered_map<PTUNodePtr, double> > BranchLenMap;

	/* constructors */
	/** Default constructor, do nothing */
	PhyloTreeUnrooted() : csLen(0) {  }

	/** Construct a PTUnrooted from a Newick Tree */
	PhyloTreeUnrooted(const NewickTree& ntree);

public:

	/* member methods */

	/** Get the number of nodes of this tree */
	size_t numNodes() const {
		return id2node.size();
	}

	size_t numEdges() const;

	/** Get number of leaves in this tree */
	size_t numLeaves() const;

	/** get number of aligned sites */
	int numAlignSites() const {
		return csLen;
	}

	/** Get root node */
	const PTUNodePtr& getRoot() const {
		return root;
	}

	/* get all nodes */
	std::vector<PTUNodePtr> getNodes() const {
		return id2node;
	}

	/* get node i */
	PTUNodePtr getNode(std::vector<PTUNodePtr>::size_type i) const {
		return id2node[i];
	}

	/**
	 * get branch length from u -> v
	 * @return  -1 if not exists
	 */
	double getBranchLength(const PTUNodePtr& u, const PTUNodePtr& v) const;

	/**
	 * get branch cost from u->v, before convoluted into the Pr(length)
	 * @return uinitiated matrix if not exists
	 */
	Matrix4Xd getBranchCost(const PTUNodePtr& u, const PTUNodePtr& v) const;

	/** Load sequences from MSA into this this */
	long loadMSA(const MSA& msa);

	/**
	 * Set the underlying DNA Sub Model as a copy of given model
	 */
	void setModel(const DNASubModel& model) {
		this->model.reset(model.clone());
	}

	/**
	 * Set the underlying DNA Sub Model as the owner of this object
	 */
	void setModel(DNASubModel* model) {
		this->model.reset(model);
	}

	/**
	 * Set the underlying DNA Sub Model as a copy of this object
	 */
	void setModel(const DNASubModel* model) {
		this->model.reset(model->clone());
	}

	/**
	 * Get the underlying DNA Sub Model
	 */
	const DNASubModel& getModel() const {
		return *model;
	}

	/**
	 * save this object to an output in binary format
	 */
	ostream& save(ostream& out) const;

	/** load a binary file into this object */
	istream& load(istream& in);

	/**
	 * set tree root at given node, return the old node
	 */
	PTUNodePtr setRoot(const PTUNodePtr& newRoot);

	/**
	 * set tree root at the ith node, return the old node id
	 */
	size_t setRoot(size_t newRootId) {
		return setRoot(id2node[newRootId])->id;
	}

	/**
	 * test whether the cost (message) of node u -> v of all site j has been evaluated
	 */
	bool isEvaluated(const PTUNodePtr& u, const PTUNodePtr& v) const;

	/**
	 * test whether the cost (message) of node u -> v of site j has been evaluated
	 */
	bool isEvaluated(const PTUNodePtr& u, const PTUNodePtr& v, int j) const;

	/**
	 * initiate the cached incoming cost of between every node u and every neighbor v
	 */
	void initInCost();

	/**
	 * initiate the leaf cost
	 */
	void initLeafCost();

	/**
	 * reset the cached cost of edge u->v
	 */
	void resetCost(const PTUNodePtr& u, const PTUNodePtr& v) {
		node2cost[u][v].setConstant(INVALID_COST);
	}

	/**
	 * reset the cached cost of every node
	 */
	void resetCost();

	/**
	 * reset the cached leaf cost
	 */
	void resetLeafCost() {
		leafCost.setConstant(INVALID_COST);
	}

	/**
	 * evaluate the log-likelihood (cost) of the entire tree
	 * @return  cost matrix of the entire tree
	 */
	Matrix4Xd cost() {
		return cost(root);
	}

	/**
	 * evaluate the log-likelihood (cost) at the jth site of the entire tree
	 * @return  cost vector at the jth site
	 */
	Vector4d cost(int j) {
		return cost(root, j);
	}

	/**
	 * evaluate the conditional cost of a subtree, rooted at given node
	 * @param node  subtree root
	 * @return  conditional cost matrix of the subtree
	 */
	Matrix4Xd cost(const PTUNodePtr& node);

	/**
	 * evaluate the conditional cost of the jth site of a subtree, rooted at given node
	 * @param node  subtree root
	 * @param j  the jth aligned site
	 * @return  conditional cost at the jth site
	 */
	Vector4d cost(const PTUNodePtr& node, int j);

	/**
	 * evaluate the entire tree
	 */
	void evaluate() {
		evaluate(root);
	}

	/**
	 * evaluate the subtree at given node
	 */
	void evaluate(const PTUNodePtr& node) {
		for(int j = 0; j < csLen; ++j)
			evaluate(node, j);
	}

	/**
	 * evaluate every child of this node at the jth site of a subtree, rooted at given node
	 * but does not calculate the cost of this node itself
	 * @param node  subtree root
	 * @param j  the jth aligned site
	 */
	void evaluate(const PTUNodePtr& node, int j);

	/**
	 * calculate the entire tree cost
	 */
	double treeCost();

	/**
	 * calculate the entire tree cost at given region
	 * both start and end are 0-based inclusive
	 */
	double treeCost(int start, int end);

	/**
	 * calculate the entire tree cost at j-th site
	 * evaluate the tree if neccessary
	 */
	double treeCost(int j);

	/**
	 * write this PTUnrooted tree structure into output in given format
	 */
	ostream& writeTree(ostream& out, string format = "newick") const;

	/**
	 * write this PTUnrooted tree structure with given root recursively into output in newick format
	 */
	ostream& writeTreeNewick(ostream& out, const PTUNodePtr& node) const;

	/**
	 * get a transition dataset for parameter traning of a DNA Substitution model
	 * using one of two well studied method, "Gojobori" or "Goldman"
	 */
	vector<Matrix4d> getModelTransitionSet(string method = "Gojobori") const;

	/**
	 * get training dataset for model parameters training of a DNA Substitution model
	 * using one of two well studied method, "Gojobori" or "Goldman"
	 */
	vector<Matrix4d> getModelTraningSetGojobori() const;

	/**
	 * get training dataset for model parameters training of a DNA Substitution model
	 * using one of two well studied method, "Gojobori" or "Goldman"
	 */
	vector<Matrix4d> getModelTraningSetGoldman() const;

	/**
	 * get estimated base frequency (pi) using this tree
	 */
	Vector4d getModelFreqEst() const;

	/**
	 * make a copy of subtree with only two nodes and a branch u and v
	 * edges u->v and v->u should has already been evaluated
	 * @return  a new PhyloTreeUnrooted with only two nodes and a branch u->v,
	 * with root set as v
	 */
	PTUnrooted copySubTree(const PTUNodePtr& u, const PTUNodePtr& v) const;

	/**
	 * iteratively optimize the length of branch u->v using Felsenstein's algorithm
	 * in given CSRegion [start-end]
	 * return the updated branch length v
	 */
	double optimizeBranchLength(const PTUNodePtr& u, const PTUNodePtr& v, int start, int end);

	/**
	 * iteratively optimize the length of branch u->v using Felsenstein's algorithm
	 * return the updated branch length v
	 */
	double optimizeBranchLength(const PTUNodePtr& u, const PTUNodePtr& v) {
		return optimizeBranchLength(u, v, 0, csLen - 1);
	}

	/**
	 * place an additional seq (n) at given branch with given initial branch length
	 * after placement, tree will be re-rooted to the new internal root r,
	 * and new branch length n->r will be optimized according the given region [start, end]
	 * @param  new seq to be placed
	 * @param u  branch start (u->v)
	 * @param v  branch end (u->v)
	 * @param start  seq start position (non-gap start)
	 * @param end  seq end position (non-gap end)
	 * @param d0  estimiated initial branch length n->r
	 * @return  the modified tree, with r and n appended at the end of all nodes
	 */
	PTUnrooted& placeSeq(const DigitalSeq& seq, const PTUNodePtr& u, const PTUNodePtr& v,
			double d0, int start, int end);

	/**
	 * place an additional seq (n) at given branch with given initial branch length
	 * after placement, tree will be re-rooted to the new internal root r,
	 * and new branch length n->r will be optimized according the entire region [0, csLen-1]
	 * @param  new seq to be placed
	 * @param u  branch start (u->v)
	 * @param v  branch end (u->v)
	 * @param d0  estimiated initial branch length n->r
	 * @return  the modified tree, with r and n appended at the end of all nodes
	 */
	PTUnrooted& placeSeq(const DigitalSeq& seq, const PTUNodePtr& u, const PTUNodePtr& v,
			double d0) {
		return placeSeq(seq, u, v, 0, csLen -1, d0);
	}

private:
	/**
	 * load an edge node1->node2 from a binary input
	 */
	istream& loadEdge(istream& in);

	/**
	 * save an edge node1->node2 to a binary output
	 * only the relationship between node IDs are stored
	 */
	ostream& saveEdge(ostream& out, const PTUNodePtr& node1, const PTUNodePtr& node2) const;

	/**
	 * load the edge cost node1->node2 from a binary input
	 */
	istream& loadEdgeCost(istream& in);

	/**
	 * save the edge cost node1->node2 to a binary output
	 */
	ostream& saveEdgeCost(ostream& out, const PTUNodePtr& node, const PTUNodePtr& node2) const;

	/**
	 * load leaf cost from a binary input
	 */
	istream& loadLeafCost(istream& in);

	/**
	 * save leaf cost to a binary outout
	 */
	ostream& saveLeafCost(ostream& out) const;

	/**
	 * load root information from a binary input
	 */
	istream& loadRoot(istream& in);

	/**
	 * save root information to a binary output
	 */
	ostream& saveRoot(ostream& out) const;

	/**
	 * load root cost for every node
	 */
	istream& loadRootCost(istream& in);

	/**
	 * save root cost for every node
	 */
	ostream& saveRootCost(ostream& out) const;

	/**
	 * load DNA model from a binary input in text model
	 */
	istream& loadModel(istream& in);

	/**
	 * save DNA model to a binary output in text model
	 */
	ostream& saveModel(ostream& out) const;

public:
	/* static methods */
	/**
	 * test whether p is parent of c
	 */
	static bool isParent(const PTUNodePtr& p, const PTUNodePtr& c) {
		return c != NULL && c->parent == p;
	}

	static bool isChild(const PTUNodePtr& c, const PTUNodePtr& p) {
		return isParent(p, c);
	}

	/**
	 * test whether a node a tip
	 * all children of a tip must be leaves
	 */
	static bool isTip(const PTUNodePtr& node);

	static PTUNodePtr firstLeaf(PTUNodePtr node);
	static PTUNodePtr lastLeaf(PTUNodePtr node);
	static PTUNodePtr randomLeaf(PTUNodePtr node);

	/* return dot product between a Matrix and a vector, scale the vector if necessary */
	static Vector4d dot_product_scaled(const Matrix4d& X, const Vector4d& V);

	/* return dot product between two vectors, scale the second vector if necessary */
	static double dot_product_scaled(const Vector4d& P, const Vector4d& V);

	/* member fields */
private:
	int csLen; /* number of aligned sites */

	PTUNodePtr root; /* root node of this tree */
	vector<PTUNodePtr> id2node; /* indexed tree nodes */

	BranchLenMap node2length; /* branch length index storing edge length */
	CostMap node2cost; /* cached cost message sending from u -> v, before conjugating into the Pr(v) of the branch-length */
	Matrix4Xd leafCost; /* cached 4 X 5 leaf cost matrix,
						with each column the pre-computed cost of observing A, C, G, T or - at any given site */

	ModelPtr model; /* DNA Model used to evaluate this tree, needed to be stored with this tree */

public:
	/* static fields */
	static const double MAX_COST_EXP;
	static const double INVALID_COST;

	static const double BRANCH_EPS;
};

inline size_t PTUnrooted::numEdges() const {
	size_t nEdges = 0;
	for(vector<PTUNodePtr>::const_iterator nodeIt = id2node.begin(); nodeIt != id2node.end(); ++nodeIt)
		nEdges += (*nodeIt)->numNeighbors();
	return nEdges;
}

inline size_t PTUnrooted::numLeaves() const {
	size_t nLeaves = 0;
	for(vector<PTUNodePtr>::const_iterator nodeIt = id2node.begin(); nodeIt != id2node.end(); ++nodeIt)
		if((*nodeIt)->isLeaf())
			nLeaves++;
	return nLeaves;
}

inline Matrix4Xd PTUnrooted::cost(const PTUNodePtr& node) {
	if(isEvaluated(node, node->parent))
		return node2cost[node][node->parent];

	Matrix4Xd costVec(4, csLen);
	for(int j = 0; j < csLen; ++j)
		costVec.col(j) = cost(node, j);
	return costVec;
}

inline ostream& PTUnrooted::writeTree(ostream& out, string format) const {
	StringUtils::toLower(format);
	if(format == "newick")
		return writeTreeNewick(out, root) << ";";
	else {
		errorLog << "Cannot write PTUnrooted tree, unknown tree format " << format << std::endl;
		out.setstate(std::ios_base::failbit);
		return out;
	}
}

inline bool PTUnrooted::isEvaluated(const PTUNodePtr& u, const PTUNodePtr& v) const {
	CostMap::const_iterator outerResult = node2cost.find(u);
	if(outerResult != node2cost.end()) {
		unordered_map<PTUNodePtr, Matrix4Xd>::const_iterator innerResult = outerResult->second.find(v);
		if(innerResult != outerResult->second.end())
			return innerResult->second.cols() == csLen && /* Matrix is initiated */
					(innerResult->second.array() != INVALID_COST).all(); /* values are all valid */
	}

	return false;
}

inline bool PTUnrooted::isEvaluated(const PTUNodePtr& u, const PTUNodePtr& v, int j) const {
	CostMap::const_iterator outerResult = node2cost.find(u);
	if(outerResult != node2cost.end()) {
		unordered_map<PTUNodePtr, Matrix4Xd>::const_iterator innerResult = outerResult->second.find(v);
		if(innerResult != outerResult->second.end())
			return innerResult->second.cols() == csLen && /* Matrix is initiated */
					(innerResult->second.col(j).array() != INVALID_COST).all(); /* values are not invalid */
	}

	return false;
}

inline double PTUnrooted::getBranchLength(const PTUNodePtr& u, const PTUNodePtr& v) const {
	return node2length.at(u).at(v);
//	BranchLenMap::const_iterator resultOuter = node2length.find(u);
//	if(resultOuter == node2length.end())
//		return -1;
//	else {
//		boost::unordered_map<PTUNodePtr, double>::const_iterator resultInner = resultOuter->second.find(v);
//		return resultInner == resultOuter->second.end() ? -1 : resultInner->second;
//	}
}

inline Matrix4Xd PTUnrooted::getBranchCost(const PTUNodePtr& u, const PTUNodePtr& v) const {
	CostMap::const_iterator resultOuter = node2cost.find(u);
	if(resultOuter == node2cost.end())
		return Matrix4Xd();
	else {
		boost::unordered_map<PTUNodePtr, Matrix4Xd>::const_iterator resultInner = resultOuter->second.find(v);
		return resultInner == resultOuter->second.end() ? Matrix4Xd() : resultInner->second;
	}
}

inline double PTUnrooted::treeCost(int start, int end) {
	double cost = 0;
	for(int j = start; j <= end; ++j)
		cost += treeCost(j);
	return cost;
}

inline double PTUnrooted::treeCost() {
	return treeCost(0, csLen - 1);
}

inline double PTUnrooted::treeCost(int j) {
	return dot_product_scaled(model->getPi(), cost(j));
}

inline vector<Matrix4d> PTUnrooted::getModelTransitionSet(string method) const {
	StringUtils::toLower(method);
	if(method == "gojobori")
		return getModelTraningSetGojobori();
	else if(method == "goldman")
		return getModelTraningSetGoldman();
	else
		throw invalid_argument("Unknown DNA substitution model training method '" + method + "'");
}

inline bool PTUnrooted::isTip(const PTUNodePtr& node) {
	if(node->isLeaf())
		return false;
	for(vector<PTUNodePtr>::const_iterator child = node->neighbors.begin(); child != node->neighbors.end(); ++child)
		if(isChild(*child, node) && !(*child)->isLeaf())
			return false;
	return true;
}

inline PTUnrooted::PTUNodePtr PhyloTreeUnrooted::firstLeaf(PTUNodePtr node) {
	while(!node->isLeaf())
		node = node->firstChild();
	return node;
}

inline PTUnrooted::PTUNodePtr PhyloTreeUnrooted::lastLeaf(PTUNodePtr node) {
	while(!node->isLeaf())
		node = node->lastChild();
	return node;
}

inline PTUnrooted::PTUNodePtr PhyloTreeUnrooted::randomLeaf(PTUNodePtr node) {
	while(!node->isLeaf()) {
		const vector<PTUNodePtr>& children = node->getChildren();
		node = children[rand() % children.size()];
	}
	return node;
}

inline Vector4d PTUnrooted::dot_product_scaled(const Matrix4d& X, const Vector4d& V) {
	Vector4d Y;
	double minV = V.minCoeff();
	double scale = minV != inf && minV > MAX_COST_EXP ? minV - MAX_COST_EXP : 0;

	for(Vector4d::Index i = 0; i < Y.rows(); ++i)
		Y(i) = -::log(X.row(i).dot(((-V).array() + scale).exp().matrix())) + scale;
	return Y;
}

inline double PTUnrooted::dot_product_scaled(const Vector4d& P, const Vector4d& V) {
	double minV = V.minCoeff();
	double scale = minV != inf && minV > MAX_COST_EXP ? minV - MAX_COST_EXP : 0;

	return -::log(P.dot(((-V).array() + scale).exp().matrix())) + scale;
}

} /* namespace EGriceLab */

#endif /* SRC_PHYLOTREEUNROOTED_H_ */
