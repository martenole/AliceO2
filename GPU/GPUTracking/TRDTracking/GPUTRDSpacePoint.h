// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file GPUTRDSpacePoint.h
/// \author Ole Schmidt, ole.schmidt@cern.ch
/// \brief Struct to hold the position/direction information of the tracklets transformed in sector coordinates

#ifndef GPUTRDSPACEPOINT_H
#define GPUTRDSPACEPOINT_H

namespace GPUCA_NAMESPACE
{
namespace gpu
{

// struct to hold the information on the space points
struct GPUTRDSpacePoint {
  float mR;                 // x position (3.5 mm above anode wires) - radial offset due to t0 mis-calibration, measured -1 mm for run 245353
  float mX[2];              // y and z position (sector coordinates)
  float mDy;                // deflection over drift length
  unsigned short mVolumeId; // basically derived from TRD chamber number
  GPUd() GPUTRDSpacePoint(float x = 0, float y = 0, float z = 0, float dy = 0) : mR(x), mDy(dy), mVolumeId(0)
  {
    mX[0] = y;
    mX[1] = z;
  }
};

} // namespace gpu
} // namespace GPUCA_NAMESPACE

#endif
