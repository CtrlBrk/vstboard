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

#include "vstprogram.h"

using namespace vst;

CFxProgram::CFxProgram(std::string *pszFile)
{
    Init();
    if (pszFile)
        LoadProgram(pszFile);
}

CFxProgram::~CFxProgram()
{
    Unload();
}

SFxProgram * CFxProgram::GetProgram()
{
    if ((!IsLoaded()) || (bChunk))
        return 0;

    return (SFxProgram *)bProgram;
}

char * CFxProgram::GetProgramName()
{
    SFxProgram* p =(SFxProgram*)bProgram;
    if (!p)
        return 0;

    return p->prgName;
}

void CFxProgram::SetProgramName(char *name)
{
    SFxProgram* p =(SFxProgram*)bProgram;
    if (!p)
        return;

    strncpy(p->prgName, name, sizeof(p->prgName));
    p->prgName[sizeof(p->prgName)-1] = '\0';

}

float CFxProgram::GetProgParm(int nParm)
{
    SFxProgram* p =(SFxProgram*)bProgram;
    if (!p || nParm > p->numParams)
        return 0;

#ifndef chunkGlobalMagic                /* VST SDK 2.4 rev2?                 */
    return p->content.params[nParm];
#else
    return p->params[nParm];
#endif
}

bool CFxProgram::SetProgParm(int nParm, float val)
{
    SFxProgram* p =(SFxProgram*)bProgram;
    if (!p || nParm > p->numParams)
        return false;

    if (val < 0.0) val = 0.0;
    if (val > 1.0) val = 1.0;

#ifndef chunkGlobalMagic                /* VST SDK 2.4 rev2?                 */
    p->content.params[nParm] = val;
#else
    p->params[nParm] = val;
#endif

    return true;
}

bool CFxProgram::SetSize(int nParams)
{
    int nTotLen = sizeof(SFxProgramBase) + nParams * sizeof(float);
    unsigned char *nProg = new unsigned char[nTotLen];
    if (!nProg)
        return false;

    Unload();
    bProgram = nProg;
    nProgLen = nTotLen;
    bChunk = false;

    memset(nProg, 0, nTotLen);
    SFxProgram *pProg = (SFxProgram *)bProgram;
    pProg->chunkMagic = cMagic;
    pProg->byteSize = nTotLen-8;
    pProg->fxMagic = fMagic;
    pProg->version = MyVersion;
    pProg->numParams = nParams;
    for (int i = 0; i < nParams; i++)
        pProg->params[i] = 0.f;

    return true;
}

bool CFxProgram::SetChunkSize(int nChunkSize)
{
    int nTotLen = ((int)((SFxProgramChunk*)0)->chunk) + nChunkSize;
    unsigned char *nProg = new unsigned char[nTotLen];
    if (!nProg)
        return false;

    Unload();
    bProgram = nProg;
    nProgLen = nTotLen;
    bChunk = true;

    memset(nProg, 0, nTotLen);
    SFxProgramChunk *pProg = (SFxProgramChunk *)bProgram;
    pProg->chunkMagic = cMagic;
    pProg->fxMagic = chunkPresetMagic;
    pProg->version = MyVersion;
    pProg->numParams = 0;
    pProg->size = nChunkSize;
    pProg->byteSize = nTotLen;

    return true;
}

bool CFxProgram::LoadProgram(std::string *pszFile)
{
    FILE *fp = fopen(pszFile->c_str(), "rb");
    if (!fp)
        return false;
    bool brc = true;
    unsigned char *nProg = NULL;
    try
    {
        fseek(fp, 0, SEEK_END);
        size_t tLen = (size_t)ftell(fp);
        rewind(fp);
        nProg = new unsigned char[tLen];
        if (!nProg)
            throw (int)1;
        if (fread(nProg, 1, tLen, fp) != tLen)
            throw (int)1;

        SFxProgram * pProg = (SFxProgram*)nProg;

        if (NeedsBSwap) {
            SwapBytes(pProg->chunkMagic);
            SwapBytes(pProg->byteSize);
            SwapBytes(pProg->fxMagic);
            SwapBytes(pProg->version);
            SwapBytes(pProg->fxID);
            SwapBytes(pProg->fxVersion);
            SwapBytes(pProg->numParams);
        }

        if ((pProg->chunkMagic != cMagic) ||
            (pProg->version > MyVersion) ||
            ((pProg->fxMagic != fMagic) &&
             (pProg->fxMagic != chunkPresetMagic)))
            throw (int)1;

        if (pProg->fxMagic == fMagic) {
            if (NeedsBSwap) {
                for (int i = 0; i < pProg->numParams; i++)
                    SwapBytes(pProg->params[i]);
            }
        }
        else if (pProg->fxMagic == chunkPresetMagic)
        {
            if (NeedsBSwap)                     /* eventually swap necessary bytes   */
            {
                SFxProgramChunk *pChunk = (SFxProgramChunk *)pProg;
                SwapBytes(pChunk->size);
                /* size check - must not be too large*/
                //if (pChunk->size + ((size_t)((SFxBankChunk *)0)->chunk) > tLen)
                //    throw (int)1;
            }
        }


        Unload();
        bProgram = nProg;
        nProgLen = (int)tLen;
        bChunk = (pProg->fxMagic == chunkPresetMagic);

    }
    catch(...)
    {
        brc = false;
        if(nProg)
            delete[] nProg;
    }
    fclose(fp);
    return brc;
}

bool CFxProgram::SaveProgram(std::string *pszFile)
{
    if (!IsLoaded())
        return false;
    unsigned char *nProg = new unsigned char[nProgLen];
    if (!nProg)
        return false;
    memcpy(nProg, bProgram, nProgLen);

    SFxProgram * pProg = (SFxProgram*)nProg;
    int numParams = pProg->numParams;

    if (NeedsBSwap)
    {
        SwapBytes(pProg->chunkMagic);
        SwapBytes(pProg->byteSize);
        SwapBytes(pProg->fxMagic);
        SwapBytes(pProg->version);
        SwapBytes(pProg->fxID);
        SwapBytes(pProg->fxVersion);
        SwapBytes(pProg->numParams);
        for (int j = 0; j < numParams; j++)
            SwapBytes(pProg->params[j]);
    }
    if (bChunk)
    {
        if (NeedsBSwap)                       /* if byte-swapping needed           */
            SwapBytes(((SFxProgramChunk*)pProg)->size);
    }

    bool brc = true;
    FILE *fp = NULL;
    try
    {
        fp = fopen(pszFile->c_str(), "wb");
        if (!fp)
            throw (int)1;
        if (fwrite(nProg, 1, nProgLen, fp) != (size_t)nProgLen)
            throw (int)1;
    }
    catch(...)
    {
        brc = false;
    }
    if (fp)
        fclose(fp);
    delete[] nProg;

    return brc;
}

void CFxProgram::Init()
{
    bProgram = NULL;
    Unload();
}

void CFxProgram::Unload()
{
    if(bProgram)
        delete[] bProgram;
    bProgram = 0;
    nProgLen = 0;
    bChunk = false;
}
