set( TUVOK_HEADERS  StdTuvokDefines.h
                    3rdParty/LUA/lapi.h
                    3rdParty/LUA/lauxlib.h
                    3rdParty/LUA/lcode.h
                    3rdParty/LUA/lctype.h
                    3rdParty/LUA/ldebug.h
                    3rdParty/LUA/ldo.h
                    3rdParty/LUA/lfunc.h
                    3rdParty/LUA/lgc.h
                    3rdParty/LUA/llimits.h
                    3rdParty/LUA/lmem.h
                    3rdParty/LUA/lobject.h
                    3rdParty/LUA/lopcodes.h
                    3rdParty/LUA/lparser.h
                    3rdParty/LUA/lstate.h
                    3rdParty/LUA/lstring.h
                    3rdParty/LUA/ltable.h
                    3rdParty/LUA/ltm.h
                    3rdParty/LUA/lua.h
                    3rdParty/LUA/lua.hpp
                    3rdParty/LUA/luaconf.h
                    3rdParty/LUA/lualib.h
                    3rdParty/LUA/lundump.h
                    3rdParty/LUA/lvm.h
                    3rdParty/LUA/lzio.h
                    Basics/Appendix.h
                    Basics/ArcBall.h
                    Basics/Checksums/crc32.h
                    Basics/Checksums/MD5.h
                    Basics/EndianConvert.h
                    Basics/EndianFile.h
                    Basics/GeometryGenerator.h
                    Basics/Grids.h
                    Basics/KDTree.h
                    Basics/LargeFile.h
                    Basics/LargeFileFD.h
                    Basics/LargeFileC.h
                    Basics/LargeRAWFile.h
                    Basics/LargeFileMMap.h
                    Basics/MathTools.h
                    Basics/MC.h
                    Basics/Mesh.h
                    Basics/PerfCounter.h
                    Basics/Plane.h
                    Basics/ProgressTimer.h
                    Basics/StdDefines.h
                    Basics/SysTools.h
                    Basics/SystemInfo.h
                    Basics/Timer.h
                    Basics/Clipper.h
                    Basics/TuvokException.h
                    Basics/Vectors.h
                    Basics/nonstd.h
                    Basics/AvgMinMaxTracker.h
                    Basics/MinMaxBlock.h
                    Basics/Threads.h
                    Controller/Controller.h
                    Controller/MasterController.h
                    DebugOut/AbstrDebugOut.h
                    DebugOut/ConsoleOut.h
                    DebugOut/MultiplexOut.h
                    DebugOut/TextfileOut.h
                    IO/3rdParty/bzip2/bzlib_private.h
                    IO/3rdParty/jpeglib/cderror.h
                    IO/3rdParty/jpeglib/cdjpeg.h
                    IO/3rdParty/jpeglib/jchuff.h
                    IO/3rdParty/jpeglib/jconfig.h
                    IO/3rdParty/jpeglib/jdct.h
                    IO/3rdParty/jpeglib/jdhuff.h
                    IO/3rdParty/jpeglib/jerror.h
                    IO/3rdParty/jpeglib/jinclude.h
                    IO/3rdParty/jpeglib/jlossls.h
                    IO/3rdParty/jpeglib/jlossy.h
                    IO/3rdParty/jpeglib/jmemsys.h
                    IO/3rdParty/jpeglib/jmorecfg.h
                    IO/3rdParty/jpeglib/jpegint.h
                    IO/3rdParty/jpeglib/jpeglib.h
                    IO/3rdParty/jpeglib/jversion.h
                    IO/3rdParty/jpeglib/mangle_jpeg.h
                    IO/3rdParty/jpeglib/transupp.h
                    IO/3rdParty/tiff/t4.h
                    IO/3rdParty/tiff/tif_dir.h
                    IO/3rdParty/tiff/tif_fax3.h
                    IO/3rdParty/tiff/tiffconf.h
                    IO/3rdParty/tiff/tiff.h
                    IO/3rdParty/tiff/tiffio.h
                    IO/3rdParty/tiff/tiffiop.h
                    IO/3rdParty/tiff/tiffvers.h
                    IO/3rdParty/tiff/tif_predict.h
                    IO/3rdParty/tiff/uvcode.h
                    IO/3rdParty/lzma/LzFind.h
                    IO/3rdParty/lzma/LzHash.h
                    IO/3rdParty/lzma/LzmaDec.h
                    IO/3rdParty/lzma/LzmaEnc.h
                    IO/3rdParty/lzma/Types.h
                    IO/3rdParty/lz4/lz4.h
                    IO/3rdParty/lz4/lz4hc.h
                    IO/const-brick-iterator.h
                    IO/AbstrConverter.h
                    IO/AmiraConverter.h
                    IO/AnalyzeConverter.h
                    IO/BMinMax.h
                    IO/BOVConverter.h
                    IO/Brick.h
                    IO/BrickedDataset.h
                    IO/BrickCache.h
                    IO/Dataset.h
                    IO/DICOM/DICOMParser.h
                    IO/DirectoryParser.h
                    IO/DSFactory.h
                    IO/DynamicBrickingDS.h
                    IO/FileBackedDataset.h
                    IO/GeomViewConverter.h
                    IO/gzio.h
                    IO/IASSConverter.h
                    IO/I3MConverter.h
                    IO/Images/ImageParser.h
                    IO/Images/StackExporter.h
                    IO/InveonConverter.h
                    IO/IOManager.h
                    IO/KeyValueFileParser.h
                    IO/LinearIndexDataset.h
                    IO/VGIHeaderParser.h
                    IO/NRRDConverter.h
                    IO/Quantize.h
                    IO/QVISConverter.h
                    IO/VGStudioConverter.h
                    IO/KitwareConverter.h
                    IO/RAWConverter.h
                    IO/REKConverter.h
                    IO/StkConverter.h
                    IO/MRCConverter.h
                    IO/TiffVolumeConverter.h
                    IO/AbstrGeoConverter.h
                    IO/LinesGeoConverter.h
                    IO/OBJGeoConverter.h
                    IO/PLYGeoConverter.h
                    IO/StLGeoConverter.h
                    IO/MobileGeoConverter.h
                    IO/G3D.h
                    IO/MedAlyVisGeoConverter.h
                    IO/MedAlyVisFiberTractGeoConverter.h
                    IO/TransferFunction1D.h
                    IO/TransferFunction2D.h
                    IO/TuvokIOError.h
                    IO/TuvokJPEG.h
                    IO/TuvokSizes.h
                    IO/Tuvok_QtPlugins.h
                    IO/uvfDataset.h
                    IO/uvfMesh.h
                    IO/VTKConverter.h
                    IO/exception/IOException.h
                    IO/UVF/DataBlock.h
                    IO/UVF/GlobalHeader.h
                    IO/UVF/Histogram1DDataBlock.h
                    IO/UVF/Histogram2DDataBlock.h
                    IO/UVF/KeyValuePairDataBlock.h
                    IO/UVF/MaxMinDataBlock.h
                    IO/UVF/RasterDataBlock.h
                    IO/UVF/GeometryDataBlock.h
                    IO/UVF/UVFBasic.h
                    IO/UVF/UVF.h
                    IO/UVF/UVFTables.h
                    IO/UVF/TOCBlock.h
                    IO/UVF/ExtendedOctree/ExtendedOctree.h
                    IO/UVF/ExtendedOctree/ExtendedOctreeConverter.h
                    IO/UVF/ExtendedOctree/VolumeTools.h
                    IO/UVF/ExtendedOctree/Hilbert.h
                    IO/UVF/ExtendedOctree/ZlibCompression.h
                    IO/UVF/ExtendedOctree/LzmaCompression.h
                    IO/UVF/ExtendedOctree/Lz4Compression.h
                    IO/UVF/ExtendedOctree/BzlibCompression.h
                    IO/TTIFFWriter/TTIFFWriter.h
                    IO/VariantArray.h
                    IO/VFFConverter.h
                    IO/XML3DGeoConverter.h
                    LuaScripting/LuaClassConstructor.h
                    LuaScripting/LuaClassInstance.h
                    LuaScripting/LuaClassRegistration.h
                    LuaScripting/LuaCommon.h
                    LuaScripting/LuaError.h
                    LuaScripting/LuaFunBinding.h
                    LuaScripting/LuaFunBindingCore.h
                    LuaScripting/LuaMemberReg.h
                    LuaScripting/LuaMemberRegUnsafe.h
                    LuaScripting/LuaProvenance.h
                    LuaScripting/LuaScripting.h
                    LuaScripting/LuaScriptingExecBody.h
                    LuaScripting/LuaScriptingExecHeader.h
                    LuaScripting/LuaStackRAII.h
                    LuaScripting/TuvokSpecific/LuaDatasetProxy.h
                    LuaScripting/TuvokSpecific/LuaIOManagerProxy.h
                    LuaScripting/TuvokSpecific/LuaTransferFun1DProxy.h
                    LuaScripting/TuvokSpecific/LuaTransferFun2DProxy.h
                    LuaScripting/TuvokSpecific/LuaTuvokTypes.h
                    LuaScripting/TuvokSpecific/MatrixMath.h
                    Renderer/AbstrRenderer.h
                    Renderer/Context.h
                    Renderer/ContextIdentification.h
                    Renderer/CullingLOD.h
                    Renderer/FrameCapture.h
                    Renderer/GL/GLCommon.h
                    Renderer/GL/GLContext.h
                    Renderer/GL/GLFrameCapture.h
                    Renderer/GL/GLGPURayTraverser.h
                    Renderer/GL/GLGridLeaper.h
                    Renderer/GL/GLHashTable.h
                    Renderer/GL/GLInclude.h
                    Renderer/GL/GLObject.h
                    Renderer/GL/GLRaycaster.h
                    Renderer/GL/GLRenderer.h
                    Renderer/GL/GLSBVR2D.h
                    Renderer/GL/GLSBVR.h
                    Renderer/GL/GLSLProgram.h
                    Renderer/GL/GLStateManager.h
                    Renderer/GL/GLTargetBinder.h
                    Renderer/GL/GLTexture1D.h
                    Renderer/GL/GLTexture2D.h
                    Renderer/GL/GLTexture3D.h
                    Renderer/GL/GLTexture.h
                    Renderer/GL/GLVBO.h
                    Renderer/GL/GLVolume2DTex.h
                    Renderer/GL/GLVolume3DTex.h
                    Renderer/GL/GLVolume.h
                    Renderer/GL/GLVolumePool.h
                    Renderer/GL/QtGLContext.h
                    Renderer/GL/RenderMeshGL.h
                    Renderer/GPUMemMan/GPUMemManDataStructs.h
                    Renderer/GPUMemMan/GPUMemMan.h
                    Renderer/GPUObject.h
                    Renderer/RenderMesh.h
                    Renderer/RenderRegion.h
                    Renderer/SBVRGeogen2D.h
                    Renderer/SBVRGeogen3D.h
                    Renderer/SBVRGeogen.h
                    Renderer/ShaderDescriptor.h
                    Renderer/StateManager.h
                    Renderer/TFScaling.h
                    Renderer/VisibilityState.h
                    Renderer/writebrick.h
                    )

set(TUVOK_SRCS 3rdParty/LUA/lapi.cpp
               3rdParty/LUA/lauxlib.cpp
               3rdParty/LUA/lbaselib.cpp
               3rdParty/LUA/lbitlib.cpp
               3rdParty/LUA/lcode.cpp
               3rdParty/LUA/lcorolib.cpp
               3rdParty/LUA/lctype.cpp
               3rdParty/LUA/ldblib.cpp
               3rdParty/LUA/ldebug.cpp
               3rdParty/LUA/ldo.cpp
               3rdParty/LUA/ldump.cpp
               3rdParty/LUA/lfunc.cpp
               3rdParty/LUA/lgc.cpp
               3rdParty/LUA/linit.cpp
               3rdParty/LUA/liolib.cpp
               3rdParty/LUA/llex.cpp
               3rdParty/LUA/lmathlib.cpp
               3rdParty/LUA/lmem.cpp
               3rdParty/LUA/loadlib.cpp
               3rdParty/LUA/lobject.cpp
               3rdParty/LUA/lopcodes.cpp
               3rdParty/LUA/loslib.cpp
               3rdParty/LUA/lparser.cpp
               3rdParty/LUA/lstate.cpp
               3rdParty/LUA/lstring.cpp
               3rdParty/LUA/lstrlib.cpp
               3rdParty/LUA/ltable.cpp
               3rdParty/LUA/ltablib.cpp
               3rdParty/LUA/ltm.cpp
               3rdParty/LUA/lundump.cpp
               3rdParty/LUA/lvm.cpp
               3rdParty/LUA/lzio.cpp
               Basics/Appendix.cpp
               Basics/ArcBall.cpp
               Basics/Checksums/MD5.cpp
               Basics/EndianFile.cpp
               Basics/GeometryGenerator.cpp
               Basics/LargeRAWFile.cpp
               Basics/LargeFileFD.cpp
               Basics/LargeFile.cpp
               Basics/LargeFileC.cpp
               Basics/LargeFileMMap.cpp
               Basics/MathTools.cpp
               Basics/MC.cpp
               Basics/Plane.cpp
               Basics/ProgressTimer.cpp
               Basics/SystemInfo.cpp
               Basics/Timer.cpp
               Basics/Clipper.cpp
               Basics/SysTools.cpp
               Basics/Mesh.cpp
               Basics/KDTree.cpp
               Basics/Threads.cpp
               Controller/MasterController.cpp
               DebugOut/AbstrDebugOut.cpp
               DebugOut/ConsoleOut.cpp
               DebugOut/MultiplexOut.cpp
               DebugOut/TextfileOut.cpp
               IO/3rdParty/bzip2/blocksort.c
               IO/3rdParty/bzip2/bzlib.c
               IO/3rdParty/bzip2/compress.c
               IO/3rdParty/bzip2/crctable.c
               IO/3rdParty/bzip2/decompress.c
               IO/3rdParty/bzip2/huffman.c
               IO/3rdParty/bzip2/randtable.c
               IO/3rdParty/jpeglib/cdjpeg.c
               IO/3rdParty/jpeglib/jcapimin.c
               IO/3rdParty/jpeglib/jcapistd.c
               IO/3rdParty/jpeglib/jccoefct.c
               IO/3rdParty/jpeglib/jccolor.c
               IO/3rdParty/jpeglib/jcdctmgr.c
               IO/3rdParty/jpeglib/jcdiffct.c
               IO/3rdParty/jpeglib/jchuff.c
               IO/3rdParty/jpeglib/jcinit.c
               IO/3rdParty/jpeglib/jclhuff.c
               IO/3rdParty/jpeglib/jclossls.c
               IO/3rdParty/jpeglib/jclossy.c
               IO/3rdParty/jpeglib/jcmainct.c
               IO/3rdParty/jpeglib/jcmarker.c
               IO/3rdParty/jpeglib/jcmaster.c
               IO/3rdParty/jpeglib/jcodec.c
               IO/3rdParty/jpeglib/jcomapi.c
               IO/3rdParty/jpeglib/jcparam.c
               IO/3rdParty/jpeglib/jcphuff.c
               IO/3rdParty/jpeglib/jcpred.c
               IO/3rdParty/jpeglib/jcprepct.c
               IO/3rdParty/jpeglib/jcsample.c
               IO/3rdParty/jpeglib/jcscale.c
               IO/3rdParty/jpeglib/jcshuff.c
               IO/3rdParty/jpeglib/jctrans.c
               IO/3rdParty/jpeglib/jdapimin.c
               IO/3rdParty/jpeglib/jdapistd.c
               IO/3rdParty/jpeglib/jdatadst.c
               IO/3rdParty/jpeglib/jdatasrc.c
               IO/3rdParty/jpeglib/jdcoefct.c
               IO/3rdParty/jpeglib/jdcolor.c
               IO/3rdParty/jpeglib/jddctmgr.c
               IO/3rdParty/jpeglib/jddiffct.c
               IO/3rdParty/jpeglib/jdhuff.c
               IO/3rdParty/jpeglib/jdinput.c
               IO/3rdParty/jpeglib/jdlhuff.c
               IO/3rdParty/jpeglib/jdlossls.c
               IO/3rdParty/jpeglib/jdlossy.c
               IO/3rdParty/jpeglib/jdmainct.c
               IO/3rdParty/jpeglib/jdmarker.c
               IO/3rdParty/jpeglib/jdmaster.c
               IO/3rdParty/jpeglib/jdmerge.c
               IO/3rdParty/jpeglib/jdphuff.c
               IO/3rdParty/jpeglib/jdpostct.c
               IO/3rdParty/jpeglib/jdpred.c
               IO/3rdParty/jpeglib/jdsample.c
               IO/3rdParty/jpeglib/jdscale.c
               IO/3rdParty/jpeglib/jdshuff.c
               IO/3rdParty/jpeglib/jdtrans.c
               IO/3rdParty/jpeglib/jerror.c
               IO/3rdParty/jpeglib/jfdctflt.c
               IO/3rdParty/jpeglib/jfdctfst.c
               IO/3rdParty/jpeglib/jfdctint.c
               IO/3rdParty/jpeglib/jidctflt.c
               IO/3rdParty/jpeglib/jidctfst.c
               IO/3rdParty/jpeglib/jidctint.c
               IO/3rdParty/jpeglib/jidctred.c
               IO/3rdParty/jpeglib/jmemmgr.c
               IO/3rdParty/jpeglib/jmemnobs.c
               IO/3rdParty/jpeglib/jquant1.c
               IO/3rdParty/jpeglib/jquant2.c
               IO/3rdParty/jpeglib/jutils.c
               IO/3rdParty/jpeglib/rdcolmap.c
               IO/3rdParty/jpeglib/rdswitch.c
               IO/3rdParty/jpeglib/transupp.c
               IO/3rdParty/tiff/tif_aux.c
               IO/3rdParty/tiff/tif_close.c
               IO/3rdParty/tiff/tif_codec.c
               IO/3rdParty/tiff/tif_color.c
               IO/3rdParty/tiff/tif_compress.c
               IO/3rdParty/tiff/tif_dir.c
               IO/3rdParty/tiff/tif_dirinfo.c
               IO/3rdParty/tiff/tif_dirread.c
               IO/3rdParty/tiff/tif_dirwrite.c
               IO/3rdParty/tiff/tif_dumpmode.c
               IO/3rdParty/tiff/tif_error.c
               IO/3rdParty/tiff/tif_extension.c
               IO/3rdParty/tiff/tif_fax3.c
               IO/3rdParty/tiff/tif_fax3sm.c
               IO/3rdParty/tiff/tif_flush.c
               IO/3rdParty/tiff/tif_getimage.c
               IO/3rdParty/tiff/tif_luv.c
               IO/3rdParty/tiff/tif_lzw.c
               IO/3rdParty/tiff/tif_next.c
               IO/3rdParty/tiff/tif_open.c
               IO/3rdParty/tiff/tif_packbits.c
               IO/3rdParty/tiff/tif_pixarlog.c
               IO/3rdParty/tiff/tif_predict.c
               IO/3rdParty/tiff/tif_print.c
               IO/3rdParty/tiff/tif_read.c
               IO/3rdParty/tiff/tif_strip.c
               IO/3rdParty/tiff/tif_swab.c
               IO/3rdParty/tiff/tif_thunder.c
               IO/3rdParty/tiff/tif_tile.c
               IO/3rdParty/tiff/tif_version.c
               IO/3rdParty/tiff/tif_warning.c
               IO/3rdParty/tiff/tif_write.c
               IO/3rdParty/tiff/tif_zip.c
               IO/3rdParty/tiff/tif_jpeg.c
               IO/3rdParty/lzma/LzFind.c
               IO/3rdParty/lzma/LzmaDec.c
               IO/3rdParty/lzma/LzmaEnc.c
               IO/3rdParty/lz4/lz4.c
               IO/3rdParty/lz4/lz4hc.c
               IO/const-brick-iterator.cpp
               IO/AbstrConverter.cpp
               IO/AmiraConverter.cpp
               IO/AnalyzeConverter.cpp
               IO/BMinMax.cpp
               IO/BOVConverter.cpp
               IO/BrickedDataset.cpp
               IO/BrickCache.cpp
               IO/Dataset.cpp
               IO/DICOM/DICOMParser.cpp
               IO/DirectoryParser.cpp
               IO/DynamicBrickingDS.cpp
               IO/DSFactory.cpp
               IO/FileBackedDataset.cpp
               IO/GeomViewConverter.cpp
               IO/gzio.c
               IO/IASSConverter.cpp
               IO/I3MConverter.cpp
               IO/Images/ImageParser.cpp
               IO/Images/StackExporter.cpp
               IO/InveonConverter.cpp
               IO/IOManager.cpp
               IO/KeyValueFileParser.cpp
               IO/LinearIndexDataset.cpp
               IO/VGIHeaderParser.cpp
               IO/NRRDConverter.cpp
               IO/QVISConverter.cpp
               IO/VGStudioConverter.cpp
               IO/KitwareConverter.cpp
               IO/RAWConverter.cpp
               IO/REKConverter.cpp
               IO/StkConverter.cpp
               IO/MRCConverter.cpp
               IO/TiffVolumeConverter.cpp
               IO/AbstrGeoConverter.cpp
               IO/LinesGeoConverter.cpp
               IO/OBJGeoConverter.cpp
               IO/PLYGeoConverter.cpp
               IO/StLGeoConverter.cpp
               IO/MobileGeoConverter.cpp
               IO/G3D.cpp
               IO/MedAlyVisGeoConverter.cpp
               IO/MedAlyVisFiberTractGeoConverter.cpp
               IO/TransferFunction1D.cpp
               IO/TransferFunction2D.cpp
               IO/TuvokJPEG.cpp
               IO/uvfDataset.cpp
               IO/uvfMesh.cpp
               IO/VTKConverter.cpp
               IO/UVF/DataBlock.cpp
               IO/UVF/GlobalHeader.cpp
               IO/UVF/Histogram1DDataBlock.cpp
               IO/UVF/Histogram2DDataBlock.cpp
               IO/UVF/KeyValuePairDataBlock.cpp
               IO/UVF/MaxMinDataBlock.cpp
               IO/UVF/RasterDataBlock.cpp
               IO/UVF/GeometryDataBlock.cpp
               IO/UVF/UVF.cpp
               IO/UVF/UVFTables.cpp
               IO/UVF/TOCBlock.cpp
               IO/UVF/ExtendedOctree/ExtendedOctree.cpp
               IO/UVF/ExtendedOctree/ExtendedOctreeConverter.cpp
               IO/UVF/ExtendedOctree/VolumeTools.cpp
               IO/UVF/ExtendedOctree/ZlibCompression.cpp
               IO/UVF/ExtendedOctree/LzmaCompression.cpp
               IO/UVF/ExtendedOctree/Lz4Compression.cpp
               IO/UVF/ExtendedOctree/BzlibCompression.cpp
               IO/TTIFFWriter/TTIFFWriter.cpp
               IO/VariantArray.cpp
               IO/VFFConverter.cpp
               IO/XML3DGeoConverter.cpp
               LuaScripting/LuaClassConstructor.cpp
               LuaScripting/LuaClassInstance.cpp
               LuaScripting/LuaClassRegistration.cpp
               LuaScripting/LuaMemberReg.cpp
               LuaScripting/LuaMemberRegUnsafe.cpp
               LuaScripting/LuaProvenance.cpp
               LuaScripting/LuaScripting.cpp
               LuaScripting/LuaStackRAII.cpp
               LuaScripting/TuvokSpecific/LuaDatasetProxy.cpp
               LuaScripting/TuvokSpecific/LuaIOManagerProxy.cpp
               LuaScripting/TuvokSpecific/LuaTransferFun1DProxy.cpp
               LuaScripting/TuvokSpecific/LuaTransferFun2DProxy.cpp
               LuaScripting/TuvokSpecific/LuaTuvokTypes.cpp
               LuaScripting/TuvokSpecific/MatrixMath.cpp
               Renderer/AbstrRenderer.cpp
               Renderer/Context.cpp
               Renderer/CullingLOD.cpp
               Renderer/GL/GLCommon.cpp
               Renderer/GL/GLFBOTex.cpp
               Renderer/GL/GLFrameCapture.cpp
               Renderer/GL/GLGPURayTraverser.cpp
               Renderer/GL/GLGridLeaper.cpp
               Renderer/GL/GLHashTable.cpp
               Renderer/GL/GLRaycaster.cpp
               Renderer/GL/GLRenderer.cpp
               Renderer/GL/GLSBVR2D.cpp
               Renderer/GL/GLSBVR.cpp
               Renderer/GL/GLSLProgram.cpp
               Renderer/GL/GLStateManager.cpp
               Renderer/GL/GLTargetBinder.cpp
               Renderer/GL/GLTexture1D.cpp
               Renderer/GL/GLTexture2D.cpp
               Renderer/GL/GLTexture3D.cpp
               Renderer/GL/GLTexture.cpp
               Renderer/GL/GLVBO.cpp
               Renderer/GL/GLVolume2DTex.cpp
               Renderer/GL/GLVolume3DTex.cpp
               Renderer/GL/GLVolume.cpp
               Renderer/GL/GLVolumePool.cpp
               Renderer/GL/RenderMeshGL.cpp
               Renderer/GPUMemMan/GPUMemMan.cpp
               Renderer/GPUMemMan/GPUMemManDataStructs.cpp
               Renderer/RenderMesh.cpp
               Renderer/RenderRegion.cpp
               Renderer/SBVRGeogen2D.cpp
               Renderer/SBVRGeogen3D.cpp
               Renderer/SBVRGeogen.cpp
               Renderer/ShaderDescriptor.cpp
               Renderer/TFScaling.cpp
               Renderer/VisibilityState.cpp
               IO/3rdParty/tiff/tif_unix.c
               )
