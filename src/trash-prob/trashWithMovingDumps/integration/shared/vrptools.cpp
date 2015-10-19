#include <stdlib.h> /* malloc, calloc, realloc, free */

#include "vrptools.h"
#include "loadfromfiles.h"

#include "trashconfig.h"
#include "trashprob.h"
#include "truckManyVisitsDump.h"
#include "fleetOpt.h"

VRPTools::VRPTools():
    mRightSide(false),
    mReady(false),
    mNIters(100),
    mContainersCount(0),
    mVehiclesCount(0),
    mOtherLocsCount(0),
    mTimeTableCount(0)
{
    // Set logs
    mLogDir = fs::temp_directory_path();
    mLogFile = fs::path("libvrptools");
    mUseOsrm = osrmAvailable();

#ifdef DOVRPLOG
  if ( not google::IsGoogleLoggingInitialized() ) {
    FLAGS_log_dir = mLogDir.c_str();
    google::InitGoogleLogging( mLogFile.c_str() );
    FLAGS_logtostderr = 0;
    FLAGS_stderrthreshold = google::FATAL;
    FLAGS_minloglevel = google::INFO;
    FLAGS_logbufsecs = 0;
    // Shutdown
    //google::ShutdownGoogleLogging();
  }
#endif

}

void VRPTools::setLogFilePath(std::string logDir, std::string logFileName) {
    fs::path p(logDir);
    if (fs::exists(p) && fs::is_directory(p)) {
        if ( fs::portable_name(logFileName) ) {
            mLogFile = fs::path(logFileName);
            mLogDir = p;
        }
    }
}

bool VRPTools::osrmAvailable() {
    return osrmi->getConnection();
}

void VRPTools::setUseOsrm(bool opt) {
    if (opt) {
        if ( osrmi->getConnection() ) {
            mUseOsrm = opt;
        } else {
            mUseOsrm = false;
        }
    } else {
        mUseOsrm = opt;
    }
}

bool VRPTools::checkOsrmClient()
{
    bool testResult = false;
#ifdef OSRMCLIENT
    osrmi->useOsrm(true);
    testResult = osrmi->testOsrmClient(
        -34.905113, -56.157043,
        -34.906807, -56.158463,
        -34.9076,   -56.157028
    );
#ifdef VRPMINTRACE
    if (testResult)
        DLOG(INFO) << "osrm test passed";
    else
        DLOG(INFO) << "osrm test FAIL";
#endif  // VRPMINTRACE
#endif  // OSRMCLIENT
    return testResult;
}

bool VRPTools::readDataFromFiles(std::string fileBasePath)
{
    // .containers.txt .otherlocs.txt .vehicles.txt .dmatrix-time.txt
    LoadFromFiles loader = LoadFromFiles(fileBasePath);
    mContainers = loader.getContainers(mContainersCount);
    mOtherLocs = loader.getOtherlocs(mOtherLocsCount);
    mVehicles = loader.getVehicles(mVehiclesCount);
    mTimeTable = loader.getTtimes(mTimeTableCount);

#ifdef VRPMINTRACE
    DLOG(INFO) << "mContainersCount: " << mContainersCount;
    DLOG(INFO) << "mOtherLocsCount: " << mOtherLocsCount;
    DLOG(INFO) << "mVehiclesCount: " << mVehiclesCount;
    DLOG(INFO) << "mTimeTableCount: " << mTimeTableCount;
#endif
}

bool VRPTools::readContainersFromFile(std::string fileBasePath)
{
  LoadFromFiles loader = LoadFromFiles();
  loader.load_containers(fileBasePath + ".containers.txt");
  mContainers = loader.getContainers(mContainersCount);
#ifdef VRPMINTRACE
  DLOG(INFO) << "mContainersCount: " << mContainersCount;
#endif
}

bool VRPTools::readOtherLocsFromFile(std::string fileBasePath)
{
  LoadFromFiles loader = LoadFromFiles();
  loader.load_otherlocs(fileBasePath + ".otherlocs.txt");
  mOtherLocs = loader.getOtherlocs(mOtherLocsCount);
#ifdef VRPMINTRACE
  DLOG(INFO) << "mOtherLocsCount: " << mOtherLocsCount;
#endif
}

bool VRPTools::check()
{
    if (mContainers && mOtherLocs && mTimeTable && mVehicles) {
        // Backup
        bool oldStateOsrm = osrmi->getUse();
        osrmi->useOsrm( mUseOsrm );

        TrashProb prob(
            mContainers,
            mContainersCount,
            mOtherLocs,
            mOtherLocsCount,
            mTimeTable,
            mTimeTableCount,
            mVehicles,
            mVehiclesCount,
            1 // Only check, not run!
        );
        bool ret = false;
        if ( prob.isValid() or prob.getErrorsString().size() == 0 ) {
            ret = true;
        } else {
            ret = false;
        #ifdef VRPMINTRACE
            DLOG(INFO) << "Errors: " << prob.getErrorsString();
        #endif
            //*data_err_msg = strdup( prob.getErrorsString().c_str() );
        }
        twc->cleanUp();

        // Restore
        osrmi->useOsrm( oldStateOsrm );

        return ret;
    } else {
        return false;
    }
}

void VRPTools::solve()
{
    if (mContainers && mOtherLocs && mTimeTable && mVehicles) {

        bool oldStateOsrm = osrmi->getUse();
        osrmi->useOsrm( mUseOsrm );

        TrashProb prob(
            mContainers,
            mContainersCount,
            mOtherLocs,
            mOtherLocsCount,
            mTimeTable,
            mTimeTableCount,
            mVehicles,
            mVehiclesCount,
            0 // Not check, run!
        );

    #ifdef DOVRPLOG
        DLOG(INFO) << "Datos del problema";
        prob.dumpdataNodes();
        prob.dumpDepots();
        prob.dumpDumps();
        prob.dumpPickups();
    #endif

        // If not valid -> exit
        if ( !prob.isValid() ) {
        #ifdef DOVRPLOG
            DLOG(INFO) << "Problema no es válido";
            DLOG(INFO) << prob.getErrorsString();
        #endif
            twc->cleanUp();
            return;
        }

        TruckManyVisitsDump tp( prob );
        tp.process(0);
    #ifdef DOVRPLOG
        DLOG(INFO) << "Initial solution: 0 is best";
    #endif

        double best_cost = 9999999;
        Solution best_sol( tp );
        best_cost = best_sol.getCostOsrm();

        for (int icase = 1; icase < 7; ++icase) {
        #ifdef DOVRPLOG
            DLOG(INFO) << "initial solution: " << icase;
        #endif
            tp.process(icase);
            if (best_cost > tp.getCostOsrm()) {
        #ifdef DOVRPLOG
            DLOG(INFO) << "initial solution: " << icase << " is best";
        #endif
            best_cost = tp.getCostOsrm();
            best_sol = tp;
          }
        }

        Optimizer optSol(best_sol, mNIters);
        if (best_cost > optSol.getCostOsrm()) {
            best_cost = optSol.getCostOsrm();
            best_sol = optSol;
        }

    #ifdef DOVRPLOG
        DLOG(INFO) << "=-=-=-=-=-=- OPTIMIZED SOLUTION -=-=-=-=-=-=-=";
        DLOG(INFO) << "Number of containers: " << best_sol.countPickups();
        best_sol.dumpCostValues();
        DLOG(INFO) << "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=";
        best_sol.tau();
        DLOG(INFO) << "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=";
    #endif

        best_sol.dumpSolutionForPg();
        std::vector<std::string> urls = best_sol.getOSRMUrl();

        /*
        std::vector<int> solAsVector = best_sol.solutionAsVectorID();
        for (int i=0; i< solAsVector.size(); i++) {
            std::cout << solAsVector[i] << "\t";
        }
        std::cout << std::endl;
        */

        // Limpio
        twc->cleanUp();

        // Restore
        osrmi->useOsrm( oldStateOsrm );

    } else {

    }
}

<<<<<<< HEAD
void VRPTools::createTimeMatrix(std::string fileBasePath) {

  // .containers.txt .otherlocs.txt .vehicles.txt .dmatrix-time.txt
  LoadFromFiles loader(fileBasePath);
  mContainers = loader.getContainers(mContainersCount);
  mOtherLocs = loader.getOtherlocs(mOtherLocsCount);
  #ifdef VRPMINTRACE
    DLOG(INFO) << "mContainersCount: " << mContainersCount;
    DLOG(INFO) << "mOtherLocsCount: " << mOtherLocsCount;
  #endif
  // Multiplier for before and after
  double mb = 0.70;
  double ma = 1.30;
  // Delete previous
  mPhantomNodes.clear();
  #ifdef VRPMINTRACE
    DLOG(INFO) << "mPhantonNodes cleared!";
    DLOG(INFO) << "Containers have " << mContainersCount << " elements!";
    DLOG(INFO) << "Otherlocs have " << mOtherLocsCount << " elements!";
  #endif
=======
bool VRPTools::createTimeMatrix(const std::string &fileBasePath, std::string &data, std::string &errors)
{

  // Error print stream
  std::stringstream ss;

  // .containers.txt .otherlocs.txt .vehicles.txt .dmatrix-time.txt
  readContainersFromFile(fileBasePath);
  readOtherLocsFromFile(fileBasePath);

  if (mContainersCount <=0 || mOtherLocsCount <=0) {
    ss << "Error reading data" << std::endl;
    errors = ss.str();
    return false;
  }
>>>>>>> 32b96d51e1a57ea2be7509e8c7f2af0508221315

  // Variables
  bool oldStateOsrm;
  // PhantomNode
  double phaNLon, phaNLat;
  // Physical Node
  double phyNLon, phyNLat;
  unsigned int one_way;
  unsigned int fw_id, rv_id, fw_wt, rv_wt, street_id;

<<<<<<< HEAD
  // Backup OSRM state
  oldStateOsrm = osrmi->getUse();
  osrmi->useOsrm(true);  //forcing osrm usage
  osrmi->clear();

  int pncount;
  pncount = 0;
  TwBucket <knode> nodesOnPath;

  std::stringstream ss;
  bool errorBearing = false;
  // All nodes must have bearing (pickups, dumps, etc)
  for (UINT i = 0; i < mContainersCount; i++) {
    // Only picukp has phantom nodes (OBSOLETE!)
    // if ( mContainers[i]->isPickup() )
    one_way = 100;

    // Custom/modified version of nearest plugin
    osrmi->getOsrmNearest( mContainers[i]->x(), mContainers[i]->x(), phaNLon, phaNLat, one_way, fw_id, rv_id, fw_wt, rv_wt, street_id);

    // Get nearest fisical OSRM node (edge intersection) of phantom
    osrmi->getOsrmLocate(phaNLon, phaNLat, phyNLon, phyNLat);

    // Bearing calculation
    Node phyNode = Node(phyNLon,phyNLat);
    Node phaNode = Node(phaNLon,phaNLat);
    bool ret = mContainers[i]->isRightToSegment(phyNode, phaNode);
=======
  int pncount;
  pncount = 0;
  std::vector< Twnode > nodes;
  // UID is user ID, double is the bearing
  std::map<UID,double> bearings;
  ss.precision(6);
  ss << std::fixed << "ID\tX\tY" << std::endl;
  // True on first error
  bool errorBearing = false;
  // Node internal Id
  int nid = 0;

  // Backup OSRM state
  oldStateOsrm = osrmi->getUse();
  // forcing osrm usage
  osrmi->useOsrm(true);
  // Delete data
  osrmi->clear();

  // Containers
  for (UINT i = 0; i < mContainersCount; i++) {
    one_way = 100;
    // Custom/modified version of nearest plugin
    osrmi->getOsrmNearest( mContainers[i].x, mContainers[i].y, phaNLon, phaNLat, one_way, fw_id, rv_id, fw_wt, rv_wt, street_id);
    // If forward_weight or reverse_weight phantom node is on physical node
    // if ( (one_way != 1) && (fw_wt == 0 || rv_wt == 0) ) {
    if (fw_wt == 0 || rv_wt == 0) {
      errorBearing = true;
      ss << mContainers[i].id << "\t"
        << mContainers[i].x << "\t"
        << mContainers[i].y << std::endl;
      continue;
    }
    // Get nearest fisical OSRM node (edge intersection) of phantom
    osrmi->getOsrmLocate(phaNLon, phaNLat, phyNLon, phyNLat);
    // Bearing calculation
    Node contNode = Node(mContainers[i].x, mContainers[i].y);
    Node phyNode = Node(phyNLon,phyNLat);
    Node phaNode = Node(phaNLon,phaNLat);
    // Is right
    bool ret = contNode.isRightToSegment(phyNode, phaNode);
>>>>>>> 32b96d51e1a57ea2be7509e8c7f2af0508221315
    double bearing;
    if (ret) {
      bearing = phyNode.bearing(phaNode, false);
    } else {
      bearing = phyNode.bearing(phaNode, true);
    }

<<<<<<< HEAD
    //typnode=pickup,id,x,y
    twn = Twnode(mContainers[i]->nid(), mContainers[i]->id(), mContainers[i]->x(), mContainers[i]->x() );
    BearingNodeInfo_t nodeinfo;
    nodeinfo->forward_node_id = fw_id;
    nodeinfo->reverse_node_id = rv_id,;
    nodeinfo->bearing = bearing;
    BearingNodes[ mContainers[i].nid() ] = nodeinfo;

    nodesOnPath.push_back(twn);
    if(fw_id==0||rv_id==0)
    {
      errorBearing = true;
      ss.precision(6);
      ss << "NID" << mContainers[i].nid() << "\tFWID" << fw_id <<"\tRVID" << rv_id << "\t" << bearing;
    }
  }

  // the same loop for otherlocs
  for (UINT i = 0; i < mOtherLocsCount; i++) {
    // Only picukp has phantom nodes (OBSOLETE!)
    // if ( mOtherLocs[i]->isPickup() )
    one_way = 100;

    // Custom/modified version of nearest plugin
    osrmi->getOsrmNearest( mOtherLocs[i]->x(), mOtherLocs[i]->y(), phaNLon, phaNLat, one_way, fw_id, rv_id, fw_wt, rv_wt, street_id);

    // Get nearest fisical OSRM node (edge intersection) of phantom
    osrmi->getOsrmLocate(phaNLon, phaNLat, phyNLon, phyNLat);

    // Bearing calculation
    Node phyNode = Node(phyNLon,phyNLat);
    Node phaNode = Node(phaNLon,phaNLat);
    bool ret = mOtherLocs[i]->isRightToSegment(phyNode, phaNode);
=======
    Twnode twNode = Twnode(nid, mContainers[i].id, mContainers[i].x, mContainers[i].y);
    nodes.push_back(twNode);
    bearings[nid] = bearing;
    nid++;
  }

  // Otherlocs
  for (UINT i = 0; i < mOtherLocsCount; i++) {
    one_way = 100;
    // Custom/modified version of nearest plugin
    osrmi->getOsrmNearest( mOtherLocs[i].x, mOtherLocs[i].y, phaNLon, phaNLat, one_way, fw_id, rv_id, fw_wt, rv_wt, street_id);
    // If forward_weight or reverse_weight phantom node is on physical node
    // if ( (one_way != 1) && (fw_wt == 0 || rv_wt == 0) ) {
    if (fw_wt == 0 || rv_wt == 0) {
      errorBearing = true;
      ss << mOtherLocs[i].id << "\t"
        << mOtherLocs[i].x << "\t"
        << mOtherLocs[i].y << std::endl;
      continue;
    }
    // Get nearest fisical OSRM node (edge intersection) of phantom
    osrmi->getOsrmLocate(phaNLon, phaNLat, phyNLon, phyNLat);
    // Bearing calculation
    Node contNode = Node(mOtherLocs[i].x, mOtherLocs[i].y);
    Node phyNode = Node(phyNLon,phyNLat);
    Node phaNode = Node(phaNLon,phaNLat);
    // Is right
    bool ret = contNode.isRightToSegment(phyNode, phaNode);
>>>>>>> 32b96d51e1a57ea2be7509e8c7f2af0508221315
    double bearing;
    if (ret) {
      bearing = phyNode.bearing(phaNode, false);
    } else {
      bearing = phyNode.bearing(phaNode, true);
    }
<<<<<<< HEAD
    //typnode=pickup,id,x,y
    twn = Twnode(1, mOtherLocs[i]->id, mOtherLocs[i]->x(), mOtherLocs[i]->y() );
    //typnode=pickup,id,x,y
    twn = Twnode(mOtherLocs[i]->nid(), mOtherLocs[i]->id(), mOtherLocs[i]->x(), mOtherLocs[i]->x() );
    BearingNodeInfo_t nodeinfo;
    nodeinfo->forward_node_id = fw_id;
    nodeinfo->reverse_node_id = rv_id;
    nodeinfo->bearing = bearing;
    BearingNodes[ mOtherLocs[i].nid() ] = nodeinfo;

    nodesOnPath.push_back(twn);
    if(fw_id==0||rv_id==0)
    {
      errorBearing = true;
      ss.precision(6);
      ss << "NID" << mOtherLocs[i].nid() << "\tFWID" << fw_id <<"\tRVID" << rv_id << "\t" << bearing;
    }
  }
    if(errorBearing)
    {
      std::string s = ss.str();
    }
    //get all the times using osrm
    bool oldStateOsrm = osrmi->getUse();
    osrmi->useOsrm(true);  //forcing osrm usage
    osrmi->clear();

    // To build the call
    std::vector< Twnode > call;
    std::vector< double > bearings;

    // Add nodes and bearings to data containers
    for (unsigned int i = 0; i < nodesOnPath.size(); ++i) {
        double bearingi;
        auto it = BearingNodes.find( nodesOnPath[i]->nid );
        if ( it!=BearingNodes.end() ) {
          bearingi = it->second.bearing();
          bearings.push_back(bearingi);
          call.push_back(nodesOnPath[i]);
          for (unsigned int j = 0; j < nodesOnPath.size(); ++j) {
            if(i!=j)
            {
              double bearingj;
              auto it = BearingNodes.find( nodesOnPath[j]->nid );
              if ( it!=BearingNodes.end() ) {
                bearingj = it->second.bearing();
                bearings.push_back(bearingj);
                call.push_back(nodesOnPath[j]);

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
                double time; //usar time , tiempo total
                if (!osrmi->getOsrmTime( nodesOnPath[i].x(), nodesOnPath[i].y() , nodesOnPath[j].x(),
                                              nodesOnPath[j].y(),time)){
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
                else
                 std::out << call[i].id() << "\t" << call[j].id() << "\t" << bearings << "\t"<< bearingsj << "\t" << time << "\t";
              }
              else {
            #ifdef VRPMINTRACE
                DLOG(INFO) << "Error: Node " << nodesOnPath[i].nid() << "(" << nodesOnPath[i].id() << ") have no bearing!";
            #endif
              }
            }
          }
        } else {
        #ifdef VRPMINTRACE
            DLOG(INFO) << "Error: Node " << nodesOnPath[i].nid() << "(" << nodesOnPath[i].id() << ") have no bearing!";
        #endif
        }
    }
  }
=======

    Twnode twNode = Twnode(nid,mOtherLocs[i].id, mOtherLocs[i].x, mOtherLocs[i].y);
    nodes.push_back(twNode);
    bearings[nid] = bearing;
    nid++;
  }

  if (errorBearing) {
    osrmi->useOsrm(oldStateOsrm);
    errors = ss.str();
    return false;
  }

  std::stringstream tt;
  tt.precision(2);
  tt << std::fixed << "#FROMNODE TONODE OSRMTIME" << std::endl;
  double osrmTime;
  int nodesSize = nodes.size();
  for (UINT i = 0; i < nodesSize; i++) {
    int from = nodes[i].nid();
    for (UINT j = 0; j < nodesSize; j++) {
      int to = nodes[j].nid();
      if ( from == to ){
        continue;
      }
      osrmi->getOsrmTime(
        nodes[i].y(),
        nodes[i].x(),
        bearings[from],
        nodes[j].y(),
        nodes[j].x(),
        bearings[to],
        osrmTime
      );
      tt << nodes[i].id() << " " << nodes[j].id() << " " << osrmTime << std::endl;
    }
  }
  osrmi->useOsrm(oldStateOsrm);
  data = tt.str();
  return true;
>>>>>>> 32b96d51e1a57ea2be7509e8c7f2af0508221315
}
