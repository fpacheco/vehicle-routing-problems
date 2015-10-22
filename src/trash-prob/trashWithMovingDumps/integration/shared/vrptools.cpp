#include <stdlib.h>     // malloc, calloc, realloc, free
#include <cmath>        // std::abs

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

  // Variables
  bool oldStateOsrm;
  // PhantomNode
  double phaNLon, phaNLat;
  // Physical Node
  double phyNLon, phyNLat;
  unsigned int one_way;
  unsigned int fw_id, rv_id, fw_wt, rv_wt, street_id;
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

    // Nodes
    Node contNode = Node(mContainers[i].x, mContainers[i].y);
    Node phaNode = Node(phaNLon,phaNLat);
    double distance = contNode.haversineDistance(phaNode);

    double dlon, dlat;
    dlon = std::abs(mContainers[i].x-phaNLon);
    dlat = std::abs(mContainers[i].y-phaNLat);

    DLOG(INFO) << mContainers[i].id << "\t" << distance;

    if ( (dlon<0.000001 || dlat<0.000001) && (distance < 5) ) {
      errorBearing = true;
      ss << mContainers[i].id << "\t"
        << mContainers[i].x << "\t"
        << mContainers[i].y << std::endl;
      continue;
    }

    /*
    double dlon, dlat;
    dlon = std::abs(mOtherLocs[i].x-phaNLon);
    dlat = std::abs(mOtherLocs[i].y-phaNLat);
    //if (fw_wt == 0 || rv_wt == 0) {
    if ( dlon<0.000001 || dlat<0.000001) {
      errorBearing = true;
      ss << mContainers[i].id << "\t"
        << mContainers[i].x << "\t"
        << mContainers[i].y << std::endl;
      continue;
    }
    */

    //bearing FROM container TO phantomnode
    double compBearing;
    double bearing;
    compBearing = contNode.bearing(phaNode, false);
    bearing = compBearing + 90;
    if (bearing>=360) {
      bearing-=360;
    }

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

    Node contNode = Node(mOtherLocs[i].x, mOtherLocs[i].y);
    Node phaNode = Node(phaNLon,phaNLat);
    double distance = contNode.haversineDistance(phaNode);

    DLOG(INFO) << mOtherLocs[i].id << "\t" << distance;

    double dlon, dlat;
    dlon = std::abs(mOtherLocs[i].x-phaNLon);
    dlat = std::abs(mOtherLocs[i].y-phaNLat);

    if ( (dlon<0.000001 || dlat<0.000001) && (distance < 5) ) {
      errorBearing = true;
      ss << mOtherLocs[i].id << "\t"
        << mOtherLocs[i].x << "\t"
        << mOtherLocs[i].y << std::endl;
      continue;
    }

    /*
    double dlon, dlat;
    dlon = std::abs(mOtherLocs[i].x-phaNLon);
    dlat = std::abs(mOtherLocs[i].y-phaNLat);
    //if (fw_wt == 0 || rv_wt == 0) {
    if ( dlon<0.000001 && dlat<0.000001) {
      errorBearing = true;
      ss << mOtherLocs[i].id << "\t"
        << mOtherLocs[i].x << "\t"
        << mOtherLocs[i].y << std::endl;
      continue;
    }
    */

    //bearing FROM container TO phantomnode
    double compBearing;
    double bearing;
    // Bearing calculation
    // Node contNode = Node(mOtherLocs[i].x, mOtherLocs[i].y);
    // Node phaNode = Node(phaNLon,phaNLat);
    compBearing = contNode.bearing(phaNode, false);
    bearing = compBearing + 90;
    if (bearing>=360) {
      bearing-=360;
    }

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

  bool errorTimes = false;
  // Empty ss
  ss.str("");
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
      bool res = osrmi->getOsrmTime(
        nodes[i].y(),
        nodes[i].x(),
        bearings[from],
        nodes[j].y(),
        nodes[j].x(),
        bearings[to],
        osrmTime
      );
      if (res) {
        tt << nodes[i].id() << " " << nodes[j].id() << " " << osrmTime << std::endl;
      } else {
        errorTimes=true;
        ss << "From: " << nodes[i].id() << " To:" << nodes[j].id() << " failed!" << std::endl;
      }
    }
  }

  if (errorTimes) {
    osrmi->useOsrm(oldStateOsrm);
    errors = ss.str();
    return false;
  }

  osrmi->useOsrm(oldStateOsrm);
  data = tt.str();
  return true;
}
