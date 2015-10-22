#include "gdali.h"

bool GdalI::openVectorDatasource(std::string filePath)
{

  mReadDataset = (GDALDataset*) GDALOpenEx( filePath.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( mReadDataset == NULL ) {
    return false;
  }
  mReadFilePath = filePath;
  return true;

}

void GdalI::closeVectorDatasource()
{
  if( mReadDataset != NULL ) {
    GDALClose( mReadDataset );
  }
}

bool GdalI::hasDContainersFields()
{
  int nLayers = mReadDataset->GetLayerCount();
  if ( nLayers>0 ) {
    mReadOGRLayer = mReadDataset->GetLayer(0);
    OGRFeatureDefn *featDef = mReadOGRLayer->GetLayerDefn();
    for (int i=0; i<featDef->GetFieldCount(); i++) {
      OGRFieldDefn *fieldDef = featDef->GetFieldDefn(i);
      std::string fName = fieldDef->GetNameRef();
    }
  } else {
    return false;
  }
}

bool GdalI::hasDOtherlocsFields()
{
  return false;
}

bool GdalI::hasDVehiclesFields()
{
  return false;
}

bool GdalI::hasDTimeMatrixFields()
{
  return false;
}


