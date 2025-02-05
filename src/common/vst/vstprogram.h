#ifndef VSTPROGRAM_H
#define VSTPROGRAM_H

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
    /* CFxProgram : class for an Fx Program file                                 */
    /*****************************************************************************/

    class CFxProgram : public CFxBase
    {
    public:
        CFxProgram(std::string *pszFile = 0);
        virtual ~CFxProgram();
        bool LoadProgram(std::string *pszFile);
        bool SaveProgram(std::string *pszFile);
        void Unload();
        bool IsLoaded() { return !!bProgram; }
        bool IsChunk() { return bChunk; }

        bool SetSize(int nParams);
        bool SetChunkSize(int nChunkSize);

        long GetVersion() { if (!bProgram) return 0; return ((SFxBase*)bProgram)->version; }
        long GetFxID() { if (!bProgram) return 0; return ((SFxBase*)bProgram)->fxID; }
        void SetFxID(long id) { if (bProgram) ((SFxBase*)bProgram)->fxID = id; if (!bChunk) GetProgram()->fxID = id; }
        long GetFxVersion() { if (!bProgram) return 0; return ((SFxBase*)bProgram)->fxVersion; }
        void SetFxVersion(long v) { if (bProgram) ((SFxBase*)bProgram)->fxVersion = v; if (!bChunk) GetProgram()->fxVersion = v; }
        long GetNumParams() { if (bProgram) return 0; return GetProgram()->numParams; }
        long GetChunkSize() { if (!bProgram) return 0; return ((SFxProgramChunk *)bProgram)->size; }
        void *GetChunk() { if (!bProgram) return 0; return ((SFxProgramChunk *)bProgram)->chunk; }
        bool SetChunk(void *chunk) { if (!bChunk) return false; memcpy(((SFxProgramChunk *)bProgram)->chunk, chunk, ((SFxProgramChunk *)bProgram)->size); return true; }

        SFxProgram * GetProgram();
        char * GetProgramName();
        void SetProgramName(char *name);
        float GetProgParm(int nParm);
        bool SetProgParm(int nParm, float val = 0.0);

    protected:
        void Init();
        unsigned char * bProgram;
        int nProgLen;
        bool bChunk;

    };
}

#endif // VSTPROGRAM_H
