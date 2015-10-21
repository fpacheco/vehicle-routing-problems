#ifndef GDALI_H
#define GDALI_H

#include <string>
#include <vector>
#include "ogrsf_frmts.h"


class GdalI {

public:
  GdalI() {
     GDALAllRegister();
  }
  // ~GdalI() { }
  bool openVectorDatasource(std::string filePath);
  void closeVectorDatasource();
  void writeSeqOutput();
  bool hasDContainersFields();
  bool hasDOtherlocsFields();
  bool hasDVehiclesFields();
  bool hasDTimeMatrixFields();


private:
  // GDAL datasource
  GDALDataset *mReadDataset;
  // OGR file to read
  std::string mReadFilePath;
  // Layer
  OGRLayer *mReadOGRLayer;

  // GDAL datasource
  GDALDataset *mWriteDataset;
  // OGR file to read
  std::string mWriteFilePath;
  // Layer
  OGRLayer *mWriteOGRLayer;

};

#endif
