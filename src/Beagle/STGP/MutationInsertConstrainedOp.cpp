/*
 *  Open BEAGLE
 *  Copyright (C) 2001-2007 by Christian Gagne and Marc Parizeau
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Contact:
 *  Laboratoire de Vision et Systemes Numeriques
 *  Departement de genie electrique et de genie informatique
 *  Universite Laval, Quebec, Canada, G1K 7P4
 *  http://vision.gel.ulaval.ca
 *
 */

/*!
 *  \file   beagle/GP/src/MutationInsertConstrainedOp.cpp
 *  \brief  Source code of class GP::MutationInsertConstrainedOp.
 *  \author Christian Gagne
 *  $Revision: 1.5 $
 *  $Date: 2007/08/17 18:09:11 $
 */

#include "beagle/GP.hpp"

#include <algorithm>
#include <string>

using namespace Beagle;


/*!
 *  \brief Construct a GP insert mutation operator.
 *  \param inMutationPbName Mutation probability parameter name used in register.
 *  \param inName Name of the operator.
 */
GP::MutationInsertConstrainedOp::MutationInsertConstrainedOp(std::string inMutationPbName,
        std::string inName) :
		Beagle::GP::MutationInsertOp(inMutationPbName, inName)
{ }


/*!
 *  \brief Insert mutate a GP individual.
 *  \param ioIndividual GP individual to mutate.
 *  \param ioContext Context of the evolution.
 *  \return True if the individual is effectively mutated, false if not.
 */
bool GP::MutationInsertConstrainedOp::mutate(Beagle::Individual& ioIndividual, Beagle::Context& ioContext)
{
	Beagle_StackTraceBeginM();

	Beagle_LogDetailedM(
	    ioContext.getSystem().getLogger(),
	    "mutation", "Beagle::GP::MutationInsertConstrainedOp",
	    std::string("Mutating individual with GP::MutationInsertConstrainedOp")
	);

	GP::Individual& lIndividual = castObjectT<GP::Individual&>(ioIndividual);
	GP::Context& lContext = castObjectT<GP::Context&>(ioContext);
	const unsigned int lMaxAttempts = mNumberAttempts->getWrappedValue();
	const unsigned int lMaxTreeDepth = mMaxTreeDepth->getWrappedValue();

	const Factory& lFactory = ioContext.getSystem().getFactory();
	GP::Tree::Alloc::Handle lTreeAlloc =
		castHandleT<GP::Tree::Alloc>(lFactory.getConceptAllocator("Genotype"));

	// Store original context values
	const unsigned int lOldGenotypeIndex = lContext.getGenotypeIndex();
	const GP::Tree::Handle lOldGenotypeHandle = lContext.getGenotypeHandle();

	Beagle_LogDebugM(
	    ioContext.getSystem().getLogger(),
	    "mutation", "Beagle::GP::MutationInsertConstrainedOp",
	    "Individual before constrained GP insert mutation"
	);
	Beagle_LogObjectDebugM(
	    ioContext.getSystem().getLogger(),
	    "mutation", "Beagle::GP::MutationInsertConstrainedOp",
	    ioIndividual
	);

	// Mutation attempts loop
	for(unsigned int i=0; i<lMaxAttempts; ++i) {

		// Choose tree and node to mutate
		const unsigned int lChosenTree = lIndividual.chooseRandomTree(lContext);
		const unsigned int lChosenNodeIndex = lIndividual.chooseRandomNode(lChosenTree, lContext);
		GP::Tree::Handle lOriginalTree = lIndividual[lChosenTree];

		// Compute depth of tree generated by mutation
		lContext.setGenotypeIndex(lChosenTree);
		lContext.setGenotypeHandle(lOriginalTree);
		lContext.emptyCallStack();
		lOriginalTree->setContextToNode(lChosenNodeIndex, lContext);
		const unsigned int lMutationDepth =
		    lContext.getCallStackSize() + lOriginalTree->getTreeDepth(lChosenNodeIndex);

		// Check that mutation will not generate a tree deeper than the maximum allowed depth
		if(lMutationDepth > lMaxTreeDepth) {
			Beagle_LogDebugM(
			    ioContext.getSystem().getLogger(),
			    "mutation", "Beagle::GP::MutationInsertConstrainedOp",
			    std::string("Constrained insert mutation attempt failed as the generated tree will exceed ")+
			    std::string("maximum allowed tree depth")
			);
			continue;
		}

		// Create new tree
		Beagle_LogDebugM(
		    ioContext.getSystem().getLogger(),
		    "mutation", "Beagle::GP::MutationInsertConstrainedOp",
		    std::string("Creating new tree")
		);
		GP::Tree::Handle lNewTree = castHandleT<GP::Tree>(lTreeAlloc->allocate());
		lNewTree->setPrimitiveSetIndex(lOriginalTree->getPrimitiveSetIndex());
		lNewTree->setNumberArguments(lOriginalTree->getNumberArguments());

		// Replace original tree with new tree
		lIndividual[lChosenTree] = lNewTree;
		lContext.setGenotypeHandle(lNewTree);

		// Copy unchanged part of original tree into new tree
		Beagle_LogDebugM(
		    ioContext.getSystem().getLogger(),
		    "mutation", "Beagle::GP::MutationInsertConstrainedOp",
		    std::string("Copying unchanged part of original tree to new tree")
		);
		lNewTree->insert(lNewTree->end(),
		                 lOriginalTree->begin(),
		                 lOriginalTree->begin()+lChosenNodeIndex);
		Beagle_AssertM(lNewTree->size() == lChosenNodeIndex);

		// Generate new branch primitive to insert.
		GP::PrimitiveSet& lPrimitiveSet = lNewTree->getPrimitiveSet(lContext);
		Primitive::Handle lBranchInserted = lPrimitiveSet.select(GP::Primitive::eBranch, lContext);
		if(lBranchInserted==NULL) {
			Beagle_LogDebugM(
			    ioContext.getSystem().getLogger(),
			    "mutation", "Beagle::GP::MutationInsertConstrainedOp",
			    std::string("Constrained insert mutation attempt failed as it seems impossible ")+
			    std::string("to select a branch primitive in the actual context")
			);
			lIndividual[lChosenTree] = lOriginalTree;
			lContext.setGenotypeHandle(lOriginalTree);
			continue;
		}
		Beagle_LogDebugM(
		    ioContext.getSystem().getLogger(),
		    "mutation", "Beagle::GP::MutationInsertConstrainedOp",
		    std::string("Branch primitive to be inserted by mutation is primitive '")+
		    lBranchInserted->getName()+std::string("'")
		);

		// Insert new branch
		lBranchInserted = lBranchInserted->giveReference(GP::Primitive::eBranch, lContext);
		lNewTree->push_back(Node(lBranchInserted,1));
		if(lBranchInserted->validate(lContext) == false) {
			Beagle_LogDebugM(
			    ioContext.getSystem().getLogger(),
			    "mutation", "Beagle::GP::MutationInsertConstrainedOp",
			    std::string("Constrained insert mutation attempt failed as the selected ")+
			    std::string("branch doesn't match the constraints")
			);
			lIndividual[lChosenTree] = lOriginalTree;
			lContext.setGenotypeHandle(lOriginalTree);
			continue;
		}
		const unsigned int lNbArgsInsertedBranch = lBranchInserted->getNumberArguments();
		Beagle_AssertM(lNbArgsInsertedBranch != 0);
		const unsigned int lSubtreeArgIndex =
		    lContext.getSystem().getRandomizer().rollInteger(0, lNbArgsInsertedBranch-1);
		Beagle_AssertM(lSubtreeArgIndex < lNbArgsInsertedBranch);
		const unsigned int lSubtreeSize = (*lOriginalTree)[lChosenNodeIndex].mSubTreeSize;

		// Generate inserted node subtrees
		bool lArgsGenFailed = false;
		for(unsigned int j=0; j<lNbArgsInsertedBranch; ++j) {
			if(j == lSubtreeArgIndex) {
				const unsigned int lSubtreeIndex = lNewTree->size();
				lNewTree->insert(lNewTree->end(),
				                 lOriginalTree->begin()+lChosenNodeIndex,
				                 lOriginalTree->begin()+lChosenNodeIndex+lSubtreeSize);
				lContext.pushCallStack(lSubtreeIndex);
				if(lNewTree->validateSubTree(lSubtreeIndex, lContext) == false) {
					lArgsGenFailed = true;
					break;
				}
				lContext.popCallStack();
				(*lNewTree)[lChosenNodeIndex].mSubTreeSize += lSubtreeSize;
			} else {
				Primitive::Handle lArgInserted = lPrimitiveSet.select(GP::Primitive::eTerminal, lContext);
				if(lArgInserted == NULL) {
					lArgsGenFailed = true;
					break;
				}
				lArgInserted = lArgInserted->giveReference(GP::Primitive::eTerminal, lContext);
				const unsigned int lSubtreeIndex = lNewTree->size();
				lNewTree->push_back(Node(lArgInserted,1));
				lContext.pushCallStack(lSubtreeIndex);
				if(lArgInserted->validate(lContext) == false) {
					lArgsGenFailed = true;
					break;
				}
				lContext.popCallStack();
				++(*lNewTree)[lChosenNodeIndex].mSubTreeSize;
			}
		}
		if(lArgsGenFailed) {
			Beagle_LogDebugM(
			    ioContext.getSystem().getLogger(),
			    "mutation", "Beagle::GP::MutationInsertConstrainedOp",
			    std::string("Constrained insert mutation attempt failed as it seems impossible ")+
			    std::string("to select a terminal primitive under the inserted branch in the actual context")
			);
			lIndividual[lChosenTree] = lOriginalTree;
			lContext.setGenotypeHandle(lOriginalTree);
			continue;
		}

		// Complete new tree with rest of original tree
		Beagle_AssertM(lOriginalTree->size() >= (lChosenNodeIndex+lSubtreeSize));
		lNewTree->insert(lNewTree->end(),
		                 lOriginalTree->begin()+lChosenNodeIndex+lSubtreeSize,
		                 lOriginalTree->end());
		Beagle_AssertM(lNewTree->size() == (lOriginalTree->size()+lNbArgsInsertedBranch));

		// Correct subtree size data and terminate mutation process
		Beagle_LogDebugM(
		    ioContext.getSystem().getLogger(),
		    "mutation", "Beagle::GP::MutationInsertConstrainedOp",
		    std::string("Correcting the 'mSubTreeSize' fields of tree")
		);
		lContext.popCallStack();
		for(unsigned int j=0; j<lContext.getCallStackSize(); ++j) {
			(*lNewTree)[lContext[j]].mSubTreeSize += lNbArgsInsertedBranch;
		}

		// Mutation successful, log messages and return
		std::ostringstream lOSS;
		lOSS << "Successfully inserted a new node at index " << lChosenNodeIndex;
		lOSS << " of the " << uint2ordinal(lChosenTree) << " tree of the actual individual";
		Beagle_LogTraceM(
		    ioContext.getSystem().getLogger(),
		    "mutation",
		    "Beagle::GP::MutationInsertConstrainedOp",
		    lOSS.str()
		);
		Beagle_LogDebugM(
		    ioContext.getSystem().getLogger(),
		    "mutation",
		    "Beagle::GP::MutationInsertConstrainedOp",
		    "Individual after constrained GP insert mutation"
		);
		Beagle_LogObjectDebugM(
		    ioContext.getSystem().getLogger(),
		    "mutation",
		    "Beagle::GP::MutationInsertConstrainedOp",
		    ioIndividual
		);
		lContext.emptyCallStack();
		lContext.setGenotypeIndex(lOldGenotypeIndex);
		lContext.setGenotypeHandle(lOldGenotypeHandle);
		return true;
	}

	// Insert mutation failed, return without mutating the individual
	lContext.emptyCallStack();
	lContext.setGenotypeIndex(lOldGenotypeIndex);
	lContext.setGenotypeHandle(lOldGenotypeHandle);
	Beagle_LogTraceM(
	    ioContext.getSystem().getLogger(),
	    "mutation", "Beagle::GP::MutationInsertConstrainedOp",
	    std::string("All constrained insert mutation attempts failed; ")+
	    std::string("resuming from mutation without modifying the individual")
	);
	return false;

	Beagle_StackTraceEndM();
}

