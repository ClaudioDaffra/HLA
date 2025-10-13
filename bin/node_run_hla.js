#!/usr/bin/env node

// Carica il modulo compilato da Emscripten
const hla = require('./hla.js');

// Attende che il runtime WASM sia completamente inizializzato
hla.onRuntimeInitialized = () => {
  try {
    // 1. Monta la directory di lavoro corrente (host) nella directory /work (WASM)
    hla.FS.mkdir('/work');
    hla.FS.mount(hla.FS.filesystems.NODEFS, { root: process.cwd() }, '/work');
    
    // 2. Cambia la directory di lavoro corrente a /work
    hla.FS.chdir('/work');

    // 3. Prende gli argomenti passati allo script (es. -i, file.hla, ecc.)
    const originalArgs = process.argv.slice(2);
    const rewrittenArgs = [];

    // 4. Analizza gli argomenti per riscrivere i percorsi dei file
    for (let i = 0; i < originalArgs.length; i++) {
      const currentArg = originalArgs[i];
      const prevArg = i > 0 ? originalArgs[i - 1] : null;

      // Se l'argomento precedente era un flag che si aspetta un file (-i o -o),
      // allora questo argomento è un percorso e va modificato.
      if (prevArg === '-i' || prevArg === '-o') {
        // Estrai solo il nome del file senza il percorso
        const fileName = currentArg.split('/').pop();
        rewrittenArgs.push(fileName);
      } else {
        // Altrimenti, è un flag (come -i) o un altro tipo di opzione, quindi lo lasciamo com'è.
        rewrittenArgs.push(currentArg);
      }
    }

    console.log(`[Node Wrapper] Esecuzione di main con argomenti: ${rewrittenArgs.join(' ')}`);

    // 5. Esegue la funzione main() del C++ con gli argomenti corretti
    hla.callMain(rewrittenArgs);

  } catch (e) {
    // Emscripten termina il programma sollevando un'eccezione.
    // Se lo status è 0, è tutto ok, altrimenti mostriamo l'errore.
    if (e.name !== 'ExitStatus' || e.status !== 0) {
      console.error('Errore durante l\'esecuzione del modulo WASM:', e);
    }
  }
};