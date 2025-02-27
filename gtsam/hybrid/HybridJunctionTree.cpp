/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation,
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file HybridJunctionTree.cpp
 * @date Mar 11, 2022
 * @author Fan Jiang
 */

#include <gtsam/hybrid/HybridEliminationTree.h>
#include <gtsam/hybrid/HybridGaussianFactorGraph.h>
#include <gtsam/hybrid/HybridJunctionTree.h>
#include <gtsam/inference/JunctionTree-inst.h>
#include <gtsam/inference/Key.h>

#include <unordered_map>

namespace gtsam {

// Instantiate base classes
template class EliminatableClusterTree<HybridBayesTree,
                                       HybridGaussianFactorGraph>;
template class JunctionTree<HybridBayesTree, HybridGaussianFactorGraph>;

struct HybridConstructorTraversalData {
  typedef
      typename JunctionTree<HybridBayesTree, HybridGaussianFactorGraph>::Node
          Node;
  typedef
      typename JunctionTree<HybridBayesTree,
                            HybridGaussianFactorGraph>::sharedNode sharedNode;

  HybridConstructorTraversalData* const parentData;
  sharedNode junctionTreeNode;
  FastVector<SymbolicConditional::shared_ptr> childSymbolicConditionals;
  FastVector<SymbolicFactor::shared_ptr> childSymbolicFactors;
  KeySet discreteKeys;

  // Small inner class to store symbolic factors
  class SymbolicFactors : public FactorGraph<Factor> {};

  HybridConstructorTraversalData(HybridConstructorTraversalData* _parentData)
      : parentData(_parentData) {}

  // Pre-order visitor function
  static HybridConstructorTraversalData ConstructorTraversalVisitorPre(
      const boost::shared_ptr<HybridEliminationTree::Node>& node,
      HybridConstructorTraversalData& parentData) {
    // On the pre-order pass, before children have been visited, we just set up
    // a traversal data structure with its own JT node, and create a child
    // pointer in its parent.
    HybridConstructorTraversalData data =
        HybridConstructorTraversalData(&parentData);
    data.junctionTreeNode = boost::make_shared<Node>(node->key, node->factors);
    parentData.junctionTreeNode->addChild(data.junctionTreeNode);

    for (HybridFactor::shared_ptr& f : node->factors) {
      for (auto& k : f->discreteKeys()) {
        data.discreteKeys.insert(k.first);
      }
    }

    return data;
  }

  // Post-order visitor function
  static void ConstructorTraversalVisitorPostAlg2(
      const boost::shared_ptr<HybridEliminationTree::Node>& ETreeNode,
      const HybridConstructorTraversalData& data) {
    // In this post-order visitor, we combine the symbolic elimination results
    // from the elimination tree children and symbolically eliminate the current
    // elimination tree node.  We then check whether each of our elimination
    // tree child nodes should be merged with us.  The check for this is that
    // our number of symbolic elimination parents is exactly 1 less than
    // our child's symbolic elimination parents - this condition indicates that
    // eliminating the current node did not introduce any parents beyond those
    // already in the child->

    // Do symbolic elimination for this node
    SymbolicFactors symbolicFactors;
    symbolicFactors.reserve(ETreeNode->factors.size() +
                            data.childSymbolicFactors.size());
    // Add ETree node factors
    symbolicFactors += ETreeNode->factors;
    // Add symbolic factors passed up from children
    symbolicFactors += data.childSymbolicFactors;

    Ordering keyAsOrdering;
    keyAsOrdering.push_back(ETreeNode->key);
    SymbolicConditional::shared_ptr conditional;
    SymbolicFactor::shared_ptr separatorFactor;
    boost::tie(conditional, separatorFactor) =
        internal::EliminateSymbolic(symbolicFactors, keyAsOrdering);

    // Store symbolic elimination results in the parent
    data.parentData->childSymbolicConditionals.push_back(conditional);
    data.parentData->childSymbolicFactors.push_back(separatorFactor);
    data.parentData->discreteKeys.merge(data.discreteKeys);

    sharedNode node = data.junctionTreeNode;
    const FastVector<SymbolicConditional::shared_ptr>& childConditionals =
        data.childSymbolicConditionals;
    node->problemSize_ = (int)(conditional->size() * symbolicFactors.size());

    // Merge our children if they are in our clique - if our conditional has
    // exactly one fewer parent than our child's conditional.
    const size_t nrParents = conditional->nrParents();
    const size_t nrChildren = node->nrChildren();
    assert(childConditionals.size() == nrChildren);

    // decide which children to merge, as index into children
    std::vector<size_t> nrChildrenFrontals = node->nrFrontalsOfChildren();
    std::vector<bool> merge(nrChildren, false);
    size_t nrFrontals = 1;
    for (size_t i = 0; i < nrChildren; i++) {
      // Check if we should merge the i^th child
      if (nrParents + nrFrontals == childConditionals[i]->nrParents()) {
        const bool myType =
            data.discreteKeys.exists(conditional->frontals()[0]);
        const bool theirType =
            data.discreteKeys.exists(childConditionals[i]->frontals()[0]);

        if (myType == theirType) {
          // Increment number of frontal variables
          nrFrontals += nrChildrenFrontals[i];
          merge[i] = true;
        }
      }
    }

    // now really merge
    node->mergeChildren(merge);
  }
};

/* ************************************************************************* */
HybridJunctionTree::HybridJunctionTree(
    const HybridEliminationTree& eliminationTree) {
  gttic(JunctionTree_FromEliminationTree);
  // Here we rely on the BayesNet having been produced by this elimination tree,
  // such that the conditionals are arranged in DFS post-order.  We traverse the
  // elimination tree, and inspect the symbolic conditional corresponding to
  // each node.  The elimination tree node is added to the same clique with its
  // parent if it has exactly one more Bayes net conditional parent than
  // does its elimination tree parent.

  // Traverse the elimination tree, doing symbolic elimination and merging nodes
  // as we go.  Gather the created junction tree roots in a dummy Node.
  typedef HybridConstructorTraversalData Data;
  Data rootData(0);
  rootData.junctionTreeNode =
      boost::make_shared<typename Base::Node>();  // Make a dummy node to gather
                                                  // the junction tree roots
  treeTraversal::DepthFirstForest(eliminationTree, rootData,
                                  Data::ConstructorTraversalVisitorPre,
                                  Data::ConstructorTraversalVisitorPostAlg2);

  // Assign roots from the dummy node
  this->addChildrenAsRoots(rootData.junctionTreeNode);

  // Transfer remaining factors from elimination tree
  Base::remainingFactors_ = eliminationTree.remainingFactors();
}

}  // namespace gtsam
