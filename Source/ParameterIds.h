#pragma once

// Imena (ID-evi) svih parametara na jednom mjestu, da se ne pogriješi pri tipkanju.
// Pazi: ID se ne smije mijenjati kad postoje spremljeni projekti, jer FL Studio po njemu pamti postavke.
namespace ParameterIds
{
    inline constexpr auto waveform = "waveform";
    inline constexpr auto bendRange = "bendrange";
    inline constexpr auto cutoff = "cutoff";
    inline constexpr auto resonance = "resonance";
    inline constexpr auto attack = "attack";
    inline constexpr auto decay = "decay";
    inline constexpr auto sustain = "sustain";
    inline constexpr auto release = "release";
    inline constexpr auto gain = "gain";
}
