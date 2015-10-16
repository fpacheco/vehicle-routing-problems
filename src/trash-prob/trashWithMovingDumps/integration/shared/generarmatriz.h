#ifndef GENERARMATRIZ
#define GENERARMATRIZ

#endif // GENERARMATRIZ
//imports de vrptools
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>
#include <stdio.h>
#include <string>

//imports de twc
#ifdef OSRMCLIENT
#include "osrmclient.h"
#include "phantomnode.h"
#endif

#include "basictypes.h"
#include "node.h"
#include "twnode.h"
#include "twpath.h"
#include "singleton.h"
#include "pg_types_vrp.h"
#include "signalhandler.h"

bool checkOsrmClient()
{

}
void setPhantomNodes() {
  // Multiplier for before and after
  double mb = 0.70;
  double ma = 1.30;
  // Delete previous
  mPhantomNodes.clear();
  #ifdef VRPMINTRACE
    DLOG(INFO) << "mPhantonNodes cleared!";
    DLOG(INFO) << "original have " << original.size() << " elements!";
  #endif

  // Variables
  bool oldStateOsrm;
  // PhantomNode
  double phaNLon, phaNLat;
  // Physical Node
  double phyNLon, phyNLat;
  unsigned int one_way;
  unsigned int fw_id, rv_id, fw_wt, rv_wt, street_id;

  // Backup OSRM state
  oldStateOsrm = osrmi->getUse();
  osrmi->useOsrm(true);  //forcing osrm usage
  osrmi->clear();

  int pncount;
  pncount = 0;

  // All nodes must have bearing (pickups, dumps, etc)
  for (UINT i = 0; i < original.size(); i++) {
    // Only picukp has phantom nodes (OBSOLETE!)
    // if ( original[i].isPickup() ) {
    one_way = 100;

    // Custom/modified version of nearest plugin
    osrmi->getOsrmNearest( original[i].x(), original[i].y(), phaNLon, phaNLat, one_way, fw_id, rv_id, fw_wt, rv_wt, street_id);

    // Get nearest fisical OSRM node (edge intersection) of phantom
    osrmi->getOsrmLocate(phaNLon, phaNLat, phyNLon, phyNLat);

    // Bearing calculation
    Node phyNode = Node(phyNLon,phyNLat);
    Node phaNode = Node(phaNLon,phaNLat);
    bool ret = original[i].isRightToSegment(phyNode, phaNode);
    double bearing;
    if (ret) {
      bearing = phyNode.bearing(phaNode, false);
    } else {
      bearing = phyNode.bearing(phaNode, true);
    }
    // Set pn bearing
    pn.setBearing(bearing);
    //get all the times using osrm
    bool oldStateOsrm = osrmi->getUse();
    osrmi->useOsrm(true);  //forcing osrm usage
    osrmi->clear();

    // To build the call
    std::vector< Twnode > call;
    std::vector< double > bearings;

    // Add nodes and bearings to data containers
    for (unsigned int i = 0; i < nodesOnPath.size(); ++i) {
      double bearing;
      if ( getBearingForNId(nodesOnPath[i].nid(), bearing) ) {
        bearings.push_back(bearing);
        call.push_back(nodesOnPath[i]);
      } else {
    #ifdef VRPMINTRACE
        DLOG(INFO) << "Error: Node " << nodesOnPath[i].nid() << "(" << nodesOnPath[i].id() << ") have no bearing!";
    #endif
      }
    }

    // Add point to the call
    osrmi->addViaPoints(call, bearings);
    if (!osrmi->getOsrmViaroute()) {
      #ifdef VRPMINTRACE
          DLOG(INFO) << "getOsrmViaroute failed";
      #endif
      osrmi->useOsrm(oldStateOsrm);
      return;
    }

    // To store time returned
    std::deque< double > times; //usar time , tiempo total
    if (!osrmi->getOsrmTimes(times)){
      #ifdef VRPMINTRACE

        std::stringstream ss;
        ss.precision(6);
        ss << std::fixed;

        ss << "http://localhost:5000/viaroute?";

        DLOG(INFO) << "getOsrmTimes failed";
        DLOG(INFO) << "\tNID\tID\tBEARING\t";
        for (unsigned int i = 0; i < call.size(); ++i) {
            DLOG(INFO) << "\t" << call[i].nid() << "\t" << call[i].id() << "\t" << bearings[i] << "\t";
            ss << "loc=" << call[i].y() << "," << call[i].x() << "&b=" << static_cast<int>(bearings[i]) << "&";
        }
        std::string s = ss.str();
        DLOG(INFO) << s.substr(0, s.size()-1);
      #endif
      osrmi->useOsrm(oldStateOsrm);
      return;
    }



    //TERMINA
    // Check one_way and two_ways streets
    if (one_way == 1) {
#ifdef VRPMINTRACE
      DLOG(INFO) << original[i].id() << " [lon,lat] " << original[i].x() << original[i].y() << " is one way street with bearing " << bearing;
#endif
    } else if (one_way == 0) {
#ifdef VRPMINTRACE
      DLOG(INFO) << original[i].id() << " [lon,lat] " << original[i].x() << original[i].y() << " is in two way street! " << bearing;
#endif
      // Add before and after to pn
      double alon, alat, blon, blat;
      // WARNING: longitude and latitude!!!!!!
      // Only valid for very short distances
      //
      // I think
      //
      // FN---------PN
      //            |
      //         original[i]
      //
      // Before
      blon = phyNLon + mb * (phaNLon - phyNLon);
      blat = phyNLat + mb * (phaNLat - phyNLat);
      // After
      alon = phyNLon + ma * (phaNLon - phyNLon);
      alat = phyNLat + ma * (phaNLat - phyNLat);

      /*
              #ifdef VRPMINTRACE
                  std::cout << std::setprecision(8) << "PN: (" << pnlon << "," << pnlat << ")" << std::endl;
                  std::cout << "FN: (" << phyNLon << "," << phyNLat << ")" << std::endl;
                  std::cout << "Before: (" << blon << "," << blat << ")" << std::endl;
                  std::cout << "After: (" << alon << "," << alat << ")" << std::endl;
              #endif
      */

      Point pb, pa;
      if (ret) {
        pb = Point(blon,blat);
        pa = Point(alon,alat);
        pn.setBeforePNode( pb );
        pn.setAfterPNode( pa );
      } else {
        // Not as you think!!!
        //
        //         original[i]
        //            |
        // FN---------PN
        //
        pb = Point(alon,alat);
        pa = Point(blon,blat);
        pn.setBeforePNode( pb );
        pn.setAfterPNode( pa );
      }
    }

#ifdef VRPMINTRACE
    DLOG(INFO) << std::setprecision(8) << "PhantomNode";
    DLOG(INFO) << pn;
#endif

    // Add pn to de map. Map i with nid (internal node id) NOT id (user node id).
    mPhantomNodes[ original[i].nid() ] = pn;
    pncount++;
    //} // if pickup
  }
  osrmi->useOsrm(oldStateOsrm);

  #ifdef VRPMINTRACE
      DLOG(INFO) << "Begin PhantomNodes for pickups sites";
      DLOG(INFO) << "\t" << "CONID" << "\t" << "COID" << "\t" << "COLON" << "\t" << "COLAT" << "\t" << "PNID" << "\t" << "PNLON" << "\t" << "PNLAT" << "\t"
                 << "BEARING" << "\t" << "BELON" << "\t" << "BELAT" << "\t" << "AFLON" << "\t" << "AFLAT";
      for (UINT i = 0; i < original.size(); i++) {
          UID nid = original[i].nid();
          auto it = mPhantomNodes.find( nid );
          if ( it!=mPhantomNodes.end() ) {
              DLOG(INFO) << std::setprecision(8) << "\t" << original[i].nid() << "\t" << original[i].id() << "\t" << original[i].x() << "\t"  << original[i].y() << "\t"
                         << it->second.id() << "\t" << it->second.point().x() << "\t" << it->second.point().y() << "\t"
                         << it->second.bearing() << "\t"
                         << it->second.beforePNode().x() << "\t" << it->second.beforePNode().y() << "\t"
                         << it->second.afterPNode().x() << "\t" << it->second.afterPNode().y();
          }
      }
      DLOG(INFO) << "End PhantomNodes for pickups sites";
  #endif
}
bool readDataFromFiles(std::string fileBasePath)
{
    //copiado de bool VRPTools::readDataFromFiles(std::string fileBasePath)

    // .containers.txt .otherlocs.txt .vehicles.txt .dmatrix-time.txt
    LoadFromFiles loader(fileBasePath);
    mContainers = loader.getContainers(mContainersCount);
    mOtherLocs = loader.getOtherlocs(mOtherLocsCount);
  //  mVehicles = loader.getVehicles(mVehiclesCount);
   // mTimeTable = loader.getTtimes(mTimeTableCount);
#ifdef VRPMINTRACE
    DLOG(INFO) << "mContainersCount: " << mContainersCount;
    DLOG(INFO) << "mOtherLocsCount: " << mOtherLocsCount;
    DLOG(INFO) << "mVehiclesCount: " << mVehiclesCount;
    DLOG(INFO) << "mTimeTableCount: " << mTimeTableCount;
#endif
}
