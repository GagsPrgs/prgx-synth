#pragma once

#include <algorithm>
#include <cmath>

// Dvopolni niskopropusni filtar: propušta niske tonove, a više utišava.
// Izveden je kao "state variable" filtar (TPT izvedba), koji ostaje stabilan i kad se cutoff brzo mijenja.
// Nema alokacije ni zaključavanja, pa je siguran za audio nit.
class LowPassFilter
{
public:
    void reset() noexcept
    {
        state1 = 0.0;
        state2 = 0.0;
    }

    // cutoffHz: od koje se frekvencije visoki tonovi počinju utišavati.
    // resonance: od 0 do 1, koliko se tonovi oko cutoffa naglase (0 = bez isticanja).
    void setParameters (double cutoffHz, double resonance, double sampleRate) noexcept
    {
        const double g = std::tan (pi * limitCutoff (cutoffHz, sampleRate) / sampleRate);
        const double k = 1.0 / qFor (resonance);

        a1 = 1.0 / (1.0 + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
        outputGain = gainFor (resonance);
    }

    float process (float input) noexcept
    {
        const double v3 = input - state2;
        const double v1 = a1 * state1 + a2 * v3;
        const double v2 = state2 + a2 * state1 + a3 * v3;

        state1 = 2.0 * v1 - state1;
        state2 = 2.0 * v2 - state2;

        return static_cast<float> (v2 * outputGain);
    }

    // Filtar je "posve otvoren" kad je cutoff na vrhu raspona, a rezonancije nema. Tada ga zaobilazimo:
    // filtar blizu Nyquista inače malo mijenja oblik vala (ne i boju), pa zadani zvuk ostaje isti kao bez filtra.
    static bool isOpen (double cutoffHz, double resonance) noexcept
    {
        return cutoffHz >= 19990.0 && resonance <= 0.0;
    }

    // Koliko filtar pojačava (više od 1) ili utišava (manje od 1) zadanu frekvenciju.
    // Služi za crtež odziva filtra u sučelju, a isti račun se koristi i u testovima.
    static double magnitudeAt (double frequencyHz, double cutoffHz, double resonance, double sampleRate) noexcept
    {
        const double x = std::tan (pi * std::min (frequencyHz, 0.499 * sampleRate) / sampleRate)
                         / std::tan (pi * limitCutoff (cutoffHz, sampleRate) / sampleRate);
        const double k = 1.0 / qFor (resonance);

        return gainFor (resonance) / std::sqrt ((1.0 - x * x) * (1.0 - x * x) + k * k * x * x);
    }

private:
    // Cutoff ne smije prići polovici brzine uzorkovanja (Nyquist), tamo filtar gubi smisao.
    static double limitCutoff (double cutoffHz, double sampleRate) noexcept { return std::min (cutoffHz, 0.45 * sampleRate); }

    // Q raste eksponencijalno s gumbom, pa je gumb ravnomjeran na sluh.
    static double qFor (double resonance) noexcept { return minQ * std::pow (maxQ / minQ, resonance); }

    // Što je rezonancija jača, to je cijeli izlaz malo tiši, da isticanje ne postane preglasno.
    static double gainFor (double resonance) noexcept { return 1.0 / (1.0 + resonance); }

    static constexpr double pi = 3.14159265358979323846;
    static constexpr double minQ = 0.7071067811865476;  // bez isticanja (ravan odziv)
    static constexpr double maxQ = 8.0;                 // najjače isticanje, ali bez samooscilacije

    double a1 = 0.0, a2 = 0.0, a3 = 0.0;
    double state1 = 0.0, state2 = 0.0;
    double outputGain = 1.0;
};
