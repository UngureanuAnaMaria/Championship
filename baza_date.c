#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
   NotUsed=0;
   for(int i=0; i < argc; i++)
   {
       printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }

   printf("\n");
   return 0;
}

int main()
{
   sqlite3 *db;
   char *msg_err = 0;
   int rc;
   char *sql;

   rc = sqlite3_open("test.db", &db);

   if(rc != SQLITE_OK)
   {
      fprintf(stderr,"Nu se poate deschide baza de date: %s.\n", sqlite3_errmsg(db));
      sqlite3_close(db);

      return 1;
   }
   else
   {
      fprintf(stdout, "Baza de date deschisa cu succes.\n");
   }

   sql = "DROP TABLE IF EXISTS UTILIZATORII;" \
         "CREATE TABLE UTILIZATORII(" \
         "NUME_UTILIZATOR CHAR(15) PRIMARY KEY NOT NULL, "\
         "EMAIL TEXT NOT NULL,"\
         "PAROLA TEXT NOT NULL,"\
         "ADMIN INT NOT NULL,"\
         "PUNCT_SLAB TEXT NOT NULL,"\
         "PUNCT_TARE TEXT NOT NULL);";

   rc = sqlite3_exec(db, sql, callback, 0, &msg_err);

   if(rc != SQLITE_OK)
   {
      fprintf(stderr, "Nu s-a putut crea tabela UTILIZATORII.\n");
      fprintf(stderr, "Eroare SQL: %s.\n", msg_err);
      sqlite3_free(msg_err);
   }
   else
   {
      fprintf(stdout, "Tabela UTILIZATORII creata cu succes.\n");
   }

   sql = "DROP TABLE IF EXISTS CAMPIONATELE;"\
         "CREATE TABLE CAMPIONATELE(" \
         "NUME TEXT PRIMARY KEY NOT NULL,"\
         "TIP TEXT NOT NULL,"\
         "NR_JUCATORI INT NOT NULL,"\
         "STRUCTURA TEXT NOT NULL,"\
         "REGULI TEXT NOT NULL,"\
         "MOD_EXTRAGERE_PARTIDA TEXT NOT NULL,"\
         "NR_PARTICIPANTI INT,"\
         "PARTICIPANTI TEXT,"\
         "ISTORIC TEXT,"\
         "CASTIGATORI TEXT,"\
         "SCORURI,"\
         "DATA TEXT,"\
         "ORA TEXT);";

   rc = sqlite3_exec(db, sql, callback, 0, &msg_err);

   if(rc != SQLITE_OK)
   {
      fprintf(stderr, "Nu s-a putut crea tabela CAMPIONATELE.\n");
      fprintf(stderr, "Eroare SQL: %s.\n", msg_err);
      sqlite3_free(msg_err);
   }
   else
   {
      fprintf(stdout, "Tabela CAMPIONATELE creata cu succes.\n");
   }

   sql = "INSERT INTO UTILIZATORII (NUME_UTILIZATOR, EMAIL, PAROLA, ADMIN, PUNCT_SLAB, PUNCT_TARE) VALUES ('ana', 'anaungureanu2003@gmail.com', '1234', 1, 'fotbal', 'tenis'); " \
         "INSERT INTO UTILIZATORII (NUME_UTILIZATOR, EMAIL, PAROLA, ADMIN, PUNCT_SLAB, PUNCT_TARE) VALUES ('maria45', 'anaungureanu2003@gmail.com', 'tenis', 0, 'fotbal', 'tenis'); " \
         "INSERT INTO UTILIZATORII (NUME_UTILIZATOR, EMAIL, PAROLA, ADMIN, PUNCT_SLAB, PUNCT_TARE) VALUES ('andrei', 'anaungureanu2003@gmail.com', 'parola', 1, 'tenis', 'fotbal'); " \
         "INSERT INTO UTILIZATORII (NUME_UTILIZATOR, EMAIL, PAROLA, ADMIN, PUNCT_SLAB, PUNCT_TARE) VALUES ('george', 'anaungureanu2003@gmail.com', 'fotbal', 0, 'tenis', 'fotbal'); " \
         "INSERT INTO CAMPIONATELE (NUME, TIP, NR_JUCATORI, STRUCTURA, REGULI, MOD_EXTRAGERE_PARTIDA, NR_PARTICIPANTI, PARTICIPANTI, ISTORIC, CASTIGATORI, SCORURI, DATA, ORA) " \
         "VALUES ('Campionat national de tenis', 'tenis', 2, 'single-elimination', 'Jucatorul care pune mingea in joc este cel care serveste.', 'Random', 1, 'maria45', '9/4/20, 6/5/22', 'andrei, maria45', '30 - 45, 70 - 90', '2/4/24', '10.00'); " \
         "INSERT INTO CAMPIONATELE (NUME, TIP, NR_JUCATORI, STRUCTURA, REGULI, MOD_EXTRAGERE_PARTIDA, NR_PARTICIPANTI, PARTICIPANTI, ISTORIC, CASTIGATORI, SCORURI, DATA, ORA) " \
         "VALUES ('Campionat fotbal', 'fotbal', 22, 'double-elimination', 'Fiecare repriza se poate prelungi maxim 15 minute.', 'Scorul cel mai mare.', 2, 'andrei, george', '5/6/23, 7/8/23', 'andrei, andrei', '4 - 5, 3 - 6', '4/6/24', '14.00'); " ;

   rc = sqlite3_exec(db, sql, callback, 0, &msg_err);

   if(rc != SQLITE_OK)
   {
      fprintf(stderr, "Nu s-au putut insera inregistrari in tabele.\n");
      fprintf(stderr, "Eroare SQL: %s.\n", msg_err);
      sqlite3_free(msg_err);
   }
   else
   {
      fprintf(stdout, "Inregistrarile au fost introduse cu succes.\n");
   }

   sqlite3_close(db);
   return 0;
}
