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
#ifndef VRP_OSRMCLIENT_H
#define VRP_OSRMCLIENT_H

#include <string>
#include <deque>
#include <vector>

#include <osrm/coordinate.hpp>
#include <osrm/route_parameters.hpp>
#include <osrm/json_container.hpp>

#ifdef DOVRPLOG
#include "logger.h"
#endif

#include "twnode.h"


// load our assert to throw macros and tell rapidjson to use them
#include "vrp_assert.h"
#define RAPIDJSON_ASSERT assert
#include <rapidjson/document.h>

#include "timer.h"
#include "stats.h"


/*! \class OsrmClient
 * \brief This class provides a shared memory connection to OSRM.
 *
 * This class interfaces with OSRM via a shared memory connection and wraps
 * the interface into a simple class to abstract the features we need
 * access to. This interface is approximately 50 time faster than using
 * the URL based interface.
 *
 * \todo This iterface style receives OSRM results as json text documents
 *       and we have to parse them. I might be worth the effort to dig deeper
 *       into the OSRM code to avoid this step.
 */

class OSRM;
class OsrmClient
{

private:
  ///< The OSRM request structure
  RouteParameters route_parameters;
  ///< Current state of the object
  int status;
  ///< An error message if an error is reported.
  std::string err_msg;
  ///< the json response document
  std::string httpContent;
  ///< once set to false, it doesnt try to make a connection
  static bool connectionAvailable;
  static OSRM  *routing_machine;
  static OsrmClient *p_osrm;
  OsrmClient();
  OsrmClient( const OsrmClient &other );
  OsrmClient &operator=( const OsrmClient & );
  bool use, addPenalty;

public:
  static OsrmClient *Instance() {
    if ( !p_osrm ) // Only allow one instance of class to be generated.
      p_osrm = new OsrmClient;

    return p_osrm;
  }

  void clear();
  void addViaPoint( double lat, double lon );
  void addViaPoint( const Twnode &node );
  void addViaPoints( const std::deque<Twnode> &path );

  /*!
   * \brief Set whether you want the path geometry returned.
   *
   * This should be left as False because it is much faster it you
   * do not need the geometry. It defaults to false.
   *
   * \param[in] want True or False if you want the geometry returned.
   */
  void setWantGeometry( bool want )
  {
    route_parameters.geometry = want;
    route_parameters.compression = false;
  }
  void setWantGeometryText( bool want )
  {
    route_parameters.geometry = want;
    route_parameters.compression = true;
  }
  void usePenalty( bool desition ) { addPenalty = desition; }
  bool getPenalty() const { return addPenalty; }
  void useOsrm( bool desition ) { use = desition; }
  bool getUse( ) const { return use; }
  bool getOsrmViaroute();
  bool getOsrmTime( double lat1, double lon1 , double lat2, double lon2,
                    double &time );
  bool getOsrmTime( double lat1, double lon1 , double lat2, double lon2,
                    const std::string &hint1, const std::string &hint2, double &time );
  bool getOsrmTime( const Twnode &node1, const Twnode &node2, double &time );
  bool getOsrmTime( const Twnode &node1, const Twnode &node2, const Twnode &node3,
                    double &time );
  bool getOsrmTime(const Twnode &node1, const Twnode &node2, const Twnode &node3,
                    const Twnode &node4, double &time );
  bool getOsrmTime( double &time );
  bool getOsrmTimes( std::deque<double> &times );
  bool getOsrmGeometry( std::deque<Node> &geom );
  bool getOsrmGeometryText( std::string &geomText );
  bool getOsrmHints( std::deque<std::string> &hints );
  bool getOsrmStreetNames( std::deque<std::string> &names);
  bool getOsrmNamesOnRoute( std::deque<std::string> &names);
  int getStatus() const { return status; }
  int getConnection() const { return connectionAvailable; }
  std::string getErrorMsg() const { return err_msg; }
  std::string getHttpContent() const { return httpContent; }
  bool testOsrmClient(
    double x1, double y1,
    double x2, double y2,
    double x3, double y3);
  /*!
   * \brief Get coordinates of the neareast fisical node (OSRM node) for a point
   *
   * \param[in] Point longitude.
   * \param[in] Point latitude.
   * \param[out] Node longitude.
   * \param[out] Node latitude.
   * \return true on succes.
   */
  bool getOsrmLocate(double ilon, double ilat , double &olon, double &olat);
  /*!
   * \brief Get coordinates of the nearest point (virtual node) in the nearest edge (OSRM) for a point and edege name
   *
   * \param[in] Point longitude.
   * \param[in] Point latitude.
   * \param[out] Virtual node (in edge) longitude.
   * \param[out] Virtual node (in edge) latitude.
   * \param[out] OSRM forward node Id.
   * \param[out] OSRM reverse node Id.
   * \param[out] OSRM forward weight.
   * \param[out] OSRM reverse weight.
   * \param[out] OSRM edge name id.
   * \param[out] OSRM name.
   *
   * \return true on succes.
   *
   */
  bool getOsrmNearest(double ilon, double ilat ,
    double &olon, double &olat, unsigned int &one_way,
    unsigned int &forward_id, unsigned int &reverse_id,
    unsigned int &forward_wt, unsigned int &reverse_wt, unsigned int &street_id);

private:
  bool getTime( rapidjson::Document &jtree, double &time );
  bool getTimes( rapidjson::Document &jsondoc, std::deque<double> &times );
  bool getGeom( rapidjson::Document &jtree, std::deque<Node> &geom );
  bool getGeomText( rapidjson::Document &jtree, std::string &geomText );
  bool getHints( rapidjson::Document &jtree, std::deque<std::string> &hints );
  bool getNames( rapidjson::Document &jtree, std::deque<std::string> &names );
  bool getNamesOnRoute( rapidjson::Document &jsondoc, std::deque<std::string> &names );
  bool getOsrmPenalty( double &penalty );
  bool getPenalty( rapidjson::Document &jtree, double &penalty );

public:
#ifdef DOVRPLOG
  void dump() {
    DLOG( INFO ) << "----- OsrmClient ----------"
                 << "\nstatus: " << status
                 << "\nerr_msg: " << err_msg
                 << "\ncoordinates.size(): " << route_parameters.coordinates.size()
                 << "\nhttpContent: " << httpContent;
  }
#endif
};

#define osrmi OsrmClient::Instance()

#endif
