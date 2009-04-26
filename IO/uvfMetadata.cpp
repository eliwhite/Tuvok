/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
   University of Utah.


   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/
/**
  \file    uvfUVFMetadata.cpp
  \author  Tom Fogal
           SCI Institute
           University of Utah
  \brief   UVF-specific implementation of metadata.
*/

#include <algorithm>
#include "uvfMetadata.h"

namespace tuvok {

UVFMetadata::UVFMetadata() :
  m_pVolumeDataBlock(NULL),
  m_pMaxMinData(NULL)
{
}

UVFMetadata::UVFMetadata(RasterDataBlock* pVolumeDataBlock,
                         MaxMinDataBlock* pMaxMinData, bool bIsSameEndianness):
  m_pVolumeDataBlock(pVolumeDataBlock),
  m_pMaxMinData(pMaxMinData),
  m_bIsSameEndianness(bIsSameEndianness)
{
  std::vector<double> vfScale;
  size_t iSize = m_pVolumeDataBlock->ulDomainSize.size();

  // we require the data to be at least 3D
  assert(iSize >= 3);

  // we also assume that x,y,z are in the first 3 components and
  // we have no anisotropy (i.e. ulLODLevelCount.size=1)
  m_iLODLevel = m_pVolumeDataBlock->ulLODLevelCount[0];
  for (size_t i = 0;i<3;i++) {
    m_aOverlap[i] = m_pVolumeDataBlock->ulBrickOverlap[i];
    m_aMaxBrickSize[i] = m_pVolumeDataBlock->ulBrickSize[i];
    m_aScale[i] = m_pVolumeDataBlock->dDomainTransformation[i+(iSize+1)*i];
  }

  m_vvaBrickSize.resize(m_iLODLevel);
  if (m_pMaxMinData) m_vvaMaxMin.resize(m_iLODLevel);

  for (size_t j = 0;j<m_iLODLevel;j++) {
    std::vector<UINT64> vLOD;  vLOD.push_back(j);
    std::vector<UINT64> vDomSize = m_pVolumeDataBlock->GetLODDomainSize(vLOD);
    m_aDomainSize.push_back(UINT64VECTOR3(vDomSize[0], vDomSize[1],
                                          vDomSize[2]));

    std::vector<UINT64> vBrickCount = m_pVolumeDataBlock->GetBrickCount(vLOD);
    m_vaBrickCount.push_back(UINT64VECTOR3(vBrickCount[0], vBrickCount[1],
                                           vBrickCount[2]));

    m_vvaBrickSize[j].resize(m_vaBrickCount[j].x);
    if (m_pMaxMinData) {
      m_vvaMaxMin[j].resize(m_vaBrickCount[j].x);
    }
    for (UINT64 x=0; x < m_vaBrickCount[j].x; x++) {
      m_vvaBrickSize[j][x].resize(m_vaBrickCount[j].y);
      if (m_pMaxMinData) {
        m_vvaMaxMin[j][x].resize(m_vaBrickCount[j].y);
      }

      for (UINT64 y=0; y < m_vaBrickCount[j].y; y++) {
        if (m_pMaxMinData) {
          m_vvaMaxMin[j][x][y].resize(m_vaBrickCount[j].z);
        }

        for (UINT64 z=0; z < m_vaBrickCount[j].z; z++) {
          std::vector<UINT64> vBrick;
          vBrick.push_back(x);
          vBrick.push_back(y);
          vBrick.push_back(z);
          std::vector<UINT64> vBrickSize =
            m_pVolumeDataBlock->GetBrickSize(vLOD, vBrick);

          m_vvaBrickSize[j][x][y].push_back(UINT64VECTOR3(vBrickSize[0],
                                                          vBrickSize[1],
                                                          vBrickSize[2]));
        }
      }
    }
  }

  size_t iSerializedIndex = 0;
  if (m_pMaxMinData) {
    for (size_t lod=0; lod < m_iLODLevel; lod++) {
      for (UINT64 z=0; z < m_vaBrickCount[lod].z; z++) {
        for (UINT64 y=0; y < m_vaBrickCount[lod].y; y++) {
          for (UINT64 x=0; x < m_vaBrickCount[lod].x; x++) {
            // for four-component data we use the fourth component
            // (presumably the alpha channel); for all other data we use
            // the first component
            /// \todo we may have to change this if we add support for other
            /// kinds of multicomponent data.
            m_vvaMaxMin[lod][x][y][z] =
              m_pMaxMinData->GetValue(iSerializedIndex++,
                 (m_pVolumeDataBlock->ulElementDimensionSize[0] == 4) ? 3 : 0
              );
          }
        }
      }
    }
  }

  // Use a minimum of 3 elements, which ...
  size_t n_elems = std::max(static_cast<size_t>(3),
                            m_pVolumeDataBlock->ulDomainSemantics.size());
  std::vector<double> rescale(n_elems);
  for (size_t i=0; i < n_elems; i++) {
    rescale[i] = 1.0;
  }
  // ... allows us to assume we can deref the first 3 elements here.
  this->SetRescaleFactors(DOUBLEVECTOR3(rescale[0],rescale[1],rescale[2]));
}


// Return the number of bricks in the given LoD, along each axis.
UINT64VECTOR3 UVFMetadata::GetBrickCount(const UINT64 lod) const
{
  return m_vaBrickCount[lod];
}

// Size of the brick in logical space.
UINT64VECTOR3 UVFMetadata::GetBrickSize(const NDBrickKey &k) const
{
  const UINT64 lod = k.first[0];
  const UINT64VECTOR3 vBrick = UINT64VECTOR3(k.second[0].x,
                                             k.second[0].y,
                                             k.second[0].z);
  return m_vvaBrickSize[lod][vBrick.x][vBrick.y][vBrick.z];
}


// Gives the size of a brick in real space.
FLOATVECTOR3 UVFMetadata::GetEffectiveBrickSize(const NDBrickKey &k) const
{
  const UINT64 iLOD = k.first[0];
  const UINT64VECTOR3 vBrick = UINT64VECTOR3(k.second[0].x,
                                             k.second[0].y,
                                             k.second[0].z);
  FLOATVECTOR3 vBrickSize(m_vvaBrickSize[iLOD][vBrick.x][vBrick.y][vBrick.z].x,
                          m_vvaBrickSize[iLOD][vBrick.x][vBrick.y][vBrick.z].y,
                          m_vvaBrickSize[iLOD][vBrick.x][vBrick.y][vBrick.z].z);

  if (m_vaBrickCount[iLOD].x > 1) {
    if (vBrick.x > 0) vBrickSize.x -= m_aOverlap.x/2.0f;
    if (vBrick.x < m_vaBrickCount[iLOD].x-1) vBrickSize.x -= m_aOverlap.x/2.0f;
  }
  if (m_vaBrickCount[iLOD].y > 1) {
    if (vBrick.y < m_vaBrickCount[iLOD].y-1) vBrickSize.y -= m_aOverlap.y/2.0f;
    if (vBrick.y > 0) vBrickSize.y -= m_aOverlap.y/2.0f;
  }
  if (m_vaBrickCount[iLOD].z > 1) {
    if (vBrick.z < m_vaBrickCount[iLOD].z-1) vBrickSize.z -= m_aOverlap.z/2.0f;
    if (vBrick.z > 0) vBrickSize.z -= m_aOverlap.z/2.0f;
  }

  return vBrickSize;
}

UINT64VECTOR3 UVFMetadata::GetDomainSize(const UINT64 lod) const
{
  return m_aDomainSize[lod];
}

UINT64VECTOR3 UVFMetadata::GetMaxBrickSize() const
{
  return m_aMaxBrickSize;
}

UINT64VECTOR3 UVFMetadata::GetBrickOverlapSize() const
{
  return m_aOverlap;
}

UINT64 UVFMetadata::GetLODLevelCount() const
{
  return m_iLODLevel;
}

DOUBLEVECTOR3 UVFMetadata::GetScale() const
{
  double rescale[3];
  this->GetRescaleFactors(rescale);
  return m_aScale * DOUBLEVECTOR3(rescale[0], rescale[1], rescale[2]);
}

/// \todo change this if we want to support data where elements are of
// different size
UINT64 UVFMetadata::GetBitWidth() const
{
  return m_pVolumeDataBlock->ulElementBitSize[0][0];
}

UINT64 UVFMetadata::GetComponentCount() const
{
  return m_pVolumeDataBlock->ulElementDimensionSize[0];
}

/// \todo change this if we want to support data where elements are of
/// different type
bool UVFMetadata::GetIsSigned() const
{
  return m_pVolumeDataBlock->bSignedElement[0][0];
}

/// \todo change this if we want to support data where elements are of
///  different type
bool UVFMetadata::GetIsFloat() const
{
  return GetBitWidth() != m_pVolumeDataBlock->ulElementBitSize[0][0];
}

bool UVFMetadata::IsSameEndianness() const
{
  return m_bIsSameEndianness;
}


const std::vector<UINT64>&
UVFMetadata::GetBrickCountND(const std::vector<UINT64>& vLOD) const
{
  return m_pVolumeDataBlock->GetBrickCount(vLOD);
}

const std::vector<UINT64>&
UVFMetadata::GetBrickSizeND(const std::vector<UINT64>& vLOD,
                            const std::vector<UINT64>& vBrick) const
{
  return m_pVolumeDataBlock->GetBrickSize(vLOD, vBrick);
}

const std::vector<UINT64>& UVFMetadata::GetDomainSizeND() const
{
  return m_pVolumeDataBlock->ulDomainSize;
}

const std::vector<UINT64>& UVFMetadata::GetMaxBrickSizeND() const
{
  return m_pVolumeDataBlock->ulBrickSize;
}

const std::vector<UINT64>& UVFMetadata::GetBrickOverlapSizeND() const
{
  return m_pVolumeDataBlock->ulBrickOverlap;
}

const std::vector<UINT64>& UVFMetadata::GetLODLevelCountND() const
{
  return m_pVolumeDataBlock->ulLODLevelCount;
}

const std::vector<double> UVFMetadata::GetScaleND() const
{
  std::vector<double> vfScale;

  const DOUBLEVECTOR3 rescale = this->GetRescaleFactors();
  const size_t iSize = m_pVolumeDataBlock->ulDomainSize.size();

  for (size_t i=0; i < iSize; i++) {
    vfScale.push_back(m_pVolumeDataBlock->dDomainTransformation[i+(iSize+1)*i]
                      * rescale[i]);
  }

  return vfScale;
}


bool UVFMetadata::ContainsData(const NDBrickKey &k, double isoval) const
{
  // if we have no max min data we have to assume that every block is visible
  if(NULL == m_pMaxMinData) {
    return true;
  }
  const UINT64VECTOR3 vBrick = UINT64VECTOR3(k.second[0].x,
                                             k.second[0].y,
                                             k.second[0].z);
  const InternalMaxMinElement& maxMinElement = m_vvaMaxMin[k.first[0]]
                                                          [vBrick.x]
                                                          [vBrick.y]
                                                          [vBrick.z];
  return (isoval <= maxMinElement.maxScalar);
}

bool UVFMetadata::ContainsData(const NDBrickKey &k,
                               double fMin,double fMax) const
{
  // if we have no max min data we have to assume that every block is visible
  if(NULL == m_pMaxMinData) {
    return true;
  }

  const UINT64VECTOR3 vBrick = UINT64VECTOR3(k.second[0].x,
                                             k.second[0].y,
                                             k.second[0].z);
  const InternalMaxMinElement& maxMinElement = m_vvaMaxMin[k.first[0]]
                                                          [vBrick.x]
                                                          [vBrick.y]
                                                          [vBrick.z];
  return (fMax >= maxMinElement.minScalar && fMin <= maxMinElement.maxScalar);
}
bool UVFMetadata::ContainsData(const NDBrickKey &k, double fMin,double fMax,
                               double fMinGradient,double fMaxGradient) const
{
  // if we have no max min data we have to assume that every block is visible
  if(NULL == m_pMaxMinData) {
    return true;
  }

  const UINT64VECTOR3 vBrick = UINT64VECTOR3(k.second[0].x,
                                             k.second[0].y,
                                             k.second[0].z);
  const InternalMaxMinElement& maxMinElement = m_vvaMaxMin[k.first[0]]
                                                          [vBrick.x]
                                                          [vBrick.y]
                                                          [vBrick.z];
  return (fMax >= maxMinElement.minScalar &&
          fMin <= maxMinElement.maxScalar)
                         &&
         (fMaxGradient >= maxMinElement.minGradient &&
          fMinGradient <= maxMinElement.maxGradient);
}

}; // namespace tuvok
