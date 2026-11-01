# PRGX Synth

VST3 synth plugin za Windows (FL Studio), napravljen u JUCE-u i CMake-u.

**Trenutno stanje (etapa 4):** instrument s 4 valna oblika (sinus, pila, kvadrat, trokut), niskopropusnim
filtrom (Cutoff, Resonance), ovojnicom (Attack, Decay, Sustain, Release), glavnom glasnoćom (Volume),
kotačićem za visinu tona (pitch bend) i pedalom za sustain. Svira do 16 nota istovremeno (akordi).

## 1. Što treba instalirati (samo prvi put)

1. **Visual Studio Community** (2022 ili novija, besplatno): https://visualstudio.microsoft.com/
   U instalaciji označi radno opterećenje **"Desktop development with C++"**.
   To je C++ prevoditelj koji pretvara naš kod u plugin.
2. **CMake**: https://cmake.org/download/ (Windows x64 Installer).
   Tijekom instalacije odaberi **"Add CMake to the system PATH"**.
3. **Git**: https://git-scm.com/ (CMake ga koristi za preuzimanje JUCE-a).

Nakon instalacije **zatvori i ponovno otvori** terminal (ili VS Code) da bi vidio nove programe.
Provjera: `cmake --version` mora ispisati verziju.

## 2. Izgradnja

U terminalu, u mapi projekta:

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

- Dio `-G "Visual Studio 17 2022"` je važan: bez njega CMake može odabrati pogrešan alat (NMake)
  i javiti da ne pronalazi prevoditelj.
- Prva naredba pripremi projekt. **Prvi put traje nekoliko minuta** jer skida JUCE s interneta.
- Druga naredba prevede kod i napravi plugin. Prvi put traje par minuta, poslije je brže.

Gotov plugin nalazi se ovdje:

```
build\PrgxSynth_artefacts\Release\VST3\PRGX Synth.vst3
```

`PRGX Synth.vst3` izgleda kao datoteka, ali je zapravo **mapa**. Kopira se cijela.

## 3. Kopiranje u mapu koju FL Studio skenira

Standardna mapa za VST3 pluginove na Windowsima je:

```
C:\Program Files\Common Files\VST3
```

FL Studio je skenira automatski. Za pisanje u nju trebaju administratorska prava, pa:

1. Otvori Start, upiši **PowerShell**, desni klik → **Run as administrator**.
2. Idi u mapu projekta i kopiraj plugin. Zamijeni `PUT_DO_MAPE_PROJEKTA` stvarnom putanjom
   (u File Exploreru otvori mapu projekta, klikni u adresnu traku i kopiraj putanju):

```powershell
cd "PUT_DO_MAPE_PROJEKTA"
Copy-Item -Recurse -Force "build\PrgxSynth_artefacts\Release\VST3\PRGX Synth.vst3" "C:\Program Files\Common Files\VST3\"
```

(Može i ručno: u File Exploreru kopiraj mapu `PRGX Synth.vst3` i zalijepi je u
`C:\Program Files\Common Files\VST3`, Windows će tražiti potvrdu administratora.)

> Kad kasnije ponovno izgradiš plugin, **zatvori FL Studio** prije kopiranja,
> inače je stara verzija zaključana i kopiranje ne uspije.

## 4. Učitavanje u FL Studio

1. Otvori FL Studio → izbornik **Options → Manage plugins**.
2. Klikni **Find installed plugins** (ili "Find more plugins") i pričekaj skeniranje.
3. U popisu potraži **PRGX Synth** (pod *Generators*, jer je instrument) i označi zvjezdicu ★
   da se pojavi u izborniku.
4. Zatvori prozor, u **Channel rack** klikni **+** i odaberi **PRGX Synth**.

Trebao bi se otvoriti prozor s naslovom "PRGX Synth" i četiri kartice, redoslijedom kojim zvuk putuje:
**OSCILLATOR** → **FILTER** → **ENVELOPE** → **OUTPUT**. Crteži u karticama (oblik vala, odziv filtra,
oblik ovojnice) mijenjaju se dok vrtiš gumbe. Dvoklik na gumb vraća ga na početnu vrijednost, a klik na
broj ispod gumba omogućuje upis vrijednosti (npr. `250 ms`, `1.5 s`, `800` ili `1.5k` za Cutoff).

5. Sviraj note: otvori **Piano roll** (desni klik na kanal → *Piano roll*), ucrtaj note i pritisni
   Play, ili klikni klavijaturu na lijevoj strani Piano rolla. (MIDI klavijatura radi ako je spojena
   i uključena u FL Studiju.) Početni zvuk je pilasti val.

   Što rade kontrole:

   | Kontrola | Što radi |
   |---|---|
   | Waveform | oblik vala: Sine (mekan, bez viših tonova), Saw (svijetao, pun), Square (šuplji, "drveni"), Triangle (blag) |
   | Bend Range | za koliko poluglasova kotačić za visinu tona (pitch bend) pomiče notu. Postavi isto kao raspon kanala u FL Studiju (zadano 2) |
   | Cutoff | od koje frekvencije filtar utišava više tonove. Skroz desno je filtar otvoren (zvuk se ne mijenja) |
   | Resonance | koliko filtar naglasi tonove baš oko Cutoffa. Visoko daje "zvonki", šiljasti zvuk |
   | Attack | koliko dugo nota raste od tišine do pune glasnoće (dugo = "meki" početak) |
   | Decay | koliko dugo nakon toga pada na razinu držanja |
   | Sustain | razina na kojoj nota ostaje dok držiš tipku |
   | Release | koliko dugo nota utihne nakon što pustiš tipku |
   | Volume | glavna glasnoća plugina |

   Za isprobavanje: Attack oko 1 s daje "pad" zvuk koji nježno raste. Sustain 0 % uz Decay oko 300 ms daje
   kratki "pluck". Cutoff povučen dolje uz malo Resonancea daje klasični "wah" zvuk kad ga vrtiš dok nota svira.

## Struktura projekta

```
CMakeLists.txt            postavke projekta i plugina, preuzimanje JUCE-a
Source/PluginProcessor.*  parametri, primanje MIDI nota, zbrajanje glasova
Source/SynthVoice.*       jedan glas: oscilator, filtar i ovojnica (ADSR), pitch bend
Source/Filter.h           niskopropusni filtar (matematika filtra)
Source/SynthSound.h       oznaka "zvuka" koji synth svira
Source/ParameterIds.h     imena svih parametara na jednom mjestu
Source/PluginEditor.*     prozor plugina (kartice)
Source/LabeledKnob.*      gumb s natpisom i vrijednošću, spojen na parametar
Source/LabeledChoice.*    padajući izbornik s natpisom, spojen na parametar
Source/WaveformDisplay.*  crtež odabranog valnog oblika
Source/FilterDisplay.*    crtež odziva filtra
Source/EnvelopeDisplay.*  crtež oblika ovojnice
Source/Theme.h            boje sučelja
CLAUDE.md                 pravila projekta i popis etapa
.gitignore                što Git ne prati (mapa build/)
LICENSE                   licenca (AGPL v3)
```

## Licenca

Projekt je objavljen pod licencom **GNU AGPL v3** (vidi datoteku `LICENSE`). Kod možeš slobodno
koristiti, mijenjati i dijeliti, pod uvjetom da i tvoje izmijenjene verzije ostanu pod AGPL v3 i da je
izvorni kod dostupan. Razlog je JUCE: on je za open-source projekte dostupan pod AGPL v3, pa naš plugin
mora biti pod istom licencom. Za komercijalnu upotrebu bez takvih obveza JUCE ima posebnu plaćenu licencu.
