// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file TrackInterpolation.h
/// \brief Definition of the TrackInterpolation class
///
/// \author Ole Schmidt, ole.schmidt@cern.ch

#ifndef ALICEO2_TPC_TRACKINTERPOLATION_H_
#define ALICEO2_TPC_TRACKINTERPOLATION_H_

#include <gsl/span>
#include "CommonDataFormat/EvIndex.h"
#include "CommonDataFormat/RangeReference.h"
#include "ReconstructionDataFormats/Track.h"
#include "ReconstructionDataFormats/TrackTPCITS.h"
#include "ReconstructionDataFormats/MatchInfoTOF.h"
#include "ReconstructionDataFormats/GlobalTrackID.h"
#include "DataFormatsITSMFT/Cluster.h"
#include "DataFormatsITS/TrackITS.h"
#include "DataFormatsTPC/TrackTPC.h"
#include "DataFormatsTPC/Constants.h"
#include "DataFormatsTOF/Cluster.h"
#include "DataFormatsTRD/TrackTRD.h"
#include "DataFormatsTRD/Tracklet64.h"
#include "DataFormatsTRD/CalibratedTracklet.h"
#include "SpacePoints/SpacePointsCalibParam.h"
#include "TPCReconstruction/TPCFastTransformHelperO2.h"
#include "DetectorsBase/Propagator.h"
#include "DataFormatsGlobalTracking/RecoContainer.h"
#include "TRDBase/RecoParam.h"
#include "TRDBase/Geometry.h"

class TTree;

namespace o2
{

namespace tpc
{

/// Structure used to store the TPC cluster residuals
struct TPCClusterResiduals {
  short dy{};           ///< residual in Y
  short dz{};           ///< residual in Z
  short y{};            ///< Y position of track
  short z{};            ///< Z position of track
  short phi{};          ///< phi angle of track
  short tgl{};          ///< dip angle of track
  unsigned char sec{};  ///< sector number 0..17
  unsigned char dRow{}; ///< distance to previous row in units of pad rows
  short row{};          ///< TPC pad row (absolute units)
  void setDY(float val) { dy = fabs(val) < param::MaxResid ? static_cast<short>(val * 0x7fff / param::MaxResid) : static_cast<short>(std::copysign(1., val) * 0x7fff); }
  void setDZ(float val) { dz = fabs(val) < param::MaxResid ? static_cast<short>(val * 0x7fff / param::MaxResid) : static_cast<short>(std::copysign(1., val) * 0x7fff); }
  void setY(float val) { y = fabs(val) < param::MaxY ? static_cast<short>(val * 0x7fff / param::MaxY) : static_cast<short>(std::copysign(1., val) * 0x7fff); }
  void setZ(float val) { z = fabs(val) < param::MaxZ ? static_cast<short>(val * 0x7fff / param::MaxZ) : static_cast<short>(std::copysign(1., val) * 0x7fff); }
  void setPhi(float val) { phi = fabs(val) < param::MaxTgSlp ? static_cast<short>(val * 0x7fff / param::MaxTgSlp) : static_cast<short>(std::copysign(1., val) * 0x7fff); }
  void setTgl(float val) { tgl = fabs(val) < param::MaxTgSlp ? static_cast<short>(val * 0x7fff / param::MaxTgSlp) : static_cast<short>(std::copysign(1., val) * 0x7fff); }
  ClassDefNV(TPCClusterResiduals, 1);
};

/// Structure filled for each track with track quality information and a vector with TPCClusterResiduals
struct TrackData {
  int trkId{};                 ///< track ID for debugging
  float eta{};                 ///< track dip angle
  float phi{};                 ///< track azimuthal angle
  float qPt{};                 ///< track q/pT
  float chi2TPC{};             ///< chi2 of TPC track
  float chi2ITS{};             ///< chi2 of ITS track
  unsigned short nClsTPC{};    ///< number of attached TPC clusters
  unsigned short nClsITS{};    ///< number of attached ITS clusters
  unsigned short nTrkltsTRD{}; ///< number of attached TRD tracklets
  // TODO: Add an additional structure with event information and reference to the tracks for given event?
  o2::dataformats::RangeReference<> clIdx{}; ///< index of first cluster residual and total number of cluster residuals of this track
  ClassDefNV(TrackData, 1);
};

/// \class TrackInterpolation
/// This class is retrieving the TPC space point residuals by interpolating ITS/TRD/TOF tracks.
/// The residuals are stored in the specified vectors of TPCClusterResiduals
/// It has been ported from the AliTPCcalibAlignInterpolation class from AliRoot.
class TrackInterpolation
{
 public:
  using MatCorrType = o2::base::Propagator::MatCorrType;

  /// Default constructor
  TrackInterpolation() = default;

  /// Enumeration for indexing the arrays of the CacheStruct
  enum {
    ExtOut = 0, ///< extrapolation outwards of ITS track
    ExtIn,      ///< extrapolation inwards of TRD/TOF track
    Int,        ///< interpolation (mean positions of both extrapolations)
    NIndices    ///< total number of indices (3)
  };

  /// Structure for caching positions, covariances and angles for extrapolations from ITS and TRD/TOF and for interpolation
  struct CacheStruct {
    std::array<float, NIndices> y{};
    std::array<float, NIndices> z{};
    std::array<float, NIndices> sy2{};
    std::array<float, NIndices> szy{};
    std::array<float, NIndices> sz2{};
    std::array<float, NIndices> phi{};
    std::array<float, NIndices> tgl{};
    float clY{0.f};
    float clZ{0.f};
    float clAngle{0.f};
    unsigned short clAvailable{0};
  };

  // -------------------------------------- processing functions --------------------------------------------------

  /// Initialize everything
  void init();

  /// Main processing function
  void process(const o2::globaltracking::RecoContainer& inp, const std::vector<o2::dataformats::GlobalTrackID>& gids, const std::vector<o2::globaltracking::RecoContainer::GlobalIDSet>& gidTables, std::vector<o2::track::TrackParCov>& seeds, const std::vector<float>& trkTimes);

  /// Extrapolate ITS-only track through TPC and store residuals to TPC clusters along the way
  /// \param seed index
  void extrapolateTrack(int iSeed);

  /// Interpolate ITS-TRD-TOF track inside TPC and store residuals to TPC clusters along the way
  /// \param seed index
  void interpolateTrack(int iSeed);

  /// Reset cache and output vectors
  void reset();

  // -------------------------------------- settings --------------------------------------------------

  /// Sets the maximum phi angle at which track extrapolation is aborted
  void setMaxSnp(float snp) { mMaxSnp = snp; }
  /// Sets the maximum step length for track extrapolation
  void setMaxStep(float step) { mMaxStep = step; }
  /// Sets the flag if material correction should be applied when extrapolating the tracks
  void setMatCorr(MatCorrType matCorr) { mMatCorr = matCorr; }

  // --------------------------------- output ---------------------------------------------
  std::vector<TPCClusterResiduals>& getClusterResiduals() { return mClRes; }
  std::vector<TrackData>& getReferenceTracks() { return mTrackData; }

 private:
  // parameters + settings
  float mTPCTimeBinMUS{.2f}; ///< TPC time bin duration in us
  float mSigYZ2TOF{.75f};    ///< for now assume cluster error for TOF equal for all clusters in both Y and Z
  float mMaxSnp{.85f};          ///< max snp when propagating ITS tracks
  float mMaxStep{2.f};          ///< maximum step for propagation
  MatCorrType mMatCorr{MatCorrType::USEMatCorrNONE}; ///< if material correction should be done
  o2::trd::RecoParam mRecoParam;                     ///< parameters required for TRD refit
  o2::trd::Geometry* mGeoTRD;                        ///< TRD geometry instance (needed for tilted pad correction)

  // input
  const o2::globaltracking::RecoContainer* mRecoCont = nullptr;                            ///< input reco container
  const std::vector<o2::dataformats::GlobalTrackID>* mGIDs = nullptr;                      ///< GIDs of input tracks
  const std::vector<o2::globaltracking::RecoContainer::GlobalIDSet>* mGIDtables = nullptr; ///< GIDs of contributors from single detectors for each seed
  std::vector<o2::track::TrackParCov>* mSeeds = nullptr;                                   ///< seeding track parameters (ITS tracks)
  gsl::span<const TPCClRefElem> mTPCTracksClusIdx;                                         ///< input TPC cluster indices from span
  const ClusterNativeAccess* mTPCClusterIdxStruct = nullptr; ///< struct holding the TPC cluster indices

  // output
  std::vector<TrackData> mTrackData{};                  ///< this vector is used to store the track quality information on a per track basis
  std::vector<TPCClusterResiduals> mClRes{};            ///< residuals for each available TPC cluster of all tracks

  // cache
  std::array<CacheStruct, constants::MAXGLOBALPADROW> mCache{{}}; ///< caching positions, covariances and angles for track extrapolations and interpolation
  std::vector<o2::dataformats::GlobalTrackID> mGIDsSuccess;       ///< keep track of the GIDs which could be processed successfully

  // helpers
  std::unique_ptr<TPCFastTransform> mFastTransform{}; ///< TPC cluster transformation
  bool mInitDone{false};                              ///< initialization done flag
};

} // namespace tpc

} // namespace o2

#endif
