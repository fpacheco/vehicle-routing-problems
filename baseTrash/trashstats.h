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
#ifndef TRASHSTATS_H
#define TRASHSTATS_H

#include <string>
#include <vector>
#include <map>

#include "singleton.h"

class TrashStats {
  private:
    std::map<std::string, double> stats;

  public:

    TrashStats() { stats.clear(); };
    ~TrashStats() {};

    double getval(const std::string key) const;
    std::vector<std::string> getkeys() const;
    void dump(const std::string title) const;

    void inc(const std::string key);
    void set(const std::string key, double val);
    void addto(const std::string key, double val);
    void clear() { stats.clear(); };
};

typedef Singleton<TrashStats> Stats; // Global declaration

#define STATS Stats::Instance()

#endif
/*
    Then you can access parameters via:

    STATS->method();
*/
