/******************************************************************************
#   Copyright 2006 Hermann Seib
#   Copyright 2010 Raphaël François
#
#    This file is part of VstBoard.
#
#    VstBoard is free software: you can redistribute it and/or modify
#    it under the terms of the under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    VstBoard is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    under the terms of the GNU Lesser General Public License for more details.
#
#    You should have received a copy of the under the terms of the GNU Lesser General Public License
#    along with VstBoard.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#ifndef VSTBANK_H
#define VSTBANK_H

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning (disable: 4100)
#endif
#include "vstbankbase.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

namespace vst
{

    /*****************************************************************************/
    /* CFxBank : class for an Fx Bank file                                       */
    /*****************************************************************************/

    class CFxBank : public CFxBase
    {
    public:
        CFxBank(std::string *pszFile = 0);
        CFxBank(int nPrograms, int nParams);
        CFxBank(int nChunkSize);
        CFxBank(CFxBank const &org) { DoCopy(org); }
        virtual ~CFxBank();
        CFxBank & operator=(CFxBank const &org) { return DoCopy(org); }
    public:
        bool SetSize(int nPrograms, int nParams);
        bool SetSize(int nChunkSize);
        static VstInt32 PluginIdFromBank(std::string *pszFile);
        bool LoadBank(std::string *pszFile);
        bool SaveBank(std::string *pszFile);
        void Unload();
        bool IsLoaded() { return !!bBank; }
        bool IsChunk() { return bChunk; }

        // access functions
    public:
        long GetVersion() { if (!bBank) return 0; return ((SFxBase*)bBank)->version; }
        long GetFxID() { if (!bBank) return 0; return ((SFxBase*)bBank)->fxID; }
        void SetFxID(long id) { if (bBank) ((SFxBase*)bBank)->fxID = id; if (!bChunk) for (int i = GetNumPrograms() -1; i >= 0; i--) GetProgram(i)->fxID = id; }
        long GetFxVersion() { if (!bBank) return 0; return ((SFxBase*)bBank)->fxVersion; }
        void SetFxVersion(long v) { if (bBank) ((SFxBase*)bBank)->fxVersion = v; if (!bChunk) for (int i = GetNumPrograms() -1; i >= 0; i--) GetProgram(i)->fxVersion = v; }
        long GetNumPrograms() { if (!bBank) return 0; return ((SFxBankBase*)bBank)->numPrograms; }
        long GetNumParams() { if (bChunk) return 0; return GetProgram(0)->numParams; }
        long GetChunkSize() { if (!bChunk) return 0; return ((SFxBankChunk *)bBank)->size; }
        void *GetChunk() { if (!bChunk) return 0; return ((SFxBankChunk *)bBank)->chunk; }
        bool SetChunk(void *chunk) { if (!bChunk) return false; memcpy(((SFxBankChunk *)bBank)->chunk, chunk, ((SFxBankChunk *)bBank)->size); return true; }

        SFxProgram * GetProgram(int nProgNum);

        char * GetProgramName(int nProgram);
        void SetProgramName(int nProgram, char *name);
        float GetProgParm(int nProgram, int nParm);
        bool SetProgParm(int nProgram, int nParm, float val = 0.0);

    protected:
        std::string szFileName;
        unsigned char * bBank;
        int nBankLen;
        bool bChunk;

    protected:
        void Init();
        CFxBank & DoCopy(CFxBank const &org);
    };



}

#endif // VSTBANK_H
