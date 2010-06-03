/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2008 Scientific Computing and Imaging Institute,
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
  \file    RAWConverter.cpp
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \date    December 2008
*/
#include "Basics/StdDefines.h"
#include <cerrno>
#include <cstring>
#include <iterator>
#include <list>
#include "3rdParty/bzip2/bzlib.h"
#include "boost/cstdint.hpp"

#include "RAWConverter.h"
#include "Basics/SysTools.h"
#include "IO/gzio.h"
#include "UVF/Histogram1DDataBlock.h"
#include "UVF/Histogram2DDataBlock.h"
#include "UVF/MaxMinDataBlock.h"
#include "UVF/RasterDataBlock.h"
#include "UVF/KeyValuePairDataBlock.h"

using namespace std;
using boost::int64_t;
using namespace tuvok;

// holds UVF data blocks, because they cannot be deleted until the UVF file is
// written.
struct TimestepBlocks {
  TimestepBlocks(): rdb(NULL), maxmin(NULL), hist2d(NULL) {}
  ~TimestepBlocks() { delete rdb; delete maxmin; delete hist2d; }
  RasterDataBlock *rdb;
  MaxMinDataBlock *maxmin;
  Histogram2DDataBlock *hist2d;
};

bool RAWConverter::ConvertRAWDataset(const string& strFilename,
                                     const string& strTargetFilename,
                                     const string& strTempDir,
                                     UINT64 iHeaderSkip,
                                     UINT64 iComponentSize,
                                     UINT64 iComponentCount,
                                     UINT64 timesteps,
                                     bool bConvertEndianness, bool bSigned,
                                     bool bIsFloat,
                                     UINT64VECTOR3 vVolumeSize,
                                     FLOATVECTOR3 vVolumeAspect,
                                     const string& strDesc,
                                     const string& strSource,
                                     const UINT64 iTargetBrickSize,
                                     const UINT64 iTargetBrickOverlap,
                                     UVFTables::ElementSemanticTable eType,
                                     KVPairs* pKVPairs,
                                     const bool bQuantizeTo8Bit)
{
  bool bMetadata_SourceIsLittleEndian = bConvertEndianness && EndianConvert::IsBigEndian();
  bool bMetadata_Signed = bSigned;
  bool bMetadata_IsFloat = bIsFloat;
  UINT64 iMetadata_ComponentSize = iComponentSize;

  if (iComponentCount > 4) {
    T_ERROR("Currently, only up to four component data is supported.");
    return false;
  }

  if (bConvertEndianness && iComponentSize < 16) { // catch silly user input
    WARNING("Requested endian conversion for 8bit data... broken reader?");
    bConvertEndianness = false;
  }

  MESSAGE("Converting RAW dataset %s to %s", strFilename.c_str(), strTargetFilename.c_str());

  string strSourceFilename;
  string tmpFilename0 = strTempDir+SysTools::GetFilename(strFilename)+".endianess";
  string tmpFilename1 = strTempDir+SysTools::GetFilename(strFilename)+".quantized";

  if (bConvertEndianness) {
    MESSAGE("Performing endianess conversion ...");

    if (iComponentSize != 16 && iComponentSize != 32 && iComponentSize != 64) {
      T_ERROR("Unable to endian convert anything but 16bit, 32bit, or 64bit values (requested %i)", int(iComponentSize));
      return false;
    }

    LargeRAWFile WrongEndianData(strFilename, iHeaderSkip);
    WrongEndianData.Open(false);

    if (!WrongEndianData.IsOpen()) {
      T_ERROR("Unable to open source file %s", strFilename.c_str());
      return false;
    }

    LargeRAWFile ConvEndianData(tmpFilename0);
    ConvEndianData.Create();

    if (!ConvEndianData.IsOpen()) {
      T_ERROR("Unable to open temp file %s for endianess conversion", tmpFilename0.c_str());
      WrongEndianData.Close();
      return false;
    }

    UINT64 ulFileLength = WrongEndianData.GetCurrentSize();
    // hint: this must fit into memory otherwise other subsystems would break
    size_t iBufferSize = std::min<size_t>(
      size_t(ulFileLength),
      size_t(iTargetBrickSize * iTargetBrickSize * iTargetBrickSize *
             iComponentSize/8)
    );
    UINT64 ulBufferConverted = 0;

    unsigned char* pBuffer = new unsigned char[iBufferSize];

    while (ulBufferConverted < ulFileLength) {
      size_t iBytesRead = WrongEndianData.ReadRAW(pBuffer, iBufferSize);

      switch (iComponentSize) {
        case 16 : for (size_t i = 0;i<iBytesRead;i+=2)
                    EndianConvert::Swap<unsigned short>((unsigned short*)(pBuffer+i));
                  break;
        case 32 : for (size_t i = 0;i<iBytesRead;i+=4)
                    EndianConvert::Swap<float>((float*)(pBuffer+i));
                  break;
        case 64 : for (size_t i = 0;i<iBytesRead;i+=8)
                    EndianConvert::Swap<double>((double*)(pBuffer+i));
                  break;
      }

      size_t iBytesWritten = ConvEndianData.WriteRAW(pBuffer, iBytesRead);

      if (iBytesRead != iBytesWritten)  {
        T_ERROR("Read/Write error converting endianess from %s to %s",
                strFilename.c_str(), tmpFilename0.c_str());
        WrongEndianData.Close();
        ConvEndianData.Close();
        Remove(tmpFilename0, Controller::Debug::Out());
        delete [] pBuffer;
        return false;
      }

      ulBufferConverted += UINT64(iBytesWritten);

      MESSAGE("Performing endianess conversion"
              "\n%i%% complete",
              int(float(ulBufferConverted) / float(ulFileLength)*100.0f));
    }

    delete [] pBuffer;
    WrongEndianData.Close();
    ConvEndianData.Close();
    strSourceFilename = tmpFilename0;
    iHeaderSkip = 0;  // the new file is straight raw without any header
  } else strSourceFilename = strFilename;

  Histogram1DDataBlock Histogram1D;

  if (bQuantizeTo8Bit && iComponentSize > 8) {
    strSourceFilename = QuantizeTo8Bit(
        iHeaderSkip, strSourceFilename, tmpFilename1,
        iComponentSize, iComponentCount*vVolumeSize.volume()*timesteps,
        bSigned, bIsFloat, &Histogram1D
    );
    if (strSourceFilename == "") {
      T_ERROR("Unsupported source format");
      return false;
    }
    iComponentSize = 8;
  } else {
    switch (iComponentSize) {
      case 8 :
        // do not run the Process8Bits when we are dealing with unsigned
        // color data, in that case only the histogram would be computed
        // and we do not use in that case.
        /// \todo change this if we want to support non-color
        /// multi-component data
        MESSAGE("Dataset is 8bit.");
        if (iComponentCount != 4 || bSigned) {
          MESSAGE("%u component, %s data",
                  static_cast<unsigned>(iComponentCount),
                  (bSigned) ? "signed" : "unsigned");
          strSourceFilename = Process8Bits(
            iHeaderSkip, strSourceFilename, tmpFilename1,
            iComponentCount*vVolumeSize.volume()*timesteps,
            bSigned, &Histogram1D
          );
        }
        break;
      case 16 :
        MESSAGE("Dataset is 16bit integers (shorts)");
        if(bSigned) {
          strSourceFilename =
            Quantize<short, unsigned short>(
              iHeaderSkip, strSourceFilename, tmpFilename1,
              iComponentCount*vVolumeSize.volume()*timesteps, &Histogram1D
            );
        } else {
          strSourceFilename =
            Quantize<unsigned short, unsigned short>(
              iHeaderSkip, strSourceFilename, tmpFilename1,
              iComponentCount*vVolumeSize.volume()*timesteps, &Histogram1D
            );
        }
        break;
      case 32 :
        if (bIsFloat) {
          MESSAGE("Dataset is 32bit FP (floats)");
          strSourceFilename =
            Quantize<float, unsigned short>(
              iHeaderSkip, strSourceFilename, tmpFilename1,
              iComponentCount*vVolumeSize.volume()*timesteps,
              &Histogram1D
            );
        } else {
          MESSAGE("Dataset is 32bit integers.");
          if(bSigned) {
            strSourceFilename =
              Quantize<boost::int32_t, unsigned short>(
                iHeaderSkip, strSourceFilename, tmpFilename1,
                iComponentCount*vVolumeSize.volume()*timesteps,
                &Histogram1D
              );
          } else {
            strSourceFilename =
              Quantize<UINT32, unsigned short>(
                iHeaderSkip, strSourceFilename, tmpFilename1,
                iComponentCount*vVolumeSize.volume()*timesteps, &Histogram1D
              );
          }
        }
        iComponentSize = 16;
        break;
      case 64 :
        if (bIsFloat) {
          MESSAGE("Dataset is 64bit FP (doubles).");
          strSourceFilename =
            Quantize<double, unsigned short>(
              iHeaderSkip, strSourceFilename, tmpFilename1,
              iComponentCount*vVolumeSize.volume()*timesteps,
              &Histogram1D
            );
        } else {
          MESSAGE("Dataset is 64bit integers.");
          if(bSigned) {
            strSourceFilename =
              Quantize<INT64, unsigned short>(
                iHeaderSkip, strSourceFilename, tmpFilename1,
                iComponentCount*vVolumeSize.volume()*timesteps,
                &Histogram1D
              );
          } else {
            strSourceFilename =
              Quantize<UINT64, unsigned short>(
                iHeaderSkip, strSourceFilename, tmpFilename1,
                iComponentCount*vVolumeSize.volume()*timesteps,
                &Histogram1D
              );
          }
        }
        iComponentSize = 16;
        break;
    }
  }

  if (strSourceFilename == "")  {
    T_ERROR("Read/Write error quantizing to %s", strFilename.c_str());
    return false;
  }

  bool bQuantized;
  if (strSourceFilename == tmpFilename1) {
    bQuantized = true;

    // if we actually created two temp file so far we can delete the first one
    if (bConvertEndianness) {
      Remove(tmpFilename0, Controller::Debug::Out());
      bConvertEndianness = false;
    }

    iHeaderSkip = 0; // the new file is straigt raw without any header
  } else {
    bQuantized = false;
  }

  wstring wstrUVFName(strTargetFilename.begin(), strTargetFilename.end());
  UVF uvfFile(wstrUVFName);

  // assume all timesteps have same dimensions / etc, so the LOD calculation
  // applies to all TSs.
  UINT64 iLodLevelCount = 1;
  UINT64 iMaxVal = vVolumeSize.maxVal();
  // generate LOD down to at least a 64^3 brick
  while (iMaxVal > std::min<UINT64>(64, iTargetBrickSize)) {
    iMaxVal /= 2;
    iLodLevelCount++;
  }

  GlobalHeader uvfGlobalHeader;
  uvfGlobalHeader.bIsBigEndian = EndianConvert::IsBigEndian();
  uvfGlobalHeader.ulChecksumSemanticsEntry = UVFTables::CS_MD5;
  uvfFile.SetGlobalHeader(uvfGlobalHeader);

  std::vector<struct TimestepBlocks> blocks(static_cast<size_t>(timesteps));

  for(size_t ts=0; ts < static_cast<size_t>(timesteps); ++ts) {
    blocks[ts].rdb = new RasterDataBlock();
    blocks[ts].maxmin = new MaxMinDataBlock(
                          static_cast<size_t>(iComponentCount)
                        );
    blocks[ts].hist2d = new Histogram2DDataBlock();
    RasterDataBlock* dataVolume = blocks[ts].rdb;

    if (strSource == "") {
      dataVolume->strBlockID = (strDesc!="")
                               ? strDesc + " volume converted by ImageVis3D"
                               : "Volume converted by ImageVis3D";
    } else {
      dataVolume->strBlockID = (strDesc!="")
                              ? strDesc + " volume converted from " + strSource
                                + " by ImageVis3D"
                              : "Volume converted from " + strSource +
                                " by ImageVis3D";
    }

    dataVolume->ulCompressionScheme = UVFTables::COS_NONE;
    dataVolume->ulDomainSemantics.push_back(UVFTables::DS_X);
    dataVolume->ulDomainSemantics.push_back(UVFTables::DS_Y);
    dataVolume->ulDomainSemantics.push_back(UVFTables::DS_Z);

    dataVolume->ulDomainSize.push_back(vVolumeSize.x);
    dataVolume->ulDomainSize.push_back(vVolumeSize.y);
    dataVolume->ulDomainSize.push_back(vVolumeSize.z);

    dataVolume->ulLODDecFactor.push_back(2);
    dataVolume->ulLODDecFactor.push_back(2);
    dataVolume->ulLODDecFactor.push_back(2);

    dataVolume->ulLODGroups.push_back(0);
    dataVolume->ulLODGroups.push_back(0);
    dataVolume->ulLODGroups.push_back(0);

    dataVolume->ulLODLevelCount.push_back(iLodLevelCount);

    vector<UVFTables::ElementSemanticTable> vSem;

    switch (iComponentCount) {
      case 3 : vSem.push_back(UVFTables::ES_RED);
               vSem.push_back(UVFTables::ES_GREEN);
               vSem.push_back(UVFTables::ES_BLUE);
               break;
      case 4 : vSem.push_back(UVFTables::ES_RED);
               vSem.push_back(UVFTables::ES_GREEN);
               vSem.push_back(UVFTables::ES_BLUE);
               vSem.push_back(UVFTables::ES_ALPHA);
               break;
      default : for (UINT64 i = 0;i<iComponentCount;i++) {
                  vSem.push_back(eType);
                }
                break;
    }

    dataVolume->SetTypeToVector(iComponentSize,
                                iComponentSize == 32 ? 23 : iComponentSize,
                                bSigned,
                                vSem);

    dataVolume->ulBrickSize.push_back(iTargetBrickSize);
    dataVolume->ulBrickSize.push_back(iTargetBrickSize);
    dataVolume->ulBrickSize.push_back(iTargetBrickSize);

    dataVolume->ulBrickOverlap.push_back(iTargetBrickOverlap);
    dataVolume->ulBrickOverlap.push_back(iTargetBrickOverlap);
    dataVolume->ulBrickOverlap.push_back(iTargetBrickOverlap);

    vector<double> vScale;
    vScale.push_back(vVolumeAspect.x);
    vScale.push_back(vVolumeAspect.y);
    vScale.push_back(vVolumeAspect.z);
    dataVolume->SetScaleOnlyTransformation(vScale);

    MaxMinDataBlock& MaxMinData = *blocks[ts].maxmin;
    std::string tmpfile;
    {
      ostringstream tmpfn;
      tmpfn << strTempDir << ts << "tempFile.tmp";
      tmpfile = tmpfn.str();
    }
    AbstrDebugOut *dbg = &Controller::Debug::Out();

    bool bBrickingOK = false;
    LargeRAWFile SourceData(strSourceFilename, iHeaderSkip);
    // increment the header skip so the next iteration pulls out the next
    // timestep in our conglomeration of multiple TSs into a single file.
    iHeaderSkip += iComponentSize/8 * iComponentCount * vVolumeSize.volume();
    SourceData.Open(false);

    if (!SourceData.IsOpen()) {
      T_ERROR("Unable to open source file %s", strSourceFilename.c_str());
      return false;
    }

    switch (iComponentSize) {
      case 8 :
        switch (iComponentCount) {
          case 1:
            bBrickingOK =dataVolume->FlatDataToBrickedLOD(&SourceData, tmpfile+"1",
                                            CombineAverage<unsigned char,1>,
                                            SimpleMaxMin<unsigned char,1>,
                                            &MaxMinData, dbg);
            break;
          case 2:
            bBrickingOK =dataVolume->FlatDataToBrickedLOD(&SourceData, tmpfile+"2",
                                            CombineAverage<unsigned char,2>,
                                            SimpleMaxMin<unsigned char,2>,
                                            &MaxMinData, dbg);
            break;
          case 3:
            bBrickingOK =dataVolume->FlatDataToBrickedLOD(&SourceData, tmpfile+"3",
                                            CombineAverage<unsigned char,3>,
                                            SimpleMaxMin<unsigned char,3>,
                                            &MaxMinData, dbg);
            break;
          case 4:
            bBrickingOK =dataVolume->FlatDataToBrickedLOD(&SourceData, tmpfile+"4",
                                            CombineAverage<unsigned char,4>,
                                            SimpleMaxMin<unsigned char,4>,
                                            &MaxMinData, dbg);
            break;
          default:
            T_ERROR("Unsupported iComponentCount %i for iComponentSize %i.",
                    int(iComponentCount), int(iComponentSize));
            uvfFile.Close();
            SourceData.Close();
            return false;
        }
        break;
      case 16:
            switch (iComponentCount) {
              case 1 : bBrickingOK = dataVolume->FlatDataToBrickedLOD(&SourceData, tmpfile+"1", CombineAverage<unsigned short,1>, SimpleMaxMin<unsigned short, 1>, &MaxMinData, &Controller::Debug::Out()); break;
              case 2 : bBrickingOK = dataVolume->FlatDataToBrickedLOD(&SourceData, tmpfile+"2", CombineAverage<unsigned short,2>, SimpleMaxMin<unsigned short, 2>, &MaxMinData, &Controller::Debug::Out()); break;
              case 3 : bBrickingOK = dataVolume->FlatDataToBrickedLOD(&SourceData, tmpfile+"3", CombineAverage<unsigned short,3>, SimpleMaxMin<unsigned short, 3>, &MaxMinData, &Controller::Debug::Out()); break;
              case 4 : bBrickingOK = dataVolume->FlatDataToBrickedLOD(&SourceData, tmpfile+"4", CombineAverage<unsigned short,4>, SimpleMaxMin<unsigned short, 4>, &MaxMinData, &Controller::Debug::Out()); break;
              default: T_ERROR("Unsupported iComponentCount %i for iComponentSize %i.", int(iComponentCount), int(iComponentSize)); uvfFile.Close(); SourceData.Close(); return false;
            } break;
      case 32 :
            switch (iComponentCount) {
              case 1 : bBrickingOK = dataVolume->FlatDataToBrickedLOD(&SourceData, tmpfile+"1", CombineAverage<float,1>, SimpleMaxMin<float, 1>, &MaxMinData, &Controller::Debug::Out()); break;
              case 2 : bBrickingOK = dataVolume->FlatDataToBrickedLOD(&SourceData, tmpfile+"2", CombineAverage<float,2>, SimpleMaxMin<float, 2>, &MaxMinData, &Controller::Debug::Out()); break;
              case 3 : bBrickingOK = dataVolume->FlatDataToBrickedLOD(&SourceData, tmpfile+"3", CombineAverage<float,3>, SimpleMaxMin<float, 3>, &MaxMinData, &Controller::Debug::Out()); break;
              case 4 : bBrickingOK = dataVolume->FlatDataToBrickedLOD(&SourceData, tmpfile+"4", CombineAverage<float,4>, SimpleMaxMin<float, 4>, &MaxMinData, &Controller::Debug::Out()); break;
              default: T_ERROR("Unsupported iComponentCount %i for iComponentSize %i.", int(iComponentCount), int(iComponentSize)); uvfFile.Close(); SourceData.Close(); return false;
            } break;
      default: T_ERROR("Unsupported iComponentSize %i.", int(iComponentSize)); uvfFile.Close(); SourceData.Close(); return false;
    }

    if (!bBrickingOK) {
      uvfFile.Close();
      T_ERROR("Brick generation failed, aborting.");
      return false;
    }

    string strProblemDesc;
    if (!dataVolume->Verify(&strProblemDesc)) {
      T_ERROR("Verify failed with the following reason: %s",
              strProblemDesc.c_str());
      uvfFile.Close();
      if (bConvertEndianness) {
        Remove(tmpFilename0, Controller::Debug::Out());
      }
      if (bQuantized) {
        Remove(tmpFilename1, Controller::Debug::Out());
      }
      return false;
    }

    if (!uvfFile.AddDataBlock(dataVolume,dataVolume->ComputeDataSize(), true)) {
      T_ERROR("AddDataBlock failed!");
      uvfFile.Close();
      if (bConvertEndianness) {
        Remove(tmpFilename0, Controller::Debug::Out());
      }
      if (bQuantized) {
        Remove(tmpFilename1, Controller::Debug::Out());
      }
      return false;
    }

    // do compute histograms when we are dealing with color data
    /// \todo change this if we want to support non color multi component data
    if (iComponentCount != 4) {
      // if no resampling was perfomed above, we need to compute the
      // 1d histogram here
      if (Histogram1D.GetHistogram().empty()) {
        MESSAGE("Computing 1D Histogram...");
        if (!Histogram1D.Compute(dataVolume)) {
          T_ERROR("Computation of 1D Histogram failed!");
          uvfFile.Close();
          if (bConvertEndianness) {
            Remove(tmpFilename0, Controller::Debug::Out());
          }
          if (bQuantized) {
            Remove(tmpFilename1, Controller::Debug::Out());
          }
          return false;
        }
      }

      MESSAGE("Computing 2D Histogram...");
      Histogram2DDataBlock& Histogram2D = *blocks[ts].hist2d;
      if (!Histogram2D.Compute(dataVolume, Histogram1D.GetHistogram().size())) {
        T_ERROR("Computation of 2D Histogram failed!");
        uvfFile.Close();
        if (bConvertEndianness) {
          Remove(tmpFilename0, Controller::Debug::Out());
        }
        if (bQuantized) {
          Remove(tmpFilename1, Controller::Debug::Out());
        }
        return false;
      }
      MESSAGE("Storing histogram data...");
      uvfFile.AddDataBlock(&Histogram1D,Histogram1D.ComputeDataSize());
      uvfFile.AddDataBlock(&Histogram2D,Histogram2D.ComputeDataSize());
    }

    MESSAGE("Storing acceleration data...");
    uvfFile.AddDataBlock(&MaxMinData, MaxMinData.ComputeDataSize());
    SourceData.Close();
  }


  MESSAGE("Storing metadata...");

  KeyValuePairDataBlock metaPairs;
  if (strSource != "") metaPairs.AddPair("Data Source",strSource);
  if (strDesc != "") metaPairs.AddPair("Description",strDesc);

  if (bMetadata_SourceIsLittleEndian)
    metaPairs.AddPair("Source Endianess","little");
  else
    metaPairs.AddPair("Source Endianess","big");

  if (bMetadata_IsFloat)
    metaPairs.AddPair("Source Type","float");
  else
    if (bMetadata_Signed)
      metaPairs.AddPair("Source Type","signed integer");
    else
      metaPairs.AddPair("Source Type","integer");

  metaPairs.AddPair("Source Bitwidth",
                    SysTools::ToString(iMetadata_ComponentSize));

  if (pKVPairs) {
    for (size_t i = 0;i<pKVPairs->size();i++) {
      metaPairs.AddPair(pKVPairs->at(i).first,pKVPairs->at(i).second);
    }
  }

  UINT64 iDataSize = metaPairs.ComputeDataSize();
  uvfFile.AddDataBlock(&metaPairs,iDataSize);

  MESSAGE("Writing UVF file...");

  uvfFile.Create();

  MESSAGE("Computing checksum...");

  uvfFile.Close();
  blocks.clear();

  MESSAGE("Removing temporary files...");

  if (bConvertEndianness) {
    Remove(tmpFilename0, Controller::Debug::Out());
  }
  if (bQuantized) {
    Remove(tmpFilename1, Controller::Debug::Out());
  }

  MESSAGE("Done!");
  return true;
}


#ifdef WIN32
  #pragma warning( disable : 4996 ) // disable deprecated warning
#endif

/** Converts a gzip-compressed chunk of a file to a raw file.
 * @param strFilename the input (compressed) file
 * @param strTargetFilename the target uvf file
 * @param iHeaderSkip number of bytes to skip off of strFilename */
bool RAWConverter::ExtractGZIPDataset(const string& strFilename,
                                      const string& strUncompressedFile,
                                      UINT64 iHeaderSkip)
{
  FILE *f_compressed;
  FILE *f_inflated;
  int ret;

  MESSAGE("Deflating GZIP data ...");

  f_compressed = fopen(strFilename.c_str(), "rb");
  f_inflated = fopen(strUncompressedFile.c_str(), "wb");

  if(f_compressed == NULL) {
    T_ERROR("Could not open %s", strFilename.c_str());
    fclose(f_inflated);
    return false;
  }
  if(f_inflated == NULL) {
    T_ERROR("Could not open %s", strUncompressedFile.c_str());
    fclose(f_compressed);
    return false;
  }

  if(fseek(f_compressed, static_cast<long>(iHeaderSkip), SEEK_SET) != 0) {
    /// \todo use strerror(errno) and actually report the damn error.
    T_ERROR("Seek failed");
    fclose(f_compressed);
    fclose(f_inflated);
    remove(strUncompressedFile.c_str());
    return false;
  }

  gz_skip_header(f_compressed); // always needed?

  ret = gz_inflate(f_compressed, f_inflated);

  fclose(f_compressed);
  fclose(f_inflated);

  switch(ret) {
    case Z_OK:
      MESSAGE("Decompression successful.");
      break;
    case Z_MEM_ERROR:
      T_ERROR("Not enough memory decompress %s",
                 strFilename.c_str());
      return false;
      break;
    case Z_DATA_ERROR:
      T_ERROR("Deflation invalid or incomplete");
      return false;
      break;
    case Z_VERSION_ERROR:
      T_ERROR("Zlib library versioning error!");
      return false;
      break;
    default:
      WARNING("Unknown / unhandled case %d", ret);
      return false;
      break;
  }

  return true;
}

/** Tests a bzip return code for errors, and translates it to a string for the
 * debug logs.
 * @param bz_err the error code (given by the bzip2 library)
 * @return true if an error occurred */
static bool
bz_err_test(int bz_err)
{
#ifdef TUVOK_NO_IO
  T_ERROR("bzip2 library not available!");
  return true;
#else
  bool error_occurred = true;
  switch(bz_err) {
        case BZ_OK:        /* FALL THROUGH */
        case BZ_RUN_OK:    /* FALL THROUGH */
        case BZ_FLUSH_OK:  /* FALL THROUGH */
        case BZ_FINISH_OK:
            MESSAGE("Bzip operation successful.");
            error_occurred = false;
            break;
        case BZ_STREAM_END:
            MESSAGE("End of bzip stream.");
            break;
        case BZ_CONFIG_ERROR:
            T_ERROR("Bzip configuration error");
            break;
        case BZ_SEQUENCE_ERROR:
            T_ERROR("Bzip sequencing error");
            break;
        case BZ_PARAM_ERROR:
            T_ERROR("Bzip parameter error");
            break;
        case BZ_MEM_ERROR:
            T_ERROR("Bzip memory allocation failed.");
            break;
        case BZ_DATA_ERROR_MAGIC:
            WARNING("Bzip stream does not have correct magic bytes!");
            /* FALL THROUGH */
        case BZ_DATA_ERROR:
            T_ERROR("Bzip data integrity error; this usually means the "
                    "compressed file is corrupt.");
            break;
        case BZ_IO_ERROR: {
            const char *err_msg = strerror(errno);
            T_ERROR("Bzip IO error: %s", err_msg);
            break;
        }
        case BZ_UNEXPECTED_EOF:
            WARNING("EOF before end of Bzip stream.");
            break;
        case BZ_OUTBUFF_FULL:
            T_ERROR("Bzip output buffer is not large enough");
            break;
    }
    return error_occurred;
#endif
}

/** Converts a bzip2-compressed file chunk to a raw file.
 * @param strFilename the input (compressed) file
 * @param strTargetFilename the target uvf file
 * @param iHeaderSkip number of bytes to skip of strFilename's header*/
bool RAWConverter::ExtractBZIP2Dataset(const string& strFilename,
                                       const string& strUncompressedFile,
                                       UINT64 iHeaderSkip)
{
#ifdef TUVOK_NO_IO
  T_ERROR("Tuvok built without IO routines; bzip2 not available!");
  return false;
#else
  BZFILE *bzf;
  int bz_err;
  size_t iCurrentIncoreSize = GetIncoreSize();
  std::vector<char> buffer(iCurrentIncoreSize);

  FILE *f_compressed = fopen(strFilename.c_str(), "rb");
  FILE *f_inflated = fopen(strUncompressedFile.c_str(), "wb");

  if(f_compressed == NULL) {
    T_ERROR("Could not open %s", strFilename.c_str());
    fclose(f_inflated);
    return false;
  }
  if(f_inflated == NULL) {
    T_ERROR("Could not open %s", strUncompressedFile.c_str());
    fclose(f_compressed);
    return false;
  }

  if(fseek(f_compressed, static_cast<long>(iHeaderSkip), SEEK_SET) != 0) {
    /// \todo use strerror(errno) and actually report the damn error.
    T_ERROR("Seek failed");
    fclose(f_inflated);
    fclose(f_compressed);
    return false;
  }

  bzf = BZ2_bzReadOpen(&bz_err, f_compressed, 0, 0, NULL, 0);
  if(bz_err_test(bz_err)) {
    T_ERROR("Bzip library error occurred; bailing.");
    fclose(f_inflated);
    fclose(f_compressed);
    return false;
  }

  do {
    int nbytes = BZ2_bzRead(&bz_err, bzf, &buffer[0], int(iCurrentIncoreSize));
    if(bz_err != BZ_STREAM_END && bz_err_test(bz_err)) {
      T_ERROR("Bzip library error occurred; bailing.");
      fclose(f_inflated);
      fclose(f_compressed);
      return false;
    }
    if(1 != fwrite(&buffer[0], nbytes, 1, f_inflated)) {
      WARNING("%d-byte write of decompressed file failed.",
                  nbytes);
      fclose(f_inflated);
      fclose(f_compressed);
      return false;
    }
  } while(bz_err == BZ_OK);

  fclose(f_inflated);
  fclose(f_compressed);

  return true;
#endif
}

bool RAWConverter::ParseTXTDataset(const string& strFilename,
                                     const string& strBinaryFile,
                                     UINT64 iHeaderSkip,
                                     UINT64 iComponentSize,
                                     UINT64 iComponentCount,
                                     bool bSigned,
                                     bool bIsFloat,
                                     UINT64VECTOR3 vVolumeSize)
{
  ifstream sourceFile(strFilename.c_str(),ios::binary);
  if (!sourceFile.is_open()) {
    T_ERROR("Unable to open source file %s.", strFilename.c_str());
    return false;
  }

  LargeRAWFile binaryFile(strBinaryFile);
  binaryFile.Create(iComponentSize/8 * iComponentCount * vVolumeSize.volume());
  if (!binaryFile.IsOpen()) {
    T_ERROR("Unable to open temp file %s.", strBinaryFile.c_str());
    sourceFile.close();
    return false;
  }

  sourceFile.seekg(static_cast<std::streamoff>(iHeaderSkip));
  if (bIsFloat) {
    if (!bSigned) {
      T_ERROR("Unsupported data type (unsigned float)");
      sourceFile.close();
      binaryFile.Delete();
      return false;
    }
    switch (iComponentSize) {
      case 32 : {
                  float tmp;
                  while (sourceFile) {
                    sourceFile >> tmp;
                    if(sourceFile) {
                      binaryFile.WriteRAW((unsigned char*)&tmp,4);
                    }
                  }
                 break;
               }
      case 64 : {
                  double tmp;
                  while (sourceFile) {
                    sourceFile >> tmp;
                    if(sourceFile) {
                      binaryFile.WriteRAW((unsigned char*)&tmp,8);
                    }
                  }
                 break;
               }
      default : {
                  T_ERROR("Unable unsupported data type. (float)");
                  sourceFile.close();
                  binaryFile.Delete();
                  return false;
                }
    }
  } else {
    switch (iComponentSize) {
      case 8 : {
                  int tmp=0;
                  if (bSigned) {
                    while (sourceFile) {
                      sourceFile >> tmp;
                      if(sourceFile) {
                        signed char tmp2 = static_cast<signed char>(tmp);
                        binaryFile.WriteRAW((unsigned char*)&tmp2,1);
                      }
                    }
                  } else {
                    while (sourceFile) {
                      sourceFile >> tmp;
                      if(sourceFile) {
                        unsigned char tmp2 = static_cast<unsigned char>(tmp);
                        binaryFile.WriteRAW(&tmp2,1);
                      }
                    }
                  }
                 break;
               }
      case 16 : {
                  if (bSigned) {
                    signed short tmp;
                    while (sourceFile) {
                      sourceFile >> tmp;
                      if(sourceFile) {
                        binaryFile.WriteRAW((unsigned char*)&tmp,2);
                      }
                    }
                  } else {
                    unsigned short tmp;
                    while (sourceFile) {
                      sourceFile >> tmp;
                      if(sourceFile) {
                        binaryFile.WriteRAW((unsigned char*)&tmp,2);
                      }
                    }
                  }
                 break;
               }
      case 32 : {
                  if (bSigned) {
                    signed int tmp;
                    while (sourceFile) {
                      sourceFile >> tmp;
                      if(sourceFile) {
                        binaryFile.WriteRAW((unsigned char*)&tmp,4);
                      }
                    }
                  } else {
                    UINT32 tmp;
                    while (sourceFile) {
                      sourceFile >> tmp;
                      if(sourceFile) {
                        binaryFile.WriteRAW((unsigned char*)&tmp,4);
                      }
                    }
                  }
                 break;
               }
      default : {
                  T_ERROR("Unable unsupported data type. (int)");
                  sourceFile.close();
                  binaryFile.Delete();
                  return false;
                }
    }
  }
  binaryFile.Close();
  sourceFile.close();

  return true;
}

bool RAWConverter::ConvertToNative(const std::string& strRawFilename,
                                   const std::string& strTargetFilename,
                                   UINT64 iHeaderSkip,
                                   UINT64 iComponentSize, UINT64, bool, bool,
                                   UINT64VECTOR3, FLOATVECTOR3, bool,
                                   bool bQuantizeTo8Bit) {
  // convert raw to raw is easy :-), just copy the file and ignore the metadata

  // if the file exists, delete it first
  if (SysTools::FileExists(strTargetFilename))
    Remove(strTargetFilename, Controller::Debug::Out());
  if (SysTools::FileExists(strTargetFilename)) {
    T_ERROR("Unable to remove existing target file %s.", strTargetFilename.c_str());
    return false;
  }

  return AppendRAW(strRawFilename, iHeaderSkip, strTargetFilename, iComponentSize, EndianConvert::IsBigEndian(),bQuantizeTo8Bit);
}

bool RAWConverter::AppendRAW(const std::string& strRawFilename,
                             UINT64 iHeaderSkip,
                             const std::string& strTargetFilename,
                             UINT64 iComponentSize, bool bChangeEndianess,
                             bool bToSigned, bool bQuantizeTo8Bit) {
  // TODO:
  // should we ever need this combination
  // "append +quantize" the implemenation should be here :-)
  if (bQuantizeTo8Bit) {
    T_ERROR("Quantization to 8bit during append operations not supported.");
    return false;
  }

  // open source file
  LargeRAWFile fSource(strRawFilename, iHeaderSkip);
  fSource.Open(false);
  if (!fSource.IsOpen()) {
    T_ERROR("Unable to open source file %s.", strRawFilename.c_str());
    return false;
  }
  // append to target file
  LargeRAWFile fTarget(strTargetFilename);
  fTarget.Append();
  if (!fTarget.IsOpen()) {
    fSource.Close();
    T_ERROR("Unable to open target file %s.", strTargetFilename.c_str());
    return false;
  }

  UINT64 iSourceSize = fSource.GetCurrentSize();
  UINT64 iCopySize = min(iSourceSize,BLOCK_COPY_SIZE);
  unsigned char* pBuffer = new unsigned char[size_t(iCopySize)];
  UINT64 iCopiedSize = 0;

  do {
    MESSAGE("Writing output data\n%g%% completed", 100.0f*float(iCopiedSize)/float(iSourceSize));

    iCopySize = fSource.ReadRAW(pBuffer, iCopySize);

    if (bToSigned) {

      switch (iComponentSize) {
        case 8  : // char to uchar
                  for (size_t i = 0;i<iCopySize;i++)
                    (*(char*)(pBuffer+i)) = char(*(unsigned char*)(pBuffer+i)) - std::numeric_limits<char>::max();
                  break;
        case 16 : // short to ushort
                  for (size_t i = 0;i<iCopySize;i+=2)
                    (*(short*)(pBuffer+i)) = short(*(unsigned short*)(pBuffer+i)) - std::numeric_limits<short>::max();
                  break;
        case 32 : // int to uint
                  for (size_t i = 0;i<iCopySize;i+=4)
                    (*(int*)(pBuffer+i)) = int(*(unsigned int*)(pBuffer+i)) - std::numeric_limits<int>::max();
                  break;
        case 64 : // ulonglong to longlong
                  for (size_t i = 0;i<iCopySize;i+=8)
                    (*(int64_t*)(pBuffer+i)) = int64_t(*(UINT64*)(pBuffer+i)) - std::numeric_limits<int64_t>::max();
                  break;
        default : T_ERROR("Unsuported data type for vff files.");
                  return false;
      }
    }

    if (bChangeEndianess) {
      switch (iComponentSize) {
        case 16 : for (size_t i = 0;i<iCopySize;i+=2)
                    EndianConvert::Swap<unsigned short>((unsigned short*)(pBuffer+i));
                  break;
        case 32 : for (size_t i = 0;i<iCopySize;i+=4)
                    EndianConvert::Swap<float>((float*)(pBuffer+i));
                  break;
        case 64 : for (size_t i = 0;i<iCopySize;i+=8)
                    EndianConvert::Swap<double>((double*)(pBuffer+i));
                  break;
      }
    }

    fTarget.WriteRAW(pBuffer, iCopySize);
    iCopiedSize += iCopySize;
  } while (iCopySize > 0);

  fSource.Close();
  fTarget.Close();
  delete [] pBuffer;

  return true;
}


bool RAWConverter::ConvertToUVF(const std::string& strSourceFilename,
                                const std::string& strTargetFilename,
                                const std::string& strTempDir,
                                const bool bNoUserInteraction,
                                const UINT64 iTargetBrickSize,
                                const UINT64 iTargetBrickOverlap,
                                const bool bQuantizeTo8Bit)
{
  std::list<std::string> files;
  files.push_front(strSourceFilename);
  return ConvertToUVF(files, strTargetFilename, strTempDir, bNoUserInteraction,
                      iTargetBrickSize, iTargetBrickOverlap, bQuantizeTo8Bit);
}

static void RemoveStdString(std::string s) { remove(s.c_str()); }

bool RAWConverter::ConvertToUVF(const std::list<std::string>& files,
                                const std::string& strTargetFilename,
                                const std::string& strTempDir,
                                const bool bNoUserInteraction,
                                const UINT64 iTargetBrickSize,
                                const UINT64 iTargetBrickOverlap,
                                const bool bQuantizeTo8Bit)
{
  UINT64        iComponentSize=8;
  UINT64        iComponentCount=1;
  bool          bConvertEndianess=false;
  bool          bSigned=true;
  bool          bIsFloat=false;
  UINT64VECTOR3 vVolumeSize;
  FLOATVECTOR3  vVolumeAspect;
  string        strTitle;
  UVFTables::ElementSemanticTable eType;
  std::list<string> strIntermediateFile;
  std::list<bool>   bDeleteIntermediateFile;
  std::list<UINT64> header_skip;

  bool success = true;
  for(std::list<std::string>::const_iterator fn = files.begin();
      fn != files.end(); ++fn) {
    std::string intermediate;
    bool bDelete;
    UINT64 iHeaderSkip;
    /// @todo assuming iComponentSize, etc. are the same for all files; should
    /// really be a list for each of them, like for intermediate, iHeaderskip,
    /// etc.
    success &= ConvertToRAW(*fn, strTempDir,
                            bNoUserInteraction,
                            iHeaderSkip, iComponentSize, iComponentCount,
                            bConvertEndianess, bSigned, bIsFloat,
                            vVolumeSize, vVolumeAspect, strTitle,
                            eType, intermediate, bDelete);
    if(!success) { break; }
    strIntermediateFile.push_front(intermediate);
    bDeleteIntermediateFile.push_front(bDelete);
    header_skip.push_front(iHeaderSkip);
  }
  // then rewrite convertrawdataset to take the new list

  if (!success) {
    T_ERROR("Convert to RAW step failed, aborting.");
    std::for_each(strIntermediateFile.begin(), strIntermediateFile.end(),

                  RemoveStdString);
    return false;
  }

  std::string merged_fn;
  std::string dataSource;
  if(files.size() > 1) {
    merged_fn = strTempDir + ".merged_time_filename";
    remove(merged_fn.c_str());
    // copy all of the data to a single file
    LargeRAWFile merged(merged_fn);
    merged.Create();

    std::list<bool>::const_iterator del = bDeleteIntermediateFile.begin();
    std::list<UINT64>::const_iterator hdr = header_skip.begin();
    const UINT64 payload_sz = vVolumeSize.volume() * iComponentSize/8 *
                              iComponentCount;
    for(std::list<std::string>::const_iterator fn = strIntermediateFile.begin();
        fn != strIntermediateFile.end(); ++fn, ++del, ++hdr) {
      LargeRAWFile input(*fn, *hdr);
      input.Open(false);

      unsigned char *data = new unsigned char[GetIncoreSize()];
      size_t bytes_written =0;
      do {
        size_t elems = input.ReadRAW(data, GetIncoreSize());
        if(elems == 0) {
          WARNING("Input file '%s' ended before we expected.", fn->c_str());
          break;
        }
        merged.WriteRAW(data, std::min(payload_sz - bytes_written,
                                       static_cast<UINT64>(elems)));
        bytes_written += elems;
      } while(bytes_written < payload_sz);

      if(*del) {
        input.Delete();
      } else {
        input.Close();
      }
    }
    *bDeleteIntermediateFile.begin() = true;
    *header_skip.begin() = 0;
    {
      ostringstream strlist;
      strlist << "Merged from ";
      std::copy(files.begin(), files.end(),
                std::ostream_iterator<std::string>(strlist, ", "));
      dataSource = strlist.str();
    }
  } else {
    merged_fn = *strIntermediateFile.begin();
    dataSource = SysTools::GetFilename(*files.begin());
  }

  bool bUVFCreated = ConvertRAWDataset(merged_fn, strTargetFilename,
                                       strTempDir, *header_skip.begin(),
                                       iComponentSize,
                                       iComponentCount,
                                       files.size(),
                                       bConvertEndianess,
                                       bSigned, bIsFloat, vVolumeSize,
                                       vVolumeAspect, strTitle,
                                       dataSource,
                                       iTargetBrickSize, iTargetBrickOverlap,
                                       UVFTables::ES_UNDEFINED, 0,
                                       bQuantizeTo8Bit);

  if (*bDeleteIntermediateFile.begin()) {
    Remove(merged_fn, Controller::Debug::Out());
  }

  return bUVFCreated;
}

bool RAWConverter::Analyze(const std::string& strSourceFilename,
                           const std::string& strTempDir,
                           bool bNoUserInteraction, RangeInfo& info) {
  UINT64        iHeaderSkip=0;
  UINT64        iComponentSize=0;
  UINT64        iComponentCount=0;
  bool          bConvertEndianess=false;
  bool          bSigned=false;
  bool          bIsFloat=false;
  UINT64VECTOR3 vVolumeSize(0,0,0);
  FLOATVECTOR3  vVolumeAspect(0,0,0);
  string        strTitle = "";
  string        strSource = "";
  UVFTables::ElementSemanticTable eType = UVFTables::ES_UNDEFINED;

  string        strRAWFilename = "";
  bool          bRAWDelete = false;


  bool bConverted = ConvertToRAW(strSourceFilename, strTempDir,
                                 bNoUserInteraction,
                                 iHeaderSkip, iComponentSize, iComponentCount,
                                 bConvertEndianess, bSigned, bIsFloat,
                                 vVolumeSize, vVolumeAspect, strTitle,
                                 eType, strRAWFilename,
                                 bRAWDelete);
  strSource = SysTools::GetFilename(strSourceFilename);

  if (!bConverted) return false;

  info.m_vAspect = vVolumeAspect;
  info.m_vDomainSize = vVolumeSize;
  // ConvertToRAW either creates a 16 or 8 bit unsigned int, so checking
  // the iComponentSize is sufficient to make sure the types are the same
  info.m_iComponentSize = iComponentSize;

  bool bAnalyzed = Analyze(strRAWFilename, iHeaderSkip, iComponentSize, iComponentCount,
                           bSigned, bIsFloat, vVolumeSize, info);

  if (bRAWDelete) {
    Remove(strRAWFilename, Controller::Debug::Out());
  }

  return bAnalyzed;
}

bool RAWConverter::Analyze(const std::string& strSourceFilename,
                           UINT64 iHeaderSkip, UINT64 iComponentSize,
                           UINT64 iComponentCount, bool bSigned,
                           bool bFloatingPoint, UINT64VECTOR3 vVolumeSize,
                           RangeInfo& info) {
  // open source file
  LargeRAWFile fSource(strSourceFilename, iHeaderSkip);
  fSource.Open(false);
  if (!fSource.IsOpen()) {
    T_ERROR("Unable to open source file %s.", strSourceFilename.c_str());
    return false;
  }

  UINT64 iElemCount = vVolumeSize.volume()*iComponentCount;

  if (bFloatingPoint) {
    if (!bSigned) {
      T_ERROR("Unable unsupported data type. (unsiged float)");
      fSource.Close();
      return false;
    }
    info.m_iValueType = 0;
    switch (iComponentSize) {
      case 32 : {
                  float fMin = numeric_limits<float>::max();
                  float fMax = -numeric_limits<float>::max();
                  MinMaxScanner<float> scanner(&fSource, fMin, fMax, iElemCount);
                  info.m_fRange.first  = fMin;
                  info.m_fRange.second = fMax;
                  break;
                }
      case 64 : {
                  double fMin = numeric_limits<double>::max();
                  double fMax = -numeric_limits<double>::max();
                  MinMaxScanner<double> scanner(&fSource, fMin, fMax, iElemCount);
                  info.m_fRange.first  = fMin;
                  info.m_fRange.second = fMax;
                  break;
               }
      default : {
                  T_ERROR("Unable unsupported data type. (float)");
                  fSource.Close();
                  return false;
                }
    }
  } else {
    if (bSigned)
      info.m_iValueType = 1;
    else
      info.m_iValueType = 2;

    switch (iComponentSize) {
      case 8 : {
                 if (bSigned) {
                   char iMin = numeric_limits<char>::max();
                   char iMax = -numeric_limits<char>::max();
                   MinMaxScanner<char> scanner(&fSource, iMin, iMax, iElemCount);
                   info.m_iRange.first  = iMin;
                   info.m_iRange.second = iMax;
                   break;
                 } else {
                   unsigned char iMin = numeric_limits<unsigned char>::max();
                   unsigned char iMax = numeric_limits<unsigned char>::min();
                   MinMaxScanner<unsigned char> scanner(&fSource, iMin, iMax, iElemCount);
                   info.m_uiRange.first  = iMin;
                   info.m_uiRange.second = iMax;
                 }
                 break;
               }
      case 16 : {
                 if (bSigned) {
                   short iMin = numeric_limits<short>::max();
                   short iMax = -numeric_limits<short>::max();
                   MinMaxScanner<short> scanner(&fSource, iMin, iMax, iElemCount);
                   info.m_iRange.first  = iMin;
                   info.m_iRange.second = iMax;
                   break;
                 } else {
                   unsigned short iMin = numeric_limits<unsigned short>::max();
                   unsigned short iMax = numeric_limits<unsigned short>::min();
                   MinMaxScanner<unsigned short> scanner(&fSource, iMin, iMax, iElemCount);
                   info.m_uiRange.first  = iMin;
                   info.m_uiRange.second = iMax;
                 }
                 break;
               }
      case 32 : {
                 if (bSigned) {
                   int iMin = numeric_limits<int>::max();
                   int iMax = -numeric_limits<int>::max();
                   MinMaxScanner<int> scanner(&fSource, iMin, iMax, iElemCount);
                   info.m_iRange.first  = iMin;
                   info.m_iRange.second = iMax;
                   break;
                 } else {
                   unsigned int iMin = numeric_limits<unsigned int>::max();
                   unsigned int iMax = numeric_limits<unsigned int>::min();
                   MinMaxScanner<unsigned int> scanner(&fSource, iMin, iMax, iElemCount);
                   info.m_uiRange.first  = iMin;
                   info.m_uiRange.second = iMax;
                 }
                 break;
               }
      case 64 : {
                 if (bSigned) {
                   int64_t iMin = numeric_limits<int64_t>::max();
                   int64_t iMax = -numeric_limits<int64_t>::max();
                   MinMaxScanner<int64_t> scanner(&fSource, iMin, iMax, iElemCount);
                   info.m_iRange.first  = iMin;
                   info.m_iRange.second = iMax;
                   break;
                 } else {
                   UINT64 iMin = numeric_limits<UINT64>::max();
                   UINT64 iMax = numeric_limits<UINT64>::min();
                   MinMaxScanner<UINT64> scanner(&fSource, iMin, iMax, iElemCount);
                   info.m_uiRange.first  = iMin;
                   info.m_uiRange.second = iMax;
                 }
                 break;
               }
      default : {
                  T_ERROR("Unable unsupported data type. (int)");
                  fSource.Close();
                  return false;
                }
    }
  }

  fSource.Close();
  return true;
}


/// Uses remove(3) to remove the file.
/// @return true if the remove succeeded.
bool RAWConverter::Remove(const std::string &path, AbstrDebugOut &dbg)
{
  if(std::remove(path.c_str()) == -1) {
#ifdef _WIN32
      char buffer[200];
      strerror_s(buffer, 200, errno);
      dbg.Warning(_func_, "Could not remove `%s': %s", path.c_str(), buffer);
#else
      dbg.Warning(_func_, "Could not remove `%s': %s", path.c_str(),
                  strerror(errno));
#endif
      return false;
  }
  return true;
}
