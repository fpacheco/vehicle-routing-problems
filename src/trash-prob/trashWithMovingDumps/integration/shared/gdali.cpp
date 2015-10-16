#include "gdali.h"

bool GdalI::openOGRDataSource(std::string filePath)
{
  mOGROpenDS = OGRSFDriverRegistrar::Open( filePath.c_str(), FALSE );
  if( mOGROpenDS == NULL ) {
    return false;
  }
  mFilePath = filePath;
  return true;
}

void GdalI::closeOGRDataSource()
{
  if( mOGROpenDS != NULL ) {
    OGRDataSource::DestroyDataSource( mOGROpenDS );
  }
}

