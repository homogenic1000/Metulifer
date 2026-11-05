#pragma once

#include <JuceHeader.h>
#include "Voice.h"

namespace ModMatrix
{
    enum Source
    {
        srcLFO1 = 0,
        srcLFO2,
        srcEnv1,
        srcEnv2,
        srcKeyPitch,
        srcVelocity,
        numSources
    };

    enum Destination
    {
        dstCutoff = 0,
        dstPitch1,
        dstPitch2,
        dstWave1,
        dstWave2,
        dstMix1,
        dstMix2,
        dstDecay1,
        dstDecay2,
        dstReverbMix,
        dstDelayTime,
        dstDelayFeedback,
        dstReduxBits,
        dstReduxRate,
        dstLfo2Rate,
        numDestinations
    };

    static inline const char* getSourceName (int s) noexcept
    {
        switch (s)
        {
            case srcLFO1:     return "LFO 1";
            case srcLFO2:     return "LFO 2";
            case srcEnv1:     return "ADSR 1";
            case srcEnv2:     return "ADSR 2";
            case srcKeyPitch: return "Key/Pitch";
            case srcVelocity: return "Velocity";
            default:          return "";
        }
    }

    static inline const char* getDestName (int d) noexcept
    {
        switch (d)
        {
            case dstCutoff:        return "VCF Cutoff";
            case dstPitch1:        return "VCO 1 Pitch";
            case dstPitch2:        return "VCO 2 Pitch";
            case dstWave1:         return "VCO 1 Wave";
            case dstWave2:         return "VCO 2 Wave";
            case dstMix1:          return "Mix VCO 1";
            case dstMix2:          return "Mix VCO 2";
            case dstDecay1:        return "ADSR 1 Decay";
            case dstDecay2:        return "ADSR 2 Decay";
            case dstReverbMix:     return "Reverb Mix";
            case dstDelayTime:     return "Delay Time";
            case dstDelayFeedback: return "Delay Feedback";
            case dstReduxBits:     return "Redux Bits";
            case dstReduxRate:     return "Redux Rate";
            case dstLfo2Rate:      return "LFO 2 Rate";
            default:               return "";
        }
    }
}
