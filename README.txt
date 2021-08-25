Nella cartella 'src' sono presenti:
	- i codici sorgenti da noi sviluppati ('simul.c', 'improved_simul.c' e 'utils.c')
	- i codici sorgenti forniti dall'autore del libro di testo necessari alla compilazione ('rngs.c', 'rvgs.c' e 'rvms.c')
	- i codici sorgenti forniti dall'autore del libro di testo utili ad inferire statistiche ('estimate.c' e 'uvs.c')

Nella cartella 'include' sono presenti:
	- un header file da noi sviluppato ('utils.h')
	- gli header file forniti dall'autore del libro di testo necessari alla compilazione ('rngs.h', 'rvgs.h' e 'rvms.h')

Per generare i file eseguibili è necessario posizionarsi nella directory in cui è presente il 'Makefile' ed eseguire i seguenti comandi:
	- 'make simulator' (o solamente 'make') per compilare la versione di base del simulatore ('bin/simul')
	- 'make improved_simulator' per compilare la versione del simulatore dotato di miglioria ('bin/improved_simul')
	- 'make uvs' per generare il programma 'bin/uvs'
	- 'make estimate' per generare il programma 'bin/estimate'
	- 'make all' per eseguire tutte le regole di compilazione precedentemente descritte

Inoltre:
	- le regole di compilazione sopra riportate generano automaticamente le cartelle 'bin' e 'obj' contenenti rispettivamente i file eseguibili e oggetto
	- eseguendo il comando 'make clean' è possibile rimuovere tutti i file oggetto generati dalla compilazione
	- eseguendo il comando 'make clean-all' è possibile rimuovere tutti i file oggetto ed eseguibili generati dalla compilazione
