# PRGX Synth

VST3 synth plugin za Windows (FL Studio), napravljen u JUCE-u i CMake-u.

**Trenutno stanje (etapa 3):** instrument koji svira note pilastim valom, do 8 nota istovremeno
(akordi), s ovojnicom (gumbi Attack, Decay, Sustain, Release) i gumbom za glasnoću (Volume).

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
2. Idi u mapu projekta i kopiraj plugin:

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

Trebao bi se otvoriti prozor s naslovom "PRGX Synth", karticom **ENVELOPE** (Attack, Decay, Sustain,
Release) i karticom **OUTPUT** (Volume). Dvoklik na gumb vraća ga na početnu vrijednost, a klik na broj
ispod gumba omogućuje upis vrijednosti (npr. `250 ms` ili `1.5 s`).

5. Sviraj note: otvori **Piano roll** (desni klik na kanal → *Piano roll*), ucrtaj note i pritisni
   Play, ili klikni klavijaturu na lijevoj strani Piano rolla. (MIDI klavijatura radi ako je spojena
   i uključena u FL Studiju.) Čut ćeš pilasti val, a gumb Volume mijenja glasnoću.

   Što rade gumbi ovojnice (kako se glasnoća note mijenja u vremenu):

   | Gumb | Što radi |
   |---|---|
   | Attack | koliko dugo nota raste od tišine do pune glasnoće (dugo = "meki" početak) |
   | Decay | koliko dugo nakon toga pada na razinu držanja |
   | Sustain | razina na kojoj nota ostaje dok držiš tipku |
   | Release | koliko dugo nota utihne nakon što pustiš tipku |

## Struktura projekta

```
CMakeLists.txt            postavke projekta i plugina, preuzimanje JUCE-a
Source/PluginProcessor.*  parametri, primanje MIDI nota, zbrajanje glasova
Source/SynthVoice.*       jedan glas: oscilator (pilasti val) i ovojnica (ADSR)
Source/SynthSound.h       oznaka "zvuka" koji synth svira
Source/ParameterIds.h     imena svih parametara na jednom mjestu
Source/PluginEditor.*     prozor plugina (kartice s gumbima)
Source/LabeledKnob.*      gumb s natpisom i vrijednošću, spojen na parametar
Source/Theme.h            boje sučelja
CLAUDE.md                 pravila projekta i popis etapa
.gitignore                što Git ne prati (mapa build/)
```
