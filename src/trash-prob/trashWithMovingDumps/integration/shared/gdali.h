#ifndef GDALI_H
#define GDALI_H

#include <string>
// OGR headers
#include "ogrsf_frmts.h"

class GdalI {

public:
  GdalI() {
     OGRRegisterAll();
  }

  // ~GdalI() { }

  bool openOGRDataSource(std::string filePath);
  void closeOGRDataSource();

private:
  // OGS datasource
  OGRDataSource *mOGROpenDS;
  // OGR file to read
  std::string mFilePath;
  // GDAL datasource
  // GDALDataset *mPoDataset;
};

#endif
