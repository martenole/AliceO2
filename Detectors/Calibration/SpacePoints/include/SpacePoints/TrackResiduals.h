// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file TrackResiduals.h
/// \brief Definition of the TrackResiduals class
///
/// \author Ole Schmidt, ole.schmidt@cern.ch

#ifndef ALICEO2_CALIB_TRACKRESIDUALS_H_
#define ALICEO2_CALIB_TRACKRESIDUALS_H_

#include <memory>
#include <vector>
#include <bitset>
#include <Rtypes.h>

#include "SpacePoints/Param.h"

#include "TTree.h"
#include "TFile.h"

namespace AliTPCDcalibRes
{
struct dts_t {                            // struct for basic local residual
  Double32_t dy;                          //[-20.,20.,15] // [-kMaxResid,kMaxResid,14]
  Double32_t dz;                          //[-20.,20.,15] // [-kMaxResid,kMaxResid,14]
  Double32_t tgSlp;                       //[-2,2,14]  //[kMaxTgSlp,kMaxTgSlp,14]
  UChar_t bvox[o2::calib::param::VoxDim]; // voxel bin info: VoxF,kVoxX,kVoxZ
  //
  dts_t() { memset(this, 0, sizeof(dts_t)); }
};
} // namespace AliTPCDcalibRes

namespace o2
{
namespace calib
{

/// \class TrackResiduals
/// \brief Class steering the space point calibration of the TPC from track residuals
///
class TrackResiduals
{
 public:
  /// Default constructor
  TrackResiduals();
  /// Destructor
  ~TrackResiduals() = default;

  /// structure to hold the results
  struct bres_t {
    float D[param::ResDim];            // values of extracted distortions
    float E[param::ResDim];            // their errors
    float DS[param::ResDim];           // smoothed residual
    float DC[param::ResDim];           // Cheb parameterized residual
    float EXYCorr{ 0.f };              // correlation between extracted X and Y
    float dYSigMAD{ 0.f };             // MAD estimator of dY sigma (dispersion after slope removal)
    float dZSigLTM{ 0.f };             // Z sigma from unbinned LTM estimator
    float stat[param::VoxHDim];        // statistics: averages of each voxel dimension + entries
    unsigned char bvox[param::VoxDim]; // voxel identifier, here the bvox[0] shows number of Q bins used for Y
    unsigned char bsec{ 0 };           // sector ID (0-35)
    unsigned char flags{ 0 };          // status flag

    bres_t() : D{}, E{}, DS{}, DC{}, stat{}, bvox{} {}
  };

  // initialization
  void init();
  void initBinning();
  void initResultsContainer(int iSec);
  void reset();

  // settings
  void setPrintMemoryUsage() { mPrintMem = true; }
  void setKernelType(int type = param::EpanechnikovKernel, float bwX = 2.1f, float bwp = 2.1f, float bwZ = 1.7f, float scX = 1.f, float scP = 1.f, float scZ = 1.f);

  // steering functions
  void processResiduals();
  void processSectorResiduals(Int_t iSec);
  void processVoxelResiduals(std::vector<float>& dy, std::vector<float>& dz, std::vector<float>& tg, bres_t& resVox);
  void processVoxelDispersions(std::vector<float>& tg, std::vector<float>& dy, bres_t& resVox);
  int validateVoxels(int iSec);
  void smooth(int iSec);

  // statistics
  float fitPoly1Robust(std::vector<float>& x, std::vector<float>& y, float* res, float* err, float cutLTM);
  float getMAD2Sigma(const std::vector<float> data);
  void medFit(int nPoints, int offset, const std::vector<float>& x, const std::vector<float>& y, float& a, float& b, float* err = nullptr, float delI = 0.f);
  float roFunc(int nPoints, int offset, const std::vector<float>& x, const std::vector<float>& y, float b, float& aa);
  float selectKthMin(const int k, std::vector<float>& data);
  bool getSmoothEstimate(int iSec, float x, float p, float z, int whichDim = 0, float* res = nullptr);
  double getKernelWeight(std::array<double, 3> u2vec, int kernelType) const;

  // binning / geometry
  unsigned short getGlbVoxBin(int ix, int ip, int iz) const;
  unsigned short getGlbVoxBin(unsigned char* bvox) const;
  void getVoxelCoordinates(int isec, int ix, int ip, int iz, float& x, float& p, float& z) const;
  float getX(int i) const;
  float getY2X(int ix, int ip) const;
  float getZ(int i) const;
  bool getXBinIgnored(int iSec, int bin) const { return mXBinsIgnore[iSec].test(bin); }
  void findVoxel(float x, float y2x, float z2x, int& ix, int& ip, int& iz) const;
  int getXBin(float x) const;
  int getY2XBin(float y2x, int ix) const;
  int getZ2XBin(float z2x) const;
  float getDXI(int ix) const { return mUniformBins[param::VoxX] ? mDXI : 1.f / param::RowDX[ix]; }
  float getDY2XI(int ix) const { return mDY2XI[ix]; }
  float getDZ2XI() const { return mDZI; }

  // general helper functions
  void printMem() const;
  void dumpToFile(const std::vector<float>& vec, const std::string fName) const;
  void dumpResults(int iSec);
  void createOutputFile();
  void closeOutputFile();

 private:
  std::unique_ptr<TFile> mFileOut;                   ///< output debug file
  std::unique_ptr<TTree> mTreeOut;                   ///< tree holding debug output
  bool mIsInitialized{ false };                      ///< initialize only once
  bool mUniformBins[param::VoxDim];                  ///< if binning is uniform for each dimension
  bool mPrintMem{ false };                           ///< turn on to print memory usage at certain points
  bool mUseErrInSmoothing{ true };                   ///< weight kernel by point error
  bool mSmoothPol2[param::VoxDim];                   ///< option to use pol1 or pol2 in each direction
  float mDX{ 0.f };                                  ///< x bin size
  float mDXI{ 0.f };                                 ///< inverse of x bin size
  int mNXBins{ -1 };                                 ///< number of bins in radial direction
  int mNY2XBins;                                     ///< number of y/x bins per sector
  int mNZBins{ 0 };                                  ///< number of z/x bins per sector
  float mDZI{ 0.f };                                 ///< inverse of bin size in z (is 5!?)
  float mDZ{ 0.f };                                  ///< bin size in z (is 0.2!?)
  int mNSmoothingFailedBins[param::NSectors2];       ///< number of failed bins / sector
  int mKernelType{ -1 };                             ///< kernel type (Epanechnikov / Gaussian)
  std::array<float, param::VoxDim> mKernelScaleEdge; ///< optional scaling factors for kernel width on the edge
  std::array<float, param::VoxDim> mKernelWInv;      ///< inverse kernel width in bins
  std::array<int, param::VoxDim> mStepKern;          ///< N bins to consider with given kernel settings
  int mNVoxPerSector{ 0 };                           ///< number of voxels per sector

  std::array<int, param::VoxDim> mNBins;                                   ///< number of bins for all dimensions
  std::array<std::bitset<param::NPadRows>, param::NSectors2> mXBinsIgnore; ///< flags which X bins to ignore
  std::array<double, param::ResDim * param::MaxSmtDim> mLastSmoothingRes;  ///< results of last smoothing operation

  std::vector<float> mMaxY2X; ///< max y/x at each x bin, accounting dead zones
  std::vector<float> mDY2XI;  ///< inverse y/x bin size at given x bin
  std::vector<float> mDY2X;   ///< y/x bin size at given x bin

  std::array<std::array<float, param::NPadRows>, param::NSectors2> mValidFracXBins; ///< for each sector for each X-bin the fraction of validated voxels

  std::vector<std::vector<bres_t>> mVoxelResults; ///< results per sector and per voxel for 3-D distortions
};

//_____________________________________________________
inline unsigned short TrackResiduals::getGlbVoxBin(unsigned char* bvox) const
{
  return bvox[param::VoxX] + bvox[param::VoxF] * mNXBins + bvox[param::VoxZ] * mNXBins * mNY2XBins;
}

//_____________________________________________________
inline unsigned short TrackResiduals::getGlbVoxBin(int ix, int ip, int iz) const
{
  unsigned char bvox[param::VoxDim];
  bvox[param::VoxX] = ix;
  bvox[param::VoxF] = ip;
  bvox[param::VoxZ] = iz;
  return getGlbVoxBin(bvox);
}

//_____________________________________________________
inline void TrackResiduals::getVoxelCoordinates(int isec, int ix, int ip, int iz, float& x, float& p, float& z) const
{
  x = getX(ix);
  p = getY2X(ix, ip);
  z = getZ(iz);
  if (isec >= param::NSectors) {
    z = -z;
  }
}

//_____________________________________________________
inline float TrackResiduals::getX(int i) const
{
  return mUniformBins[param::VoxX] ? param::MinX + (i + 0.5) * mDX : param::RowX[i];
}

//_____________________________________________________
inline float TrackResiduals::getY2X(int ix, int ip) const
{
  return (0.5f + ip) * mDY2X[ix] - mMaxY2X[ix];
}

//_____________________________________________________
inline float TrackResiduals::getZ(int i) const
{
  // always positive
  return (0.5f + i) * mDZ;
}

//_____________________________________________________
inline void TrackResiduals::findVoxel(float x, float y2x, float z2x, int& ix, int& ip, int& iz) const
{
  ix = getXBin(x);
  ip = getY2XBin(y2x, ix);
  iz = getZ2XBin(z2x);
}

//_____________________________________________________
inline int TrackResiduals::getY2XBin(float y2x, int ix) const
{
  int bp = (y2x + mMaxY2X[ix]) * mDY2XI[ix];
  if (bp < 0) {
    bp = 0;
  }
  return (bp < mNY2XBins) ? bp : mNY2XBins - 1;
}

//_____________________________________________________
inline int TrackResiduals::getZ2XBin(float z2x) const
{
  int bz = z2x * mDZI;
  if (bz < 0) {
    // accounting for clusters which were moved to the wrong side
    bz = 0;
  }
  return (bz < mNZBins) ? bz : mNZBins - 1;
}

} // namespace calib

} // namespace o2
#endif
