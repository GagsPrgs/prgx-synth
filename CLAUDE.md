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

**Poznata iznimka:** `juce::Synthesiser` interno drži `CriticalSection` dok renderira. To je u redu
jer nitko drugi ne dira taj lock: glasove i zvukove dodajemo samo u konstruktoru, a
`setCurrentPlaybackSampleRate` zovemo samo iz `prepareToPlay`. Zato nikad ne dodavaj,
ne briši glasove niti mijenjaj brzinu uzorkovanja synthu iz `processBlock` ili s druge niti dok svira.
Sav naš vlastiti kod (npr. `SynthVoice`) ostaje bez zaključavanja.

## Etape

Radimo jednu etapu po redu. Ne prelazi na sljedeću dok vlasnik ne potvrdi da trenutna radi u FL Studiju.
Git commit radi tek nakon te potvrde.

1. **Prazan instrument** s jednim gumbom za glasnoću koji se učita u FL Studio. (gotovo)
2. **Oscilator i MIDI**: pilasti val (PolyBLEP, bez aliasinga), 8 glasova preko `juce::Synthesiser`,
   note iz FL Studija. (gotovo)
3. **Ovojnica (ADSR)**: gumbi Attack, Decay, Sustain, Release (`juce::ADSR` u svakom glasu) i novo, šire
   sučelje s karticama. (gotovo)
4. **Osnove**: 4 valna oblika, niskopropusni filtar (Cutoff, Resonance), pitch bend s odabirom raspona,
   16 glasova, crteži valnog oblika, odziva filtra i ovojnice. ← trenutna
5. **Modulacija**: LFO (vibrato, tremolo, pomicanje cutoffa), ovojnica filtra.
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
- `Source/PluginProcessor.*`: parametri, MIDI i zbrajanje glasova (audio nit)
- `Source/SynthVoice.*`: jedan glas: oscilator, filtar, ovojnica, pitch bend (audio nit)
- `Source/Filter.h`: niskopropusni filtar (audio nit); isti račun odziva koristi i `FilterDisplay`
- `Source/SynthSound.h`: oznaka "zvuka" koji synth svira
- `Source/ParameterIds.h`: imena svih parametara (jedino mjesto gdje se pišu kao tekst)
- `Source/PluginEditor.*`: sučelje (glavna nit)
- `Source/LabeledKnob.*`, `Source/LabeledChoice.*`: gumb i padajući izbornik s natpisom, spojeni na parametar
- `Source/WaveformDisplay.*`, `Source/FilterDisplay.*`, `Source/EnvelopeDisplay.*`: crteži u karticama
- `Source/Theme.h`: boje sučelja

## Pravila za parametre

- Novi parametar: dodaj ID u `ParameterIds.h`, napravi ga u `createParameterLayout()` i spoji ga s gumbom
  preko `LabeledKnob`. Nikad ne piši ID kao tekst na drugom mjestu (tipfeler = nullptr = rušenje).
- ID postojećeg parametra se ne mijenja nakon što postoje spremljeni projekti (FL po njemu pamti postavke).
- Glasovi čitaju parametre kroz `std::atomic<float>*` (vidi `SynthVoice`). Ovojnica se u glasu ažurira
  samo kad se vrijednost promijeni: `juce::ADSR::setParameters` preračuna brzine, pa bi usred otpuštanja
  promijenio njegov nagib.
- Parametri s izborom (valni oblik, raspon pitch benda) spremaju se kao **indeks**. Redoslijed izbora mora
  biti isti u `PluginProcessor.cpp` i `SynthVoice.cpp`, a novi izbor se dodaje samo na **kraj** popisa
  (inače se spremljeni projekti pokvare).
- Gumbi koji mijenjaju zvuk glatko prelaze na novu vrijednost (`juce::SmoothedValue`: cutoff, pitch bend,
  uključivanje filtra), da pomicanje gumba ne pravi klikove.
- Kad je filtar posve otvoren (cutoff na vrhu, bez rezonancije), zvuk ide mimo filtra (`LowPassFilter::isOpen`),
  pa zadani zvuk ostaje bit-po-bit isti kao bez filtra.

## Pravila za sučelje

- Crteži i gumbi se na promjenu parametra ažuriraju preko `juce::ParameterAttachment` ili
  `...Attachment` klasa, koje rade na glavnoj niti. Sučelje nikad ne čita parametre izravno s audio niti
  niti ne stavlja `Listener`e koji bi se zvali s nje.
- Boje i zajednička pozadina crteža nalaze se u `Theme.h`; ne piši boje na više mjesta.
- Veličine i razmaci su konstante na vrhu `PluginEditor.cpp`; raspored se računa iz njih.
