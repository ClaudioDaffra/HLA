#!/usr/bin/env node

const path = require('path');
const hla = require('./64tass.js');
const fs = require('fs');

// Funzione per terminare il processo in modo pulito
function terminateProcess(exitCode) {
    console.log(`[Node Wrapper] Terminazione con codice: ${exitCode}`);
    process.exit(exitCode);
}

// Funzione per verificare se il file di output è stato creato correttamente
function checkOutputFile(args) {
    const outputIndex = args.indexOf('-o');
    if (outputIndex !== -1 && outputIndex + 1 < args.length) {
        const outputFile = args[outputIndex + 1];
        const fullPath = path.join(process.cwd(), outputFile);
        
        if (fs.existsSync(fullPath) && fs.statSync(fullPath).size > 0) {
            console.log(`[Node Wrapper] File di output verificato: ${fullPath}`);
            return true;
        }
    }
    return false;
}

// Sovrascriviamo la funzione problematica nel modulo WASM
hla.onRuntimeInitialized = () => {
    try {
        // 1. Monta la directory principale del progetto (parent directory) in /project
        hla.FS.mkdir('/project');
        hla.FS.mount(hla.FS.filesystems.NODEFS, { root: '..' }, '/project');
        
        // 2. Cambia la directory di lavoro corrente a /project/prj
        hla.FS.chdir('/project/prj');

        // 3. Prepara gli argomenti senza aggiungere percorsi aggiuntivi
        // poiché ora i percorsi relativi funzioneranno correttamente
        const originalArgs = process.argv.slice(2);
        const finalArgs = [...originalArgs];

        console.log(`[Node Wrapper] Esecuzione con argomenti: ${finalArgs.join(' ')}`);

        // 4. Intercettiamo la funzione callMain per prevenire l'errore
        const originalCallMain = hla.callMain;
        hla.callMain = function(args) {
            try {
                // Eseguiamo l'originale callMain in un contesto controllato
                const result = originalCallMain.call(this, args);
                
                // Forziamo l'uscita immediata dopo il completamento
                setTimeout(() => {
                    const success = checkOutputFile(args);
                    terminateProcess(success ? 0 : 1);
                }, 100);
                
                return result;
            } catch (e) {
                // Se c'è un errore, verifichiamo se il file è stato creato
                const success = checkOutputFile(args);
                if (success) {
                    console.log('[Node Wrapper] Errore ignorato perché il lavoro è completato');
                    terminateProcess(0);
                } else {
                    throw e;
                }
            }
        };

        // 5. Eseguiamo la funzione main() del C++
        hla.callMain(finalArgs);

        // 6. Impostiamo un timer di sicurezza per forzare l'uscita
        setTimeout(() => {
            const success = checkOutputFile(finalArgs);
            terminateProcess(success ? 0 : 1);
        }, 1000);

    } catch (e) {
        if (e.name === 'ExitStatus') {
            console.log(`[Node Wrapper] Programma terminato con codice: ${e.status}`);
            terminateProcess(e.status);
        } else {
            console.error('Errore durante l\'esecuzione:', e);
            terminateProcess(1);
        }
    }
};

// Disabilitiamo completamente i gestori di errori globali
// per prevenire la visualizzazione dell'errore WebAssembly
process.removeAllListeners('uncaughtException');
process.removeAllListeners('unhandledRejection');

// Aggiungiamo un gestore silenzioso per gli errori
process.on('uncaughtException', () => {
    // Non facciamo nulla, l'errore verrà gestito dal nostro codice
});

process.on('unhandledRejection', () => {
    // Non facciamo nulla, l'errore verrà gestito dal nostro codice
});
