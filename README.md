# --- TCP UDP Client - Server ---

Copyright 2023 - 2024 Nedelcu David

## Descriere

Acest proiect reprezinta implementarea unui sistem de mesagerie, in care
clientii (TCP) se aboneaza la un topic*, iar apoi serverul le trimite
acestor abonati care sunt conectati mesaje pe acel topic transmise de catre clientii
UDP*, mesaje care pot avea diferite tipuri de date*.

* *Clientii TCP - se conecteaza la server si se pot abona / dezabona de la un topic
* *Clientii UDP - trimit mesaje pe un topic
* *Topic - un string care poate avea forma: `topic1/topic2/..`, respectiv pot include
caractere wildcard: `*` (poate fi inlocuit cu 0 sau oricate nivele de topic) si `+`
(poate fi inlocuit cu exact un nivel de topic)
* *Tipuri de date - mesajele pot fi de tip: `INT`, `SHORT_REAL`, `FLOAT`, `STRING`

## Detalii de implementare

### Server

Serverul isi initializeaza socketii: TCP listener, UDP listener si stdin, intr-un
vector de structuri pollfd. Respectiv, asculta noi conexiuni TCP, adaugandu-le
in lista de clienti TCP, si primeste mesaje de la clientii UDP, pe care le trimite
clientilor TCP abonati la topicul respectiv.

Serverul mentine clientii conectati intr-un vector de structuri `user_data`, respectiv
topic-urile si cu ID-urile clientilor abonati intr-un unordered_map (hash table).

### Client TCP

Clientul TCP se conecteaza la server si isi valideaza ID-ul dat serverului (unic).
Apoi, poate trimite comenzi de abonare / dezabonare la un topic, sau de deconectare.
Clientul primeste mesajele de la server si le afiseaza sub formatul: `topic - type - data`.

Acesta foloseste si el un vector de structuri pollfd pentru a asculta de la server si
de la stdin.

### Protocol de comunicare peste TCP

Pentru a putea comunicare intre client TCP si server, folosim o structura de tip `tcp_comm`,
care contine tipul mesajului (INIT, TOPIC, SUBSCRIBE, UNSUBSCRIBE), campul id_taken (pentru
a valida ID-ul clientului), campul ID (pentru initializarea unui client),
campul topic (pentru a specifica topicul la care se aboneaza /
dezaboneaza clientul) si campul message pentru cazul in care se trimite un mesaj de la server
la client, redirectionat de la clientii UDP.

## Problema Checker

Uneori, checkerul nu ruleaza corect. O data la cateva rulari, acesta posibil sa ramana
in urma fata de outpul-ul efectiv  al serverului. Raspunsurile sunt corecte, dar checkerul
nu le recunoaste din cauza acestui 'offset'.

Manual, orice test ruleaza corect. Testul de quick-flow ruleaza, deci nu ar trebui sa fie
o problema de viteza de rulare.

Ceea ce consider ca se intampla este ca uneori checkerul ramane in urma cu output-ul
propriu zis. Totusi, de cele mai multe ori, checkerul trece toate testele.

## Concluzii

Proiectul a adus in vedere abordarea unui sistem complex, si a fost un challenge
sa gestionez atatea conexiuni si mesaje din venind si plecand din parti diferite.

Avand in vedere imbunatatirile pe care le-as putea aduce, as putea sa abordez intr-o
maniera diferita stocarea topic-urilor astfel incat sa pot face cautari mai eficiente,
respectiv a userilor conectati.