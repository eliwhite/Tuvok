/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2011 Scientific Computing and Imaging Institute,
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
  \file    StateManager.h
  \author  Jens Krueger
           IVDA, Saarbruecken
           SCI Institute, University of Utah
*/
#pragma once

#ifndef STATEMANAGER_H
#define STATEMANAGER_H

#include "StdTuvokDefines.h"

#ifdef _MSC_VER
# include <memory>
#else
# include <tr1/memory>
#endif

namespace tuvok {

  #define StateLightCount 1
  #define StateTUCount 4

  enum STATE_CULL {
    CULL_FRONT,
    CULL_BACK
  };

  enum STATE_TEX {
    TEX_1D,
    TEX_2D,
    TEX_3D,
    TEX_UNKNOWN
  };

  enum BLEND_FUNC {
    BF_ZERO,
    BF_ONE,
    BF_SRC_COLOR,
    BF_ONE_MINUS_SRC_COLOR,
    BF_DST_COLOR,
    BF_ONE_MINUS_DST_COLOR,
    BF_SRC_ALPHA,
    BF_ONE_MINUS_SRC_ALPHA,
    BF_DST_ALPHA,
    BF_ONE_MINUS_DST_ALPHA,
    BF_SRC_ALPHA_SATURATE
  };

  enum BLEND_EQUATION {
    BE_FUNC_ADD,
    BE_FUNC_SUBTRACT,
    BE_FUNC_REVERSE_SUBTRACT,
    BE_MIN,
    BE_MAX
  };

  class GLStateManager;
  class StateManager;

  /** \class GPUState
   * Base class for all GPU state this object holds
     the rendering pipleine's state in one single object
     to the GPU.*/
  class GPUState {
    public:
      GPUState() :
        enableDepth(true),
        enableCull(true),
        cullState(CULL_BACK),
        enableBlend(false),
        enableScissor(false),
        enableLighting(false),
        enableColorMaterial(false),
        enableLineSmooth(false),
        activeTexUnit(0),
        depthMask(true),
        colorMask(true),
        blendEquation(BE_FUNC_ADD),
        blendFuncSrc(BF_ONE_MINUS_DST_ALPHA),
        blendFuncDst(BF_ONE)
      {
        for (size_t i = 0;i<StateLightCount;i++) enableLight[i] = false;
        for (size_t i = 0;i<StateTUCount;i++) enableTex[i] = TEX_UNKNOWN;
      }
      virtual ~GPUState() {}

      virtual void SetEnableDepth(const bool& value, bool bForce=false) = 0;
      virtual void SetEnableCull(const bool& value, bool bForce=false) = 0;
      virtual void SetCullState(const STATE_CULL& value, bool bForce=false) = 0;
      virtual void SetEnableBlend(const bool& value, bool bForce=false) = 0;
      virtual void SetEnableScissor(const bool& value, bool bForce=false) = 0;
      virtual void SetEnableLighting(const bool& value, bool bForce=false) = 0;
      virtual void SetEnableColorMaterial(const bool& value, bool bForce=false) = 0;
      virtual void SetEnableLineSmooth(const bool& value, bool bForce=false) = 0;
      virtual void SetEnableLight(size_t i, const bool& value, bool bForce=false) = 0;
      virtual void SetEnableTex(size_t i, const STATE_TEX& value, bool bForce=false) = 0;
      virtual void SetActiveTexUnit(const size_t iUnit, bool bForce=false) = 0;
      virtual void SetDepthMask(const bool value, bool bForce=false) = 0;
      virtual void SetColorMask(const bool value, bool bForce=false) = 0;
      virtual void SetBlendEquation(const BLEND_EQUATION value, bool bForce=false) = 0;
      virtual void SetBlendFunction(const BLEND_FUNC src, const BLEND_FUNC dest, bool bForce=false) = 0;
      

      bool GetEnableDepth() const {return enableDepth;}
      bool GetEnableCull() const {return enableCull;}
      STATE_CULL GetCullState() const {return cullState;}
      bool GetEnableBlend() const {return enableBlend;}
      bool GetEnableScissor() const {return enableScissor;}
      bool GetEnableLighting() const {return enableLighting;}
      bool GetEnableColorMaterial() const {return enableColorMaterial;}
      bool GetEnableLineSmooth() const {return enableLineSmooth;}
      bool GetEnableLight(size_t i) const {return enableLight[i];}
      STATE_TEX GetEnableTex(size_t i) const {return enableTex[i];}
      size_t GetActiveTexUnit() const {return activeTexUnit;}
      bool GetDepthMask() const {return depthMask;}
      bool GetColorMask() const {return colorMask;}
      BLEND_EQUATION GetBlendEquation() const {return blendEquation;}
      BLEND_FUNC GetBlendFunctionSrc() const {return blendFuncSrc;}
      BLEND_FUNC GetBlendFunctionDst() const {return blendFuncDst;}

    protected:
      bool enableDepth;
      bool enableCull;
      STATE_CULL cullState;
      bool enableBlend;
      bool enableScissor;
      bool enableLighting;
      bool enableLight[StateLightCount];
      bool enableColorMaterial;
      bool enableLineSmooth;
      STATE_TEX enableTex[StateTUCount];
      size_t activeTexUnit;
      bool depthMask;
      bool colorMask;
      BLEND_EQUATION blendEquation;
      BLEND_FUNC blendFuncSrc;
      BLEND_FUNC blendFuncDst;


    private:
      friend class StateManager;
      friend class GLStateManager;

      virtual void Apply() = 0;
      virtual void Apply(const GPUState& state, bool bForce) = 0;


  };

  /** \class StateManager
   * Base class for all GPU state managers.
     A state manager applies is state object's properties
     to the GPU.*/
  class StateManager {
    public:
      StateManager() : m_InternalState(0) {}
      virtual ~StateManager() {}

      /** Applies a given state to GPU pipleine this manager
       *  is associated with
       * @param state the GPU state to be applied 
       * @param bForce apply the entire state even if that results in redundant state changes */
      void Apply(const GPUState& state, bool bForce=false) {
        m_InternalState->Apply(state, bForce);
      }

      virtual const GPUState* GetCurrentState() const {return m_InternalState;}
      virtual GPUState* ChangeCurrentState() {return m_InternalState;}

    protected:
       GPUState* m_InternalState;
  };

  typedef std::tr1::shared_ptr<StateManager> StateManagerPtr; 

}; //namespace tuvok

#endif // STATEMANAGER_H
