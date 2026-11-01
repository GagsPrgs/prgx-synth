# PRGX Synth — pravila projekta

VST3 synth plugin (instrument) za Windows, testira se u FL Studiju.
Napravljen u JUCE-u (verzija fiksirana u `CMakeLists.txt`, preuzima se preko FetchContent) i CMake-u.

Vlasnik projekta je početnik u programiranju: objašnjavaj jednostavnim jezikom, na hrvatskom,
i za svaku promjenu reci koje datoteke mijenjaš i zašto.

## Pravila za audio nit (`processBlock` i sve što se iz njega poziva)

Audio nit mora završiti posao unutar nekoliko milisekundi, svaki put. Zato je u njoj **zabranjeno**:

- **Alocirati memoriju**: nema `new`/`delete`, `malloc`, `std::make_unique`, rasta
  `std::vector`/`juce::Array` (`push_back`, `resize`), stvaranja `juce::String` ni `std::string`.
  Sve buffere alociraj unaprijed u `prepareToPlay` ili u konstruktoru.
- **Zaključavati**: nema `std::mutex`, `juce::CriticalSection`, `ScopedLock`, čekanja na druge niti.
  Za razmjenu podataka koristi `std::atomic` ili lock-free FIFO (`juce::AbstractFifo`).
- **Čitati ili pisati datoteke**, i općenito ništa što ide na disk, mrežu ili operacijski sustav
  (uključujući `DBG`/logiranje, `std::cout`, otvaranje prozora).

Također: parametre čitaj kroz `std::atomic<float>*` dobiven jednom u konstruktoru
(`getRawParameterValue`), ne preko traženja po imenu u `processBlock`.

## Etape

Radimo jednu etapu po redu. Ne prelazi na sljedeću dok vlasnik ne potvrdi da trenutna radi u FL Studiju.
Git commit radi tek nakon te potvrde.

1. **Prazan instrument** s jednim gumbom za glasnoću koji se učita u FL Studio. ← trenutna
2. **Oscilator i MIDI**: sviranje nota (sinus/pila), polifonija preko `juce::Synthesiser`.
3. **Ovojnica (ADSR)**: attack, decay, sustain, release.
4. **Filtar**: niskopropusni filtar s cutoff i resonance gumbima.
5. **Modulacija**: LFO i odabir valnog oblika oscilatora.
6. **Preseti i dorada sučelja**: spremanje/učitavanje preseta, ljepši izgled.

(Etape 2–6 su prijedlog i mogu se mijenjati.)

## Izgradnja

```
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Rezultat: `build/PrgxSynth_artefacts/Release/VST3/PRGX Synth.vst3` (to je mapa, kopira se cijela).
Detalji i kopiranje u FL Studio: `README.md`.

## Struktura

- `CMakeLists.txt`: postavke projekta i plugina
- `Source/PluginProcessor.*`: zvuk i parametri (audio nit)
- `Source/PluginEditor.*`: sučelje (glavna nit)
