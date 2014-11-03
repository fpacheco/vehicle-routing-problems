/*VRP*********************************************************************
 *
 * vehicle routing problems
 *      A collection of C++ classes for developing VRP solutions
 *      and specific solutions developed using these classes.
 *
 * Copyright 2014 Stephen Woodbridge <woodbri@imaptools.com>
 * Copyright 2014 Vicky Vergara <vicky_vergara@hotmail.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the MIT License. Please file LICENSE for details.
 *
 ********************************************************************VRP*/
#ifndef BUCKET_H
#define BUCKET_H

#include <deque>
#include <vector>
#include <set>
#include <iostream>
#include <algorithm>
#include <limits>
#include <cassert>
#include "node.h"
//#include "twc.h"
//#include "plot.h"

/*! \class TwBucket
 * \brief A template class that provides deque like container with lots of additional functionality.
 *
 * TwBucket provides a set like container. It is used by \ref Twpath for
 * storage. It also provides several un-evaluated path operations. None of
 * the manipulation at this level is evaluated.
 *
 * The class provides:
 * - basic deque container like operations
 * - set operations
 * - node id based tools
 * - position based tools
 * - other tools
*/
template <class knode>
class TwBucket {

  protected:
    /*! \typedef typedef typename std::vector<std::vector<double> > TravelTimes
     * \brief Define an NxN vector array for holding travel times between nodes.
     */
    typedef typename std::vector<std::vector<double> > TravelTimes;

    /*! \fn double _MIN() const
     * \brief Define double -infinity
     */
    inline double _MIN() const { return ( -std::numeric_limits<double>::max() );};

    /*! \fn double _MAX() const
     * \brief Define double +infinity
     */
    inline double _MAX() const { return ( std::numeric_limits<double>::max() );};

    static  TravelTimes TravelTime; ///< Defines the travel time matrix
    std::deque<knode> path;         ///< Defines the bucket container


    /*! \class compNode
     * \brief A node comparision class for ordering nodes in a set.
     */
    class compNode {
      public:
        bool operator()( const knode &n1, const knode &n2 ) const {
            return ( n1.getnid() < n2.getnid() );
        }
    };


    typedef unsigned long UID;
    typedef unsigned long POS;
    typedef typename std::deque<knode>::iterator iterator;
    typedef typename std::deque<knode>::reverse_iterator reverse_iterator;
    typedef typename std::deque<knode>::const_reverse_iterator
    const_reverse_iterator;
    typedef typename std::deque<knode>::const_iterator const_iterator;


  public:

    /*! \fn void setTravelTimes(const TravelTimes &_tt)
     * \brief Assign a travel time matrix to the \ref TwBucket
     */
    void setTravelTimes( const TravelTimes &_tt ) {
        TravelTime = _tt;
    }

    /*! \fn double  timePCN(POS prev, POS curr, POS next) const
     * \brief Evaluates the time from the previous to current to next container.
     * \param[in] prev Position of the previous node in the path
     * \param[in] curr Position of the current node in the path
     * \param[in] next Position of the next node in the path
     *
     * simulates the following contiguous containers in the path
     * - prev curr next
     *
     * \return travel time previous + service time current + travel time to next
     * \return infinity              if there is a TWV (timw window violation)
    */
    double  timePCN( POS prev, POS curr, POS next ) const {
        assert ( prev < path.size() );
        assert ( curr < path.size() );
        assert ( next < path.size() );
        double result = path[curr].getArrivalTime()
                        + travelTime( path[prev], path[curr] );

        if ( result > path[curr].closes() ) return _MAX();

        if ( result < path[curr].opens() )
            result = path[curr].opens() - path[prev].getDepartureTime();

        return result + path[curr].getservicetime()
               + travelTime(  path[curr], path[next] );
    }

    /*! \fn double  timePCN(POS &prev, POS &curr, const knode &dump) const
     * \brief Evaluates the time from the previous to current to the dump.
     * \param[in] prev Position of the previous node in the path
     * \param[in] curr Position of the current node in the path
     * \param[in] dump A reference to a dump node.
     *
     * simulates the following contiguous containers in the path
     * - prev curr dump
     *
     * \return travel time previous + service time current + travel time to dump
     * \return infinity              if there is a TWV (timw window violation)
    */
    double  timePCN( POS &prev, POS &curr, const knode &dump ) const {
        assert ( prev < path.size() );
        assert ( curr < path.size() );
        double result = path[curr].getArrivalTime()
                        + travelTime( path[prev], path[curr] );

        if ( result  > path[curr].closes() ) return _MAX();

        return result + path[curr].getservicetime()
               + travelTime( path[curr], dump );
    }

    /*! \fn double travelTime( const knode &from, const knode &to) const
     * \brief Fetch the travel time from Node to Node
     * \note Nodes do not need to be in the path.
    */
    double travelTime( const knode &from, const knode &to ) const {
        return travelTime( from.getnid(), to.getnid()  );
    }

    /*! \fn double travelTime(UID i, UID j) const
     * \brief Fetch the travel time from nodeId to nodeId
     * \note Nodes do not need to be in the path.
    */
    double travelTime( UID i, UID j ) const {
        assert ( i < TravelTime.size() );
        assert ( j < TravelTime.size() );
        return TravelTime[i][j];
    }


    /*! \fn double getDeltaTime(const knode &node, const knode &dump) const
     * \brief Simulate changes of times within the path
     *
     * Simulates the following change of times within the path
     * - last dump
     * - last node dump
     *
     * and checks TWV and returns infinity if the occur at:
     * - node
     * - dump
     *
     * \return \f$ delta = tt_last,node + service(n) + tt_node,dump - tt_last,dump \f$
     * \return infinity when there is a TWV
    */
    double getDeltaTime( const knode &node, const knode &dump ) const {
        knode last = path[path.size() - 1];
        double nodeArrival = last.getDepartureTime() + travelTime( last, node );

        if (  node.latearrival( nodeArrival ) ) return _MAX();

        if (  node.earlyarrival( nodeArrival ) ) nodeArrival = node.opens();

        double dumpArrival = nodeArrival + node.getservicetime() +
                             travelTime( node, dump );

        if (  dump.latearrival( dumpArrival ) ) return _MAX();

        if (  dump.earlyarrival( dumpArrival ) ) dumpArrival = dump.opens();

        double delta = dumpArrival - last.getDepartureTime();
        return delta;
    }

    /*! \fn double getDeltaTimeAfterDump(const knode &dump, const knode &node ) const
     * \brief Simulate changes in travel times within the path
     *
     * Simulates the following change of travelTimes within the path
     * - dump
     * - dump node dump2
     *
     * and checks for TWV and returns infinity if the occur at:
     * - node
     * - dump2
     *
     * \return \f$ tt_dump,node + service(node) + tt_node,dump + service(dump) \f$
     * \return infinity when there is a TWV
    */
    double getDeltaTimeAfterDump( const knode &dump, const knode &node ) const {
        double nodeArrival = dump.getDepartureTime() + travelTime( dump, node );

        if (  node.latearrival( nodeArrival ) ) return _MAX();

        if (  node.earlyarrival( nodeArrival ) ) nodeArrival = node.opens();

        double dumpArrival =  nodeArrival + node.getservicetime() +
                              travelTime( node, dump );

        if (  dump.latearrival( dumpArrival ) ) return _MAX();

        if (  dump.earlyarrival( dumpArrival ) ) dumpArrival = dump.opens();

        double delta = dumpArrival + dump.getservicetime() -
                       dump.getDepartureTime();
        return delta;
    }


    /*! \fn double getDeltaTimeSwap(POS pos1, POS pos2) const
     * \brief Compute the change in time when swapping nodes in pos1 and pos2
     *
     * Simulate swapping nodes in pos1 and pos2 in the path and compute
     * the delta time impact that would have on the path.
     *
     * \param[in] pos1 Position of the node to be swapped.
     * \param[in] pos2 Position of the other node to be swapped.
     * \return The delta time or infinity if if creates a path violation.
    */
    double getDeltaTimeSwap( POS pos1, POS pos2 ) const {
        assert( pos1 < path.size() - 1 and pos2 < path.size() );
        #ifdef TESTED
        std::cout << "Entering twBucket::getDeltaTimeSwap() \n";
        #endif

        double delta, oldTime, newTime;

        // pos1 is the lowest
        if ( pos1 > pos2 ) { int tmp = pos1; pos1 = pos2; pos2 = tmp;}

        //special case nidPrev nid1 nid2 nidNext
        if ( pos2 == pos1 + 1 ) {
            // nids invloved
            // in the same order the nodes are in the path
            int nidPrev, nid1, nid2, nidNext;
            nidPrev = path[pos1 - 1].getnid();
            nid1 = path[pos1].getnid();
            nid2 = path[pos2].getnid();

            if ( pos2 != size() ) nidNext = path[pos2 + 1].getnid();

            //                pos1-1  pos1  pos2  pos2+1
            // newpath looks: nidPrev nid2 nid1, nidNext

            // check for TWV
            if ( path[pos1 - 1].getDepartureTime()
                 + TravelTime[nidPrev][nid2] > path[pos2].closes() )
                return _MAX();

            if ( path[pos1 - 1].getDepartureTime()
                 + TravelTime[nidPrev][nid2] + path[pos1].getservicetime()
                 + TravelTime[nid2][nid1] > path[pos1].closes() )
                return _MAX();

            // locally we are ok...  no capacity Violations
            // sum (services) remains constant
            if ( pos2 + 1 == size() ) {
                // newpath looks: nidPrev nid1 nid2,  DUMP in V
                //                pos1-1  pos1  pos2  pos2+1
                // newpath looks: nidPrev nid2 nid1,  DUMP in V
                // delta = new - old
                oldTime = path[pos2].getDepartureTime();
                newTime = path[pos1 - 1].getDepartureTime()
                          + TravelTime[nidPrev][nid2] + TravelTime[nid2][nid1];
                delta = oldTime - newTime;
            }
            else {
                // oldpath looks: nidPrev nid1 nid2,  nidNext
                //                pos1-1  pos1  pos2  pos2+1
                // newpath looks: nidPrev nid2 nid1,  nidNext

                oldTime = path[pos2 + 1].getArrivalTime();
                newTime = path[pos1 - 1].getDepartureTime()
                          + TravelTime[nidPrev][nid2]
                          + TravelTime[nid2][nid1]
                          + TravelTime[nid1][nidNext] ;
                delta   =  oldTime - newTime;;
            }

            // check for TWV
            if ( pos2 + 1 < size() and deltaGeneratesTV( delta, pos2 + 1 ) )
                return _MAX();

            return delta;
            // end of case when one node is after the other
        }

        // oldpath looks: nidPrev1 nid1 nidnext1    nidPrev2    nid2,  nidNext2
        //                pos1-1  pos1  pos1+1      pos2-1      pos2    pos2+1
        // newpath looks: nidPrev1 nid2 nidnext1    nidPrev2,   nid1,  nidNext2
        double delta1 = getDeltaTime( path[pos2], pos1, pos1 + 1 );
        double delta2 = getDeltaTime( path[pos1], pos2, pos2 + 1 );

        // check if TWV is generated
        if ( ( delta1 == _MAX() ) or  ( delta2 == _MAX() ) ) return _MAX();

        if ( deltaGeneratesTVupTo( delta1, pos1, pos2 - 1 ) ) return _MAX();

        if ( deltaGeneratesTV( delta1 + delta2, pos2 + 1 ) ) return _MAX();

        // simple checks for cargo Violation
        if ( path[pos1].getdemand() == path[pos2].getdemand()
             and not path[size() - 1].hascv() ) return delta1 + delta2;

        // check for cargo Violation Missing
        // if there is no dump  on the path: return  delta1 + delta2

        // if the share the same dump  return delta1 +delta2

        return delta1 + delta2;
    }


    /*! \fn double getDeltaTime(const knode &node, POS pos , POS pos1) const
     * \brief Compute the cange in time when swapping node with the node at pos
     *
     * If the current path looks like prev -\> pos -\> pos1 then compute the
     * the change in time of swapping node for the node at pos, so the new
     * path would look like prev -\> node -\> pos1
     *
     * \param[in] node The node to evaluate if swapped with node at pos.
     * \param[in] pos The position of the node to be swapped.
     * \param[in] pos1 The next node following pos.
     * \return The change in cost or infinity if a TWV would be generated.
     */
    double getDeltaTime( const knode &node, POS pos , POS pos1 ) const {
        assert( pos1 <= path.size() );
        assert( pos > 0 and pos1 == ( pos + 1 ) );

        if ( pos == 0 and path[pos].isdepot() ) return _MAX();

        int nid = path[pos].getnid();
        int prev = path[pos - 1].getnid();

        if ( path[pos - 1].getDepartureTime()
             + TravelTime[prev][node.getnid()] > node.closes() )
            return _MAX();

        if ( pos1 == size() )
            return  TravelTime[prev][node.getnid()]
                    + node.getservicetime()
                    - ( path[pos].getDepartureTime()
                        - path[pos - 1].getDepartureTime() );

        int next = path[pos1].getnid();

        double delta  =  TravelTime[prev][node.getnid()]
                         + node.getservicetime()
                         + TravelTime[node.getnid()][next]
                         - ( path[pos1].getArrivalTime()
                             - path[pos - 1].getDepartureTime() ) ;
        return delta;
    }

    /*! \fn double getDeltaTimeTVcheck(const knode &node, POS pos, POS pos1) const
     * \brief Compute the change in time when swapping node into pos in the path and do additional time violation checks.
     *
     * If the current path looks like prev -\> pos -\> pos1 then compute the
     * the change in time of swapping node for the node at pos, so the new
     * path would look like prev -\> node -\> pos1
     *
     * \param[in] node The node to evaluate if swapped with node at pos.
     * \param[in] pos The position of the node to be swapped.
     * \param[in] pos1 The next node following pos.
     * \return The change in cost or infinity if a TWV would be generated.
     */
    double getDeltaTimeTVcheck( const knode &node, POS pos, POS pos1 ) const {
        assert( pos1 <= path.size() );
        assert( pos > 0 and pos1 == ( pos + 1 ) );

        double delta = getDeltaTime( node, pos, pos1 );

        if ( path[pos - 1].getDepartureTime()
             + TravelTime[ path[pos - 1].getnid() ] [node.getnid() ]
             > node.closes() ) return _MAX();

        if ( pos == size() ) return delta;

        if ( deltaGeneratesTV( delta, pos1 ) ) return _MAX();

        return delta;
    }


    /*! \fn double getDeltaTime(const knode &node, POS pos) const
     * \brief Compute the change in time of inserting node before pos in the path.
     *
     * Simulate inserting node before pos in the path and compute the resulting
     * change in time. No TW violations are checked.
     *
     * \param[in] node The node to be inserted in the simulation.
     * \param[in] pos The position before which the node will be inserted.
     * \return The change in travel time or infinity if the move is invalid.
     */
    double  getDeltaTime( const knode &node, POS pos ) const {
        assert( pos < path.size() );

        if ( pos == 0 or path[pos].isdepot() ) return _MAX();

        int nid = path[pos].getnid();
        int prev = path[pos - 1].getnid();

        if ( pos == size() )
            return  TravelTime[prev][node.getnid()] + node.getservicetime();

        return TravelTime[prev][node.getnid()]
               + node.getservicetime()
               + TravelTime[node.getnid()][nid]
               - TravelTime[prev][nid];
    }


    /*! \fn double getDeltaTimeTVcheck(const knode &node, POS pos) const
     * \brief Compute the change in time of inserting node before pos in the path and check for TW violations..
     *
     * Simulate inserting node before pos in the path and compute the resulting
     * change in time and check for TW violations.
     *
     * \param[in] node The node to be inserted in the simulation.
     * \param[in] pos The position before which the node will be inserted.
     * \return The change in travel time or infinity if the move is invalid.
     */
    double  getDeltaTimeTVcheck( const knode &node, POS pos ) const {
        assert( pos <= path.size() );
        assert( pos > 0 );

        double delta = getDeltaTime( node, pos );

        // check for TWV
        if ( path[pos - 1].getDepartureTime()
             + TravelTime[ path[pos - 1].getnid() ][ node.getnid()]
             > node.closes() ) return _MAX();

        if ( pos == size() ) return delta;

        // check for TWV
        if ( deltaGeneratesTV( delta, pos ) ) return _MAX();

        return delta;
    }


    /*! \fn bool deltaGeneratesTVupTo(double delta, POS pos, POS upto) const
     * \brief Check all nodes from pos to upto if adding delta would cause a violation.
     *
     * \param[in] delta The change in time to evaluate.
     * \param[in] pos The position to start evaluating.
     * \param[in] upto The position to stop evaluating.
     * \return true if delta would generate a time violation.
     */
    bool deltaGeneratesTVupTo( double delta, POS pos, POS upto ) const {
        assert( pos < path.size() and upto < size() and pos <= upto );
        bool flag = false;

        // checking if the delta affects any node after it
        for ( int i = pos; i <= upto; i++ )
            if ( path[i].getArrivalTime() + delta > path[i].closes() ) {
                flag = true;
                break;
            }

        return flag;
    }

    /*! \fn bool deltaGeneratesTV(double delta, POS pos) const
     * \brief Check all nodes forward from pos if adding delta would cause a violation.
     *
     * \param[in] delta The change in time to evaluate.
     * \param[in] pos The position to start evaluating.
     * \return true if delta would generate a time violation.
     */
    bool deltaGeneratesTV( double delta, POS pos ) const {
        if ( pos < size() )
            return  deltaGeneratesTVupTo( delta, pos, size() - 1 );
        else
            return false;
    }


    // ---------------- other tools ----------------------------------

    /*! \fn double segmentDistanceToPoint(POS pos, const knode& n) const
     * \brief Compute the shortest distance from a line segment to a node.
     *
     *
     *
     * \param[in] pos Position of start of segment.
     * \param[in] node The node to compute the distance to.
     * \return The shortest distance from node to line segment.
     */
    double segmentDistanceToPoint( POS pos, const knode &node ) const {
        assert( pos + 1 < path.size() );
        return node.distanceToSegment( path[pos], path[pos + 1] );
    }

    /*! \fn void swap( POS i, POS j )
     * \brief Swap nodes in position i and j in the path
     * \param[in] i First node position to swap.
     * \param[in] j Second node position to swap.
     */
    void swap( POS i, POS j ) {
        std::iter_swap( this->path.begin() + i, this->path.begin() + j );
    }

    /*! \fn bool swap( POS t1_pos, TwBucket<knode> &truck2, POS t2_pos )
     * \brief Swap nodes between two paths.
     *
     * Swap nodes nodes between two paths, like
     * - truck.swap( t1_pos, truck2, t2_pos );
     *
     * The node in position t1_pos of truck1 will be swapped with the node
     * in position t2_pos of truck2.
     *
     * \param[in] t1_pos Position of node in truck1
     * \param[in] truck2 Truck2, a \ref TwBucket
     * \param[in] t2_pos Position of node in truck2
     * \return true
     */
    bool swap( POS t1_pos, TwBucket<knode> &truck2, POS t2_pos ) {
        assert ( t1_pos < size() and t2_pos < truck2.size() );
        std::iter_swap( path.begin() + t1_pos, truck2.path.begin() + t2_pos );
        return true;
    }


    /*! \fn void move( int fromi, int toj )
     * \brief Move node fromi to the new position of toj in this TwBucket
     *
     */
    void move( int fromi, int toj ) {
        if ( fromi == toj ) return;

        if ( fromi < toj ) {
            insert( this->path[fromi], toj + 1 );
            erase( fromi );
        }
        else {
            insert( this->path[fromi], toj );
            erase( fromi + 1 );
        }
    };


    /*! \fn void dumpid() const
     * \brief Print the Twbucket using id as node identifiers with the title "Twbucket".
     */
    void dumpid() const {dumpid( "Twbucket" );};


    /*! \fn void dumpid(const std::string &title) const
     * \brief Print the Twbucket using id as node identifiers with user defined title.
     * \param[in] title Title to print with the output of the Twbucket.
     */
    void dumpid( const std::string &title ) const {
        std::cout << title;
        const_iterator it = path.begin();

        for ( const_iterator it = path.begin(); it != path.end(); it++ )
            std::cout << " " << it->getid();

        std::cout << std::endl;
    };

    /*! \fn void dump() const
     * \brief Print the Twbucket using nid as node identifiers with the title "Twbucket".
     */
    void dump() const {dump( "Twbucket" );};


    /*! \fn void dump(const std::string &title) const
     * \brief Print the Twbucket using nid as node identifiers with user defined title.
     * \param[in] title Title to print with the output of the Twbucket.
     */
    void dump( const std::string &title ) const {
        std::cout << title;
        const_iterator it = path.begin();

        for ( const_iterator it = path.begin(); it != path.end(); it++ )
            std::cout << " " << it->getnid();

        std::cout << std::endl;
    };

    // --------------- set operations tools -------------------------


    /*! \fn bool hasId(const knode &node) const
     * \brief Check if a node in the bucket has the same id as node.
     * \param[in] node See if this node is in the bucket based on its id.
     * \return true if a node with the same id was found.
     */
    bool hasId( const knode &node ) const { return hasid( node.getid() ); };


    /*! \fn bool hasId(UID id) const
     * \brief Check if a node in the bucket has this id.
     * \return true if a node with this id was found.
     */
    bool hasId( UID id ) const {
        const_reverse_iterator rit = path.rbegin();

        for ( const_iterator it = path.begin(); it != path.end() ; it++, ++rit ) {
            if ( it->getid() == id ) return true;

            if ( rit->getid() == id ) return true;
        }

        return false;
    };


    /*! \fn bool has(const knode &node) const
     * \brief Check if a node in the bucket has the same nid as node.
     * \param[in] node See if this node is in the bucket based on its nid.
     * \return true if a node with the same nid was found.
     */
    bool has( const knode &node ) const { return has( node.getnid() ); };


    /*! \fn bool has(UID nid) const
     * \brief Check if a node in the bucket has this nid.
     * \param[in] nid Check if a node in the bucket has this nid
     * \return true if a node with the same nid was found.
     */
    bool has( UID nid ) const {
        const_reverse_iterator rit = path.rbegin();

        for ( const_iterator it = path.begin(); it != path.end() ; it++, ++rit ) {
            if ( it->getnid() == nid ) return true;

            if ( rit->getnid() == nid ) return true;
        }

        return false;
    };


    /*! \fn bool operator ==(const TwBucket<knode> &other) const
     * \brief Compare two buckets and report of they are equal or not.
     */
    bool operator ==( const TwBucket<knode> &other ) const  {
        if ( size() != other.size() ) return false;

        if ( size() == other.size() == 0 ) return true;

        if ( ( ( *this ) - other ).size() != 0 ) return false;

        if ( ( other - ( *this ) ).size() != 0 ) return false;

        return true;
    }


    /*! \fn TwBucket<knode>& operator =(const TwBucket<knode> &other)
     * \brief Copy assignment of another bucket to this bucket.
     *
     * Clears the contents of the current bucket and copies the other
     * bucket into the current bucket.
     *
     * \prarms[in] other Bucket that will get copy assigned to this bucket.
     */
    TwBucket<knode> &operator =( const TwBucket<knode> &other )  {
        TwBucket<knode> b = other;
        path.clear();
        path.insert( path.begin(), b.path.begin(), b.path.end() );
        return *this;
    }


    // ----------- set doesnt mind order of nodes ---------------------

    /*! \fn TwBucket<knode> operator +(const TwBucket<knode> &other) const
     * \brief Perform a set UNION operation of two buckets.
     *
     * If A, B, and newBucket are TwBuckets then newBucket = A + B performs
     * a set union of A and B.
     *
     * \warning Set union does not preserve order of node.
     *
     * \param[in] other The bucket to be unioned with this bucket.
     * \return The set union of buckets this and other.
     */
    TwBucket<knode> operator +( const TwBucket<knode> &other ) const  {
        std::set<knode, compNode> a;
        a.insert( path.begin(), path.end() );
        a.insert( other.path.begin(), other.path.end() );
        TwBucket<knode> b;
        b.path.insert( b.path.begin(), a.begin(), a.end() );
        return b;
    }

    /*! \fn TwBucket<knode> operator +=(const TwBucket<knode> &other) const
     * \brief Perform a set UNION operation of this bucket and another bucket.
     *
     * If A and B are TwBuckets then A += B is equivalent to A = A + B and
     * performs a set union of A and B into A.
     *
     * \warning Set union does not preserve order of node.
     *
     * \param[in] other The other bucket to operate on.
     * \return The set union of two buckets.
     */
    TwBucket<knode> &operator +=( const TwBucket<knode> &other )  {
        std::set<knode, compNode> a;
        a.insert( path.begin(), path.end() );
        a.insert( other.path.begin(), other.path.end() );
        path.clear();
        path.insert( path.begin(), a.begin(), a.end() );
        return *this;
    }

    /*! \fn TwBucket<knode> operator *(const TwBucket<knode> &other) const
     * \brief Perform a set INTERSECTION operation between two buckets.
     *
     * If A, B, and newBucket are TwBuckets then newBucket = A * B performs
     * a set intersection of A and B.
     *
     * \warning Set intersection does not preserve order of node.
     *
     * \param[in] other The other bucket to operate on.
     * \return The set intersection of two buckets.
     */
    TwBucket<knode> operator *( const TwBucket<knode> &other ) const  {
        std::set<knode, compNode> s1;
        std::set<knode, compNode> s2;
        std::set<knode, compNode> intersect;
        s1.insert( path.begin(), path.end() );
        s2.insert( other.path.begin(), other.path.end() );
        std::set_intersection( s1.begin(), s1.end(), s2.begin(), s2.end(),
                               std::inserter( intersect, intersect.begin() ) );
        TwBucket<knode> b;
        b.path.insert( b.path.begin(), intersect.begin(), intersect.end() );
        return b;
    }

    /*! \fn TwBucket<knode>& operator *=(const TwBucket<knode> &other)
     * \brief Perform a set INTERSECTION operation of this and another bucket.
     *
     * If A and B TwBuckets then A *= B is equivalent to A = A * B and performs
     * a set intersection of A and B into A.
     *
     * \warning Set intersection does not preserve order of node.
     *
     * \param[in] other The other bucket to operate on.
     * \return The set intersection of two buckets.
     */
    TwBucket<knode> &operator *=( const TwBucket<knode> &other )  {
        std::set<knode, compNode> s1;
        std::set<knode, compNode> s2;
        std::set<knode, compNode> intersect;
        s1.insert( path.begin(), path.end() );
        s2.insert( other.path.begin(), other.path.end() );
        std::set_intersection( s1.begin(), s1.end(), s2.begin(), s2.end(),
                               std::inserter( intersect, intersect.begin() ) );
        path.clear();
        path.insert( path.begin(), intersect.begin(), intersect.end() );
        return *this;
    }

    /*! \fn TwBucket<knode> operator -( const TwBucket<knode> &other ) const
     * \brief Perform a set DIFFERENCE operation of this and another bucket.
     *
     * If A, B, and newBucket are TwBuckets then newBucket = A - B performs
     * a set difference of A minus B.
     *
     * \warning Set difference does not preserve order of node.
     *
     * \param[in] other The other bucket to operate on.
     * \return The set difference of two buckets.
     */
    TwBucket<knode> operator -( const TwBucket<knode> &other ) const  {
        std::set<knode, compNode> s1;
        std::set<knode, compNode> s2;
        std::set<knode, compNode> diff;
        s1.insert( path.begin(), path.end() );
        s2.insert( other.path.begin(), other.path.end() );
        std::set_difference( s1.begin(), s1.end(), s2.begin(), s2.end(),
                             std::inserter( diff, diff.begin() ) );
        TwBucket<knode> b;
        b.path.insert( b.path.begin(), diff.begin(), diff.end() );
        return b;
    }

    /*! \fn TwBucket<knode> operator -=( const TwBucket<knode> &other )
     * \brief Perform a set DIFFERENCE operation of this and another bucket.
     *
     * If A and B are TwBuckets then A -= B performs
     * a set difference of A minus B on bucket A.
     *
     * \warning Set difference does not preserve order of node.
     *
     * \param[in] other The other bucket to operate on.
     * \return The set difference of two buckets.
     */
    TwBucket<knode> &operator -=( const TwBucket<knode> &other )  {
        std::set<knode, compNode> s1;
        std::set<knode, compNode> s2;
        std::set<knode, compNode> diff;
        s1.insert( path.begin(), path.end() );
        s2.insert( other.path.begin(), other.path.end() );
        std::set_difference( s1.begin(), s1.end(), s2.begin(), s2.end(),
                             std::inserter( diff, diff.begin() ) );
        path.clear();
        path.insert( path.begin(), diff.begin(), diff.end() );
        return *this;
    }

    // -------------------- End of Path Tools ----------------------------

    /*! \fn double getTotTravelTime() const
     * \brief Get the total travel time of the path.
     *
     * The last node in the path contains some path statistics. This method
     * fetches the total tavel time from start to end of path.
     *
     * \return The total travel time of the path.
     * \sa Vehicle::getCost(), Vehicle::getCostOSRM()
     */
    double getTotTravelTime() const {
        assert ( size() );
        return path[size() - 1].getTotTravelTime();
    };

    /*! \fn double getTotWaitTime() const
     * \brief Get the total wait time of the path.
     *
     * The last node in the path contains some path statistics. This method
     * fetches the total wait time from start to end of path.
     *
     * \return The total wait time of the path.
     */
    double getTotWaitTime() const {
        assert ( size() );
        return path[size() - 1].getTotWaitTime();
    };

    /*! \fn double getTotServiceTime() const
     * \brief Get the total service time of the path based on the last node in the path.
     *
     * The last node in the path contains some path statistics. This method
     * fetches the total service time from start to end of path.
     *
     * \return The total service time of the path.
     */
    double getTotServiceTime() const {
        assert ( size() );
        return path[size() - 1].getTotServiceTime();
    };

    /*! \fn double getDumpVisits() const
     * \brief Get the total number of dump visits of the path.
     *
     * The last node in the path contains some path statistics. This method
     * fetches the total number of dump visits from start to end of path.
     *
     * \return The total number of dump visits of the path.
     */
    double getDumpVisits() const {
        assert ( size() );
        return path[size() - 1].getDumpVisits();
    };

    /*! \fn double getDepartureTime() const
     * \brief Get the departure time of the last node in the path.
     *
     * The last node in the path contains some path statistics. This method
     * fetches the departure time of the last node in the path.
     *
     * \return The departure time of the last node in the path.
     */
    double getDepartureTime() const {
        assert ( size() );
        return path[size() - 1].getDepartureTime();
    };

    /*! \fn int getTwvTot() const
     * \brief Get the total number of time window violations in the path.
     *
     * The last node in the path contains some path statistics. This method
     * fetches the total number of time window violations in the path.
     *
     * \return The total number of time window violations in the path.
     */
    int getTwvTot() const {
        assert ( size() );
        return path[size() - 1].gettwvTot();
    };

    /*! \fn int getCvTot() const
     * \brief Get the total number of capacity violations in the path.
     *
     * The last node in the path contains some path statistics. This method
     * fetches the total number of capacity violations in the path.
     *
     * The smart evaluation function will always have zero capacity violations
     * because they automaticially place dump nodes as required to avoid
     * capacity violations, but there are methods that do not do this so
     * this method will report if they exist in the path.
     *
     * \return The total number of capacity violations in the path.
     */
    int getCvTot() const {
        assert ( size() );
        return path[size() - 1].gettwvTot();
    };

    /*! \fn double getTotCargo() const
     * \brief Get the total cargo at the end of the route.
     *
     * The last node in the path contains some path statistics. This method
     * fetches the total cargo at the end of the route.
     *
     * The smart evaluation function will always end the path with a final
     * run to the dump to unload any cargo in the vehicle, but there are
     * methods that do not do this so this method will report if there is
     * cargo at the end of the path.
     *
     * \return The total cargo at the end of the route..
     */
    double getTotCargo() const {
        assert ( size() );
        return path[size() - 1].getcargo();
    };


    // ---------- ID based tools  to NID tools ---------------------------

    /*! \fn long int getNidFromId( UID id ) const
     * \brief Get the internal node id associated with the user id.
     * \param[in] id The user id for the node.
     * \return The internal node id or -1 if user id was not found.
     */
    long int getNidFromId( UID id ) const {
        const_reverse_iterator rit = path.rbegin();

        for ( const_iterator it = path.begin(); it != path.end() ; it++, ++rit ) {
            if ( it->getid() == id ) return it->getnid();

            if ( rit->getid() == id ) return rit->getnid();
        }

        return -1;
    };


    /*! \fn long int posFromId( UID id ) const
     * \brief Get the position in the path where id is located.
     * \param[in] id The user id for the node.
     * \return The position in the path or -1 if it is not found.
     */
    long int posFromId( UID id ) const {
        for ( const_iterator it = path.begin(); it != path.end() ; it++ ) {
            if ( it->getid() == id ) return int( it - path.begin() );
        }

        return -1;
    };


    // ------------------  NID tools  -------------------------------

    /*! \fn long int pos( const knode &node ) const
     * \brief Get the position of node in the path
     * \param[in] node A node object that we want to locate in the path
     * \return returns the position of node in the path or -1 if it's not found.
     */
    long int pos( const knode &node ) const { return pos( node.getnid() ); };

    /*! \fn long int pos( UID nid ) const
     * \brief Get the position of node id in the path
     * \param[in] nid The node id we want to locate in the path
     * \return The position of node id in the path or -1 if it's not found.
     */
    long int pos( UID nid ) const {
        for ( const_iterator it = path.begin(); it != path.end() ; it++ ) {
            if ( it->getnid() == nid ) return int( it - path.begin() );
        }

        return -1;
    };


    /*! \fn std::deque<int> getpath() const
     * \brief Get a deque of nids that are in the path.
     * \return A deque of the nids in the path.
     */
    std::deque<int> getpath() const {
        std::deque<int> p;

        for ( const_iterator it = path.begin(); it != path.end(); it++ )
            p.push_back( it->getnid() );

        return p;
    };


    // ------ deque like functions   POSITION based functions  -------

    /*! \fn void insert( const knode &node, UID atPos )
     * \brief Insert node into path at position atPos
     * \param[in] node The node to insert
     * \param[in] atPos The position it should be inserted at
     */
    void insert( const knode &node, UID atPos ) {
        assert( atPos <= path.size() );
        path.insert( path.begin() + atPos, node );
    };


    /*! \fn void erase ( int atPos )
     * \brief Erase the node at location atPos
     * \param[in] atPos The position of the node to be erased.
     */
    void erase ( int atPos ) {
        assert( atPos < path.size() );
        path.erase( path.begin() + atPos );
    };


    /* \fn void erase ( const knode &node )
     * \brief Erase node from within the path.
     * \param[in] node The node to be erased.
     */
    void erase ( const knode &node ) {
        int atPos = pos( node.getnid() );
        assert( atPos < path.size() );
        path.erase( path.begin() + atPos );
    };


    /*! \fn void erase ( int fromPos, int toPos )
     * \brief Erase all node between fromPos and toPos inclusive.
     * \param[in] fromPos Position of the start of the range to be erased.
     * \param[in] toPos Position of the last in the range to be erased.
     * \warning If fromPos and toPos are reversed it will still erase the range.
     */
    void erase ( int fromPos, int toPos ) {
        assert( fromPos < path.size() );
        assert( toPos < path.size() );

        if ( fromPos == toPos ) {
            //[fromPos]
            path.erase( fromPos );
        }
        else if ( fromPos < toPos ) {
            //[fromPos,toPos)
            path.erase( path.begin() + fromPos, path.begin() + toPos );
        }
        else {
            //[toPos,fromPos)
            path.erase( path.begin() + toPos, path.begin() + fromPos );
        }
    };

    void push_back( const knode &n ) { path.push_back( n ); };
    void push_front( const knode &n ) { path.push_front( n ); };
    void pop_back() { path.pop_back(); };
    void pop_front() { path.pop_front(); };
    void resize( unsigned int n ) {
        assert( n <= path.size() );
        path.resize( n );
    };
    void clear() { path.clear(); };
    unsigned int max_size() const { return path.max_size(); };
    unsigned int size() const { return path.size(); };
    bool empty() const { return path.empty(); };
    std::deque<knode> &Path() { return path; }
    const std::deque<knode> &Path() const  { return path; }
    knode &operator[]( unsigned int n ) {
        assert( n < path.size() );
        return path[n];
    };
    const knode  &operator[] ( unsigned int n ) const {
        assert( n < path.size() );
        return path[n];
    };
    knode &at( int n ) {
        assert( n < path.size() );
        return path.at( n );
    };
    const knode &at( int n ) const  {
        assert( n < path.size() );
        return path.at( n );
    };
    knode &front() { return path.front(); };
    const knode &front() const { return path.front(); };
    knode &back() { return path.back(); };
    const knode &back() const { return path.back(); };


};

template <class knode>
std::vector<std::vector<double> > TwBucket<knode>::TravelTime ;


#endif


