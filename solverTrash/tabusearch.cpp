
#include <iostream>
#include <vector>
#include <algorithm>

#include "neighborhoods.h"
#include "tabusearch.h"

void TabuSearch::dumpTabuList() const {
    std::map<const Move, int>::const_iterator it;

    std::cout << "TabuList at iteration: " << currentIteration << std::endl;
    for (it = TabuList.begin(); it!=TabuList.end(); ++it) {
        it->first.dump();
        std::cout << " - expires: " << it->second << std::endl;
    }
    std::cout << "--------------------------" << std::endl;
}


void TabuSearch::dumpStats() const {
    std::cout << "TabuList Stats at iteration: " << currentIteration << std::endl;
    std::cout << "   movesAdded: " << movesAdded << std::endl
              << "   movesChecked: " << movesChecked << std::endl
              << "   movesCheckedTabu: " << movesCheckedTabu << std::endl
              << "   bestUpdatedLastAt: " << bestUpdatedLastAt << std::endl
              << "   bestUpdatedCnt: " << bestUpdatedCnt << std::endl
              << "   cntInsApplied: " << cntInsApplied << std::endl
              << "   cntIntraSwApplied: " << cntIntraSwApplied << std::endl
              << "   cntInterSwApplied: " << cntInterSwApplied << std::endl
              << "   timeGenIns: " << timeGenIns << std::endl
              << "   timeGenIntraSw: " << timeGenIntraSw << std::endl
              << "   timeGenInterSw: " << timeGenInterSw << std::endl
              << "   timeApplyMoves: " << timeApplyMoves << std::endl
              << "   cntGenInsCalls: " << cntGenInsCalls << std::endl
              << "   cntGenIntraSwCalls: " << cntGenIntraSwCalls << std::endl
              << "   cntGenInterSwCalls: " << cntGenInterSwCalls << std::endl
              << "   cumInsMoves: " << cumInsMoves << std::endl
              << "   cumIntraSwMoves: " << cumIntraSwMoves << std::endl
              << "   cumInterSwMoves: " << cumInterSwMoves << std::endl
              ;
}


bool TabuSearch::isTabu(const Move& m) {
    std::map<const Move, int>::iterator it;

    ++movesChecked;  // this makes it non-const
    for (it = TabuList.begin(); it!=TabuList.end(); ++it) {
        if (it->second < currentIteration) continue;
        if (m.isForbidden(it->first)) {
            ++movesCheckedTabu;  // this makes it non-const
            return true;
        }
    }
    return false;
}


void TabuSearch::cleanExpired() {
    std::map<const Move, int>::iterator it;
    for (it = TabuList.begin(); it!=TabuList.end(); ++it)
        if (it->second < currentIteration)
            TabuList.erase(it);
}


void TabuSearch::makeTabu(const Move m) {
    TabuList[m] = currentIteration + tabuLength;
    ++movesAdded;
}


/*
    This Tabu search algorithm was adapted from the paper:

    "Tabu Search Techniques for the Hetrogeneous Vehicle Routing
    Problem with Time Windows and Carrier-Dependent Cots" by
    Sara Ceschia, Luca Di Gaspero, and Andrea Schaerf

    We use a sequential solving strategy for combining our three neighborhood
    move functions. This as know as a "token-ring" search. Given an initial
    state and a set of algorithms, it makes circularly a run at each algorithm,
    always starting from the best solution found by the previous one. The
    overall process stops either when a full round of the algorithms does not
    find an improvement or the time (aka: interation count) granted has elapsed.
    
    Each single algorithm stops when it does not improve the current best
    solution for a given number of iterations (ie: stagnation).

*/

void TabuSearch::search() {

    currentIteration = 0;

    Timer start;
    bool madeChanges;
    do {
        // set the stagnation count as the last the last parameter
        // thhe values 500, 300, 300 can from the paper mentioned above
        std::cout << "TABUSEARCH: Starting iteration: " << currentIteration
            << std::endl;
//        madeChanges = doNeighborhoodMoves(Ins,     500)
//                    | doNeighborhoodMoves(IntraSw, 300)
//                    | doNeighborhoodMoves(InterSw, 300);
        madeChanges = doNeighborhoodMoves(Ins,     5)
                    | doNeighborhoodMoves(IntraSw, 3)
                    | doNeighborhoodMoves(InterSw, 3);
        std::cout << "TABUSEARCH: Finished iteration: " << currentIteration
            << ", madeChanges: " << madeChanges << std::endl;
        dumpStats();
    }
    while (madeChanges and ++currentIteration < maxIteration);

    std::cout << "TABUSEARCH: Total time: " << start.duration() << std::endl;
}


/*
    Algorithm for processing neighborhood moves

    For the requested individual move neighborhood:
        doInsMoves, doIntraSwMoves, doInterSwMoves

    do {
        Generate the neighhood of moves order from best to worst.
        Working through the neighborhood (best to worst)
        Filter out inFeasible moves then
        if the move is aspirational we apply the move
        otherwise if the move is not Tabu apply the move
        even if it makes the solution worse.
    } until there are not valid moves or stagnation 
    return an indicator that we made changes or not.
*/

bool TabuSearch::doNeighborhoodMoves(neighborMovesName whichNeighborhood, int maxStagnation) {
    bool madeMove;
    bool loopMadeMove;
    int stagnationCnt = 0;
    madeMove = false;

    do {
        loopMadeMove = false;

        // generate the a move neighborhood based on the currentSolution
        // this is fast and stupid move generation because we will
        // only look at a very small percentage of the actual moves
        // because we have to calcuate savings we should have already
        // checked for feasibility so only feasible solutions are returned
        std::vector<Move> neighborhood;
        Timer timeNeighboorhoodGeneration;
        switch (whichNeighborhood) {
            case Ins:
                currentSolution.getInsNeighborhood(neighborhood);
                timeGenIns += timeNeighboorhoodGeneration.duration();
                ++cntGenInsCalls;
                cumInsMoves += neighborhood.size();
std::cout << "\tdoNeighborhoodMoves for Ins: " << neighborhood.size()
    << " moves generated" << std::endl;
                break;
            case IntraSw:
                currentSolution.getIntraSwNeighborhood(neighborhood);
                timeGenIntraSw += timeNeighboorhoodGeneration.duration();
                ++cntGenIntraSwCalls;
                cumIntraSwMoves += neighborhood.size();
std::cout << "\tdoNeighborhoodMoves for IntraSw: " << neighborhood.size()
    << " moves generated" << std::endl;
                break;
            case InterSw:
                currentSolution.getInterSwNeighborhood(neighborhood);
                timeGenInterSw += timeNeighboorhoodGeneration.duration();
                ++cntGenInterSwCalls;
                cumInterSwMoves += neighborhood.size();
std::cout << "\tdoNeighborhoodMoves for InterSw: " << neighborhood.size()
    << " moves generated" << std::endl;
                break;
        }


        // and sort it so we can work from the best to the worst
        std::sort(neighborhood.begin(), neighborhood.end(), Move::bySavings);

        // take the best move that we may apply and apply it, if any
        Timer applyMoveTimer;
        for (std::vector<Move>::iterator it=neighborhood.begin();
                it!=neighborhood.end(); ++it) {

            // if the move is aspirational then we apply it
            if (currentSolution.getCost() - it->getsavings() < bestSolutionCost) {
std::cout << "\tdoNeighborhoodMoves: Aspiration move: "; it->dump();
                currentSolution.applyMove(*it);
                bestSolution = currentSolution;
                bestSolutionCost = bestSolution.getCost();
                makeTabu(*it);
                loopMadeMove = true;
                stagnationCnt = 0;
                bestUpdatedLastAt = currentIteration;
                ++bestUpdatedCnt;
                // update stats
                switch (whichNeighborhood) {
                    case Ins:     ++cntInsApplied;    break;
                    case IntraSw: ++cntIntraSwApplied; break;
                    case InterSw: ++cntInterSwApplied; break;
                }

                // ok we made a move, so now the neighborhood is no
                // longer valid so break to regenerate a new neighborhood
                break;
            }
            // if the move is not Tabu, then we apply it even if
            // it makes the solution worse so we move to a new
            // area of the search space
            else if (! isTabu(*it)) {
std::cout << "\tdoNeighborhoodMoves: Not Tabu: "; it->dump();
                currentSolution.applyMove(*it);
                makeTabu(*it);
                loopMadeMove = true;
                // update stats
                switch (whichNeighborhood) {
                    case Ins:     ++cntInsApplied;    break;
                    case IntraSw: ++cntIntraSwApplied; break;
                    case InterSw: ++cntInterSwApplied; break;
                }

                // ok we made a move, so now the neighborhood is no
                // longer valid so break to regenerate a new neighborhood
                break;
            }
        }
        timeApplyMoves += applyMoveTimer.duration();
        madeMove = madeMove or loopMadeMove;
        ++stagnationCnt;
    }
    while (madeMove and stagnationCnt < maxStagnation);

    return madeMove;
}


