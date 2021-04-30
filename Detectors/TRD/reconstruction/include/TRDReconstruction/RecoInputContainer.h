// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file   RecoInputContainer.h
/// \author ole.schmidt@cern.ch
/// \brief Struct for input data required by TRD tracking workflow

#ifndef O2_TRD_RECOINPUTCONTAINER_H
#define O2_TRD_RECOINPUTCONTAINER_H

#include "ReconstructionDataFormats/TrackTPCITS.h"
#include "DataFormatsTPC/TrackTPC.h"
#include "DataFormatsTRD/Tracklet64.h"
#include "DataFormatsTRD/CalibratedTracklet.h"
#include "DataFormatsTRD/TriggerRecord.h"

#include "GPUDataTypes.h"

#include <gsl/span>
#include <memory>

namespace o2
{
namespace trd
{

struct RecoInputContainer {

  gsl::span<const o2::dataformats::TrackTPCITS> mTracksTPCITS;
  gsl::span<const o2::tpc::TrackTPC> mTracksTPC;
  gsl::span<const o2::trd::Tracklet64> mTracklets;
  gsl::span<const o2::trd::CalibratedTracklet> mSpacePoints;
  gsl::span<const o2::trd::TriggerRecord> mTriggerRecords;
};

auto getRecoInputContainer(o2::gpu::GPUTrackingInOutPointers& ptrs)
{
  std::unique_ptr<RecoInputContainer> retVal = std::make_unique<RecoInputContainer>();
  return std::move(retVal);
}

} // namespace trd
} // namespace o2

#endif
