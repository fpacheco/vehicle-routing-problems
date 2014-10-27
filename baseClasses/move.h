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
#ifndef MOVE_H
#define MOVE_H

#include <limits>
#include <cassert>

// This class defines a move object that can be placed on the Tabu list
// and/or can be applied to a given solution to transform it to a new state
// setting attributes to -1 means they are undefined and should be ignored.
//
// We are contemplating three different moves Ins, InterSw, IntraSw
// Ins (insert)
//  - remove a nid from vid1 at pos1 and insert it into vid2 as pos2
// InterSw (inter vehicle swap)
//  - exchange a node with another node in another vehicle
//    swap nid1 at pos1 in vid1 with nid2 at pos2 in vid2
// IntraSw (intra vehicle swap)
//  - exchange nid1 and nid2 in the same vehicle


class Move {
  public:
    typedef enum { Invalid = -1, Ins = 0, IntraSw = 1, InterSw = 2 } Mtype;

  private:
    Mtype mtype;      // type of move
    int nid1;       // node 1
    int nid2;       // node 2
    int vid1;       // vehicle 1
    int vid2;       // vehicle 2
    int pos1;       // position 1
    int pos2;       // position 2
    double savings; // saving generated by this move

  public:
    Move() { mtype = Invalid; nid1 = nid2 = vid1 = vid2 = pos1 = pos2 = -1; savings = -std::numeric_limits<double>::max(); };
    Move(Mtype _mtype, int _nid1, int _nid2, int _vid1, int _vid2, int _pos1, int _pos2, double _sav) {
        mtype = _mtype;
        nid1 = _nid1;
        nid2 = _nid2;
        vid1 = _vid1;
        vid2 = _vid2;
        pos1 = _pos1;
        pos2 = _pos2;
        savings = _sav;
    };

    int getmtype() const { return mtype; };
    int getnid1() const { return nid1; };
    int getnid2() const { return nid2; };
    int getvid1() const { return vid1; };
    int getvid2() const { return vid2; };
    int getpos1() const { return pos1; };
    int getpos2() const { return pos2; };
    double getsavings() const { return savings; };

    bool less(const Move& m) const;
    bool operator==(const Move &rhs) const;
    bool operator<(const Move &rhs) const { return this->less(rhs); };
    bool isForbidden(const Move &tabu) const;
    void dump() const;

    void setmtype(Mtype _mtype) { mtype = _mtype; };
    void setnid1(int nid) { nid1 = nid; };
    void setnid2(int nid) { nid2 = nid; };
    void setvid1(int vid) { vid1 = vid; };
    void setvid2(int vid) { vid2 = vid; };
    void setpos1(int pos) { pos1 = pos; };
    void setpos2(int pos) { pos2 = pos; };
    void setsavings(double save) { savings = save; };

    static bool bySavings(const Move& a, const Move& b) { return (a.getsavings()==b.getsavings())?a<b:a.getsavings()>b.getsavings(); };
    static bool bySavingsA(const Move& a, const Move& b) { return a.getsavings()<b.getsavings(); };

    int getInsFromTruck() const { assert(mtype==Move::Ins); return vid1; };
    int getInsToTruck() const { assert(mtype==Move::Ins); return vid2; };
    int getIntraSwTruck() const { assert(mtype==Move::IntraSw); return vid1; };
    int getInterSwTruck1() const { assert(mtype==Move::InterSw); return vid1; };
    int getInterSwTruck2() const { assert(mtype==Move::InterSw); return vid2; };

};

#endif
