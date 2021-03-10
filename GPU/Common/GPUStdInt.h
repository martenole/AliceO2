// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file GPUStdInt.h
/// \author Ole Schmidt

#ifndef GPUSTDINT_H
#define GPUSTDINT_H

#include "GPUCommonDef.h"

#if !defined(GPUCA_GPUCODE_DEVICE)
#include <cstdint>
#endif


#if defined(__OPENCL__)
namespace std
{
  typedef unsigned char uint8_t;
  typedef unsigned short uint16_t;
  typedef unsigned int uint32_t;
  typedef unsigned long uint64_t;

  typedef char int8_t;
  typedef short int16_t;
  typedef int int32_t;
  typedef long int64_t;
}
#endif // defined(__OPENCL__)



#endif // GPUSTDINT_H
