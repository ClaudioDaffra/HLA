#!/usr/bin/env node

// Carica il modulo compilato da Emscripten
const hla = require('./mcpp.js');
const path = require('path');

// Attende che il runtime WASM sia completamente inizializzato
hla.onRuntimeInitialized = () => {
  try {
    // 1. Monta la directory principale del progetto (HLA) nella directory /work (WASM)
    hla.FS.mkdir('/work');
    // Montiamo la directory parent invece della directory corrente
    hla.FS.mount(hla.FS.filesystems.NODEFS, { root: '..' }, '/work');
    
    // 2. Cambia la directory di lavoro corrente a /work/prj
    hla.FS.chdir('/work/prj');

    // 3. Prende gli argomenti passati allo script (es. -i, file.hla, ecc.)
    const originalArgs = process.argv.slice(2);
    const rewrittenArgs = [];

    // 4. Analizza gli argomenti per riscrivere i percorsi dei file
    for (let i = 0; i < originalArgs.length; i++) {
      const currentArg = originalArgs[i];
      const prevArg = i > 0 ? originalArgs[i - 1] : null;

      // Se l'argomento precedente era un flag che si aspetta un file (-i o -o),
      // allora questo argomento è un percorso e va modificato.
      if (prevArg === '-i') {
        // Per l'opzione -i, aggiungi il percorso completo con /work/prj/
        rewrittenArgs.push('/work/prj/' + currentArg);
      } else if (prevArg === '-o') {
        // Per l'opzione -o, estrai solo il nome del file senza il percorso
        const fileName = currentArg.split('/').pop();
        rewrittenArgs.push(fileName);
      } else if (currentArg !== '-i') {
        // Altrimenti, è un flag (ma non -i) o un altro tipo di opzione, quindi lo lasciamo com'è.
        rewrittenArgs.push(currentArg);
      }
      // Se currentArg è '-i', lo saltiamo completamente
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
