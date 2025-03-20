#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <sqlite3.h>
#include <curl/curl.h>

#define PORT 2024

int rc;
sqlite3 *db;
char *msg_err = 0;

typedef struct thData
{
   int idThread;
   int cl;
}thData;

int conectat[1000] = {0};
int admin[1000];
char nume_utilizator[15];
char punct_slab[100];
char punct_tare[100];

int conectare_utilizator(int idThread, char comanda[])
{
   int parola_gasita = 0 , nume_gasit = 0;
   int corect = 1;
   char sql[1000];
   char nume_utiliz[15];
   char parola_utiliz[50];

   rc = sqlite3_open("test.db", &db);

   if(rc != SQLITE_OK)
   {
      fprintf(stderr, "Nu se poate deschide baza de date: %s.\n", sqlite3_errmsg(db));
      sqlite3_close(db);

      exit(EXIT_FAILURE);
   }
   else
   {
      fprintf(stdout, "Baza de date deschisa cu succes.\n");
   }

   int termeni = sscanf(comanda, "conectare:%s %49[^\n]\n)", nume_utiliz, parola_utiliz);

   if(termeni != 2)
   {
      corect = 0;
   }

   sqlite3_stmt *stmt;

   sqlite3_prepare_v2(db, "select nume_utilizator from utilizatorii", -1, &stmt, 0);

   while(sqlite3_step(stmt) != SQLITE_DONE)
   {
      const char *nume = (const char *)sqlite3_column_text(stmt, 0);

      if(strcmp(nume_utiliz, nume) == 0)
      {
         strncpy(nume_utilizator, (const char *)nume, 15);
         nume_utilizator[strlen(nume_utilizator)]='\0';
         nume_gasit = 1;

         sqlite3_stmt *stmmt;
         sprintf(sql, "select parola, admin, punct_slab, punct_tare from utilizatorii where nume_utilizator = '%s'", nume_utilizator);
         sqlite3_prepare_v2(db, sql, -1, &stmmt, 0);

         while(sqlite3_step(stmmt) != SQLITE_DONE)
         {
            const char *parola = (const char *)sqlite3_column_text(stmmt, 0);

            if(strcmp(parola_utiliz, parola) == 0)
            {

               parola_gasita=1;

               const char *admin_normal = (const char *)sqlite3_column_text(stmmt, 1);
               const char *punct_sb = (const char *)sqlite3_column_text(stmmt, 2);
               const char *punct_forte = (const char *)sqlite3_column_text(stmmt, 3);

               admin[idThread] = atoi(admin_normal);

               strncpy(punct_slab, (const char *)punct_sb, 100);
               punct_slab[strlen(punct_slab)] = '\0';

               strncpy(punct_tare, (const char *)punct_forte, 100);
               punct_tare[strlen(punct_tare)] = '\0';
            }
         }

         sqlite3_finalize(stmmt);
      }
   }

   if(nume_gasit == 1 && parola_gasita == 1 && corect == 1)
   {
      conectat[idThread] = 1;
      strcpy(comanda, "Utilizator conectat.");
   }
   else if(nume_gasit == 0 && corect == 1)
   {
      strcpy(comanda, "Numele nu este in baza de date. Incercati alt nume de utilizator.");
   }
   else if(parola_gasita == 0 && corect == 1)
   {
      strcpy(comanda, "Parola gresita. Incercati din nou, conectati-va la alt cont  sau schimbati parola.");
   }
   else if(corect == 0)
   {
      strcpy(comanda, "Format incorect. Incercati din nou.");
   }

   rc = sqlite3_finalize(stmt);
   sqlite3_close(db);
   return 0;
}

void inregistrare_utilizator(char comanda[])
{
   char sql[5000];
   char nume[15];
   char email[50];
   char parola[50];
   char punct_slab[50];
   char punct_tare[50];
   int admin = 0;
   int nume_gasit = 0, email_gasit = 0;
   int corect = 1;

   rc = sqlite3_open("test.db", &db);

   if(rc != SQLITE_OK)
   {
      fprintf(stderr, "Nu se poate deschide baza de date: %s.\n", sqlite3_errmsg(db));
      sqlite3_close(db);

      exit(EXIT_FAILURE);
   }
   else
   {
      fprintf(stdout, "Baza de date deschisa cu succes.\n");
   }

   int termeni = sscanf(comanda, "inregistrare utilizator:%s %s %s %d %s %49[^\n]\n", nume, email, parola, &admin, punct_slab, punct_tare);//extras

   if(termeni != 6)
   {
      corect = 0;
   }

   sqlite3_stmt *stmt;
   sqlite3_prepare_v2(db, "select nume_utilizator, email from utilizatorii;", -1, &stmt, 0);

   while(sqlite3_step(stmt) != SQLITE_DONE)
   {
       const char *nume_utiliz = (const char *)sqlite3_column_text(stmt, 0);
       const char *email_utiliz = (const char *)sqlite3_column_text(stmt, 1);

       if(strcmp(nume, nume_utiliz) == 0)
       {
          nume_gasit = 1;
       }

       if(strcmp(email, email_utiliz) == 0)
       {
          email_gasit = 1;
       }
   }

   sqlite3_finalize(stmt);
   if(corect == 1)
   {
     if(nume_gasit == 0 && email_gasit == 0)
     {
        sprintf(sql, "INSERT INTO UTILIZATORII (NUME_UTILIZATOR, EMAIL, PAROLA, ADMIN, PUNCT_SLAB, PUNCT_TARE) VALUES ('%s', '%s', '%s', '%d', '%s', '%s');", nume, email, parola, admin, punct_slab, punct_tare);
        rc = sqlite3_exec(db, sql, 0, 0, &msg_err);

        if(rc != SQLITE_OK)
        {
           fprintf(stderr, "Nu s-a putut insera o noua inregistrare in tabela UTILIZATORII: %s.\n", sqlite3_errmsg(db));
           sqlite3_close(db);

           exit(EXIT_FAILURE);
        }
        else
        {
           fprintf(stdout, "S-a inserat o noua inregistrare in tabela UTILIZATORII cu succes.\n");
        }

        strcpy(comanda, "A fost introdusa cu succes o noua inregistrare in tabela UTILIZATORII.");

        comanda[strlen(comanda)] = '\0';
     }
     else if(nume_gasit == 1 && email_gasit == 1)
     {
        strcpy(comanda, "Exista deja un cont cu acest nume de utilizator si acelasi email.");
        comanda[strlen(comanda)] = '\0';
     }
     else if(nume_gasit == 1 && email_gasit == 0)
     {
        strcpy(comanda, "Exista deja un cont cu acest nume de utilizator.");
        comanda[strlen(comanda)] = '\0';
     }
     else if(nume_gasit == 0 && email_gasit == 1)
     {
        strcpy(comanda, "Exista deja un cont cu acest email.");
        comanda[strlen(comanda)] = '\0';
     }
   }
   else
   {
     strcpy(comanda, "Format incorect. Introduceti comanda din nou.");
   }

   sqlite3_close(db);
}



void inregistrare_campionat(char comanda[])
{
   char sql[5000];
   char nume[100];
   char tip[25];
   char structura[30];
   char reguli[200];
   char mod_extragere_partida[100];
   char participanti[200];
   char istoric[100];
   char castigatori[100];
   char scoruri[100];
   char data[15];
   char ora[6];
   char nr_jucatori[5];
   char nr_participanti[5];
   int nume_gasit = 0;
   int corect = 1;

   rc = sqlite3_open("test.db", &db);

   if(rc != SQLITE_OK)
   {
      fprintf(stderr, "Nu se poate deschide baza de date: %s.\n", sqlite3_errmsg(db));
      sqlite3_close(db);

      exit(EXIT_FAILURE);
   }
   else
   {
      fprintf(stdout, "Baza de date deschisa cu succes.\n");
   }

   int termeni = sscanf(comanda, "inregistrare campionat:%99[^:]:%24[^:]:%4[^:]:%29[^:]:%199[^:]:%99[^:]:%4[^:]:%199[^:]:%99[^:]:%99[^:]:%99[^:]:%14[^:]:%5[^\n]\n", nume, tip, nr_jucatori, structura, reguli, mod_extragere_partida, nr_participanti, participanti, istoric, castigatori, scoruri, data, ora);

   if(termeni != 13)
   {
      corect = 0;
   }

   int nr_jucatorii = atoi(nr_jucatori);
   int nr_participantii = atoi(nr_participanti);

   sqlite3_stmt *stmt;
   sqlite3_prepare_v2(db, "select nume from campionatele", -1, &stmt, 0);

   while(sqlite3_step(stmt) != SQLITE_DONE)
   {
       const char *nume_campionat = (const char *)sqlite3_column_text(stmt, 0);

       if(strstr(nume, nume_campionat) != 0)
       {
          nume_gasit = 1;
       }
   }

   rc = sqlite3_finalize(stmt);

   if(corect == 1)
   {
      if(nume_gasit == 0)
      {
         sprintf(sql, "INSERT INTO CAMPIONATELE (NUME, TIP, NR_JUCATORI, STRUCTURA, REGULI, MOD_EXTRAGERE_PARTIDA, NR_PARTICIPANTI, PARTICIPANTI, ISTORIC, CASTIGATORI, SCORURI, DATA, ORA) VALUES ('%s', '%s', '%d', '%s', '%s', '%s', '%d', '%s', '%s', '%s', '%s', '%s', '%s');", nume, tip, nr_jucatorii, structura, reguli, mod_extragere_partida, nr_participantii, participanti, istoric, castigatori, scoruri, data, ora);
         rc = sqlite3_exec(db, sql, 0, 0, &msg_err);

         if(rc != SQLITE_OK)
         {
            fprintf(stderr, "Nu s-a putut insera o noua inregistrare in tabela CAMPIONATELE: %s\n.", sqlite3_errmsg(db));
            sqlite3_close(db);

            exit(EXIT_FAILURE);
         }
         else
         {
            fprintf(stdout, "S-a inserat cu succes o noua inregistrare in tabela CAMPIONATELE.\n");
         }

         strcpy(comanda, "A fost introdusa cu succes o noua inregistrare in tabela CAMPIONATELE.");
      }
      else if(nume_gasit == 1)
      {
         strcpy(comanda, "Exista deja un campionat cu acest nume.");
      }
   }
   else
   {
      strcpy(comanda, "Format incorect. Incercati din  nou.");
   }

   sqlite3_close(db);

}

void informatii_campionate(char comanda[])
{
   const char *nume;
   const char *tip;
   const char *nr_jucatori;
   const char *structura;
   const char *reguli;
   const char *mod_extragere_partida;

   rc = sqlite3_open("test.db", &db);

   if(rc != SQLITE_OK)
   {
      fprintf(stderr, "Nu se poate deschide baza de date: %s.\n", sqlite3_errmsg(db));
      sqlite3_close(db);

      exit(EXIT_FAILURE);
   }
   else
   {
      fprintf(stdout, "Baza de date deschisa cu succes.\n");
   }

   sqlite3_stmt *stmt;
   sqlite3_prepare_v2(db, "select nume, tip, nr_jucatori, structura, reguli, mod_extragere_partida from campionatele", -1, &stmt, 0);

   strcpy(comanda, "\n");

   while(sqlite3_step(stmt) != SQLITE_DONE)
   {
       nume = (const char *)sqlite3_column_text(stmt, 0);
       strcat(comanda, "Nume campionat : ");
       strcat(comanda, nume);
       strcat(comanda, "\n");
       tip = (const char *)sqlite3_column_text(stmt, 1);
       strcat(comanda, "Tip : ");
       strcat(comanda, tip);
       strcat(comanda, "\n");
       nr_jucatori = (const char *)sqlite3_column_text(stmt, 2);
       strcat(comanda, "Numar jucatori : ");
       strcat(comanda, nr_jucatori);
       strcat(comanda, "\n");
       structura = (const char *)sqlite3_column_text(stmt, 3);
       strcat(comanda, "Structura : ");
       strcat(comanda, structura);
       strcat(comanda, "\n");
       reguli = (const char *)sqlite3_column_text(stmt, 4);
       strcat(comanda, "Reguli joc : ");
       strcat(comanda, reguli);
       strcat(comanda, "\n");
       mod_extragere_partida = (const char *)sqlite3_column_text(stmt, 5);
       strcat(comanda, "Mod extragere partida : ");
       strcat(comanda, mod_extragere_partida);
       strcat(comanda, "\n \n");
   }

   rc = sqlite3_finalize(stmt);
   sqlite3_close(db);
}

void istoric_partide(char comanda[])
{
   char sql[5000];
   const char *nume;
   const char *nr_participanti;
   const char *participanti;
   const char *istoric;
   const char *castigatori;
   const char *scoruri;
   const char *data;
   const char *ora;

   rc = sqlite3_open("test.db", &db);

   if(rc != SQLITE_OK)
   {
      fprintf(stderr, "Nu se poate deschide baza de date: %s.\n", sqlite3_errmsg(db));
      sqlite3_close(db);

      exit(EXIT_FAILURE);
   }
   else
   {
      fprintf(stdout, "Baza de date deschisa cu succes.\n");
   }

   sqlite3_stmt *stmt;
   sprintf(sql, "select nume, nr_participanti, participanti, istoric, castigatori, scoruri, data, ora from campionatele");
   sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

   strcpy(comanda, "\n");

   while(sqlite3_step(stmt) != SQLITE_DONE)
   {
       nume = (const char *)sqlite3_column_text(stmt, 0);
       strcat(comanda, "Nume campionat : ");
       strcat(comanda, nume);
       strcat(comanda, "\n");
       nr_participanti = (const char *)sqlite3_column_text(stmt, 1);
       strcat(comanda, "Numar partipipanti : ");
       strcat(comanda, nr_participanti);
       strcat(comanda, "\n");
       participanti = (const char *)sqlite3_column_text(stmt, 2);
       strcat(comanda, "Participanti partida : ");
       strcat(comanda, participanti);
       strcat(comanda, "\n");
       istoric = (const char *)sqlite3_column_text(stmt, 3);
       strcat(comanda, "Istoric partide jucate : ");
       strcat(comanda, istoric);
       strcat(comanda, "\n");
       castigatori = (const char *)sqlite3_column_text(stmt, 4);
       strcat(comanda, "Castigatorii partidelor deja jucate : ");
       strcat(comanda, castigatori);
       strcat(comanda, "\n");
       scoruri = (const char *)sqlite3_column_text(stmt, 5);
       strcat(comanda, "Scorurile partidelor jucate : ");
       strcat(comanda, scoruri);
       strcat(comanda, "\n");
       data = (const char *)sqlite3_column_text(stmt, 6);
       strcat(comanda, "Data urmatoarei partide : ");
       strcat(comanda, data);
       strcat(comanda, "\n");
       ora = (const char *)sqlite3_column_text(stmt, 7);
       strcat(comanda, "Ora urmatoarei partide : ");
       strcat(comanda, ora);
       strcat(comanda, "\n \n");
   }

   rc = sqlite3_finalize(stmt);
   sqlite3_close(db);
}

ssize_t read_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
   FILE *readhere = (FILE *)userdata;
   curl_off_t nread;
   size_t retcode = fread(ptr, size, nmemb, readhere);
   nread = (curl_off_t)retcode;
   fprintf(stderr, "Citim %" CURL_FORMAT_CURL_OFF_T "bytes din fisier.\n", nread);
   return retcode;
}

void email(char comanda[])
{
   char sql[5000];
   const char *email;

   rc = sqlite3_open("test.db", &db);

   if(rc != SQLITE_OK)
   {
      fprintf(stderr, "Nu se poate deschide baza de date: %s.\n", sqlite3_errmsg(db));
      sqlite3_close(db);

      exit(EXIT_FAILURE);
   }
   else
   {
     fprintf(stdout, "Baza de date deschisa cu succes.\n");
   }

   FILE *fo = fopen("email.txt", "w+");

   if(fo == NULL)
   {
      fprintf(stderr, "Nu se poate deschide fisierul pentru scriere.\n");
      exit(EXIT_FAILURE);
   }

   fprintf(fo, "%s", comanda);
   fclose(fo);

   sqlite3_stmt *stmt;
   sprintf(sql, "select email from utilizatorii where nume_utilizator = '%s'", nume_utilizator);
   sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

   if(sqlite3_step(stmt) != SQLITE_DONE)
   {
      email = (const char *)sqlite3_column_text(stmt, 0);
   }

   CURL *curl;
   CURLcode res = CURLE_OK;
   curl = curl_easy_init();//init sesiune

   if(curl)
   {
      curl_easy_setopt(curl, CURLOPT_URL, "smtp.gmail.com");
      curl_easy_setopt(curl, CURLOPT_USERNAME, "anaungureanu2003@gmail.com");
      curl_easy_setopt(curl, CURLOPT_PASSWORD, "klixhbyvjkqonnzy");
      curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
      curl_easy_setopt(curl, CURLOPT_MAIL_FROM, "anaungureanu2003@gmail.com");

      struct curl_slist *recipients = NULL;
      recipients = curl_slist_append(recipients, email);
      curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

      FILE *fi = fopen("email.txt", "rb");

      if(fi == NULL)
      {
         fprintf(stderr, "Nu se poate deschide fisierul pentru citire.\n");
         exit(EXIT_FAILURE);
      }

      curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
      curl_easy_setopt(curl, CURLOPT_READDATA, (void *)fi);
      curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

      res = curl_easy_perform(curl);

      if(res != CURLE_OK)
      {
         fprintf(stderr, "curl_easy_perform() a esuat: %s.\n", curl_easy_strerror(res));
      }
      else
      {
         fprintf(stdout, "Email trimis.\n");
      }

      curl_slist_free_all(recipients);
      curl_easy_cleanup(curl);
      fclose(fi);
   }

   sqlite3_finalize(stmt);
   sqlite3_close(db);
}

void inregistrare_utilizator_campionat(char comanda[])
{
   char sql[1000];
   const char *nume;
   const char *tip;
   const char *nr_jucatori;
   const char *structura;
   const char *reguli;
   const char *mod_extragere_partida;
   const char *nr_participanti;
   const char *participanti;
   const char *data;
   const char *ora;
   int corect = 1;
   int nume_gasit = 0;

   rc = sqlite3_open("test.db", &db);

   if(rc != SQLITE_OK)
   {
      fprintf(stderr, "Nu se poate deschide baza de date: %s.\n", sqlite3_errmsg(db));
      sqlite3_close(db);

      exit(EXIT_FAILURE);
   }
   else
   {
      fprintf(stdout, "Baza de date deschisa cu succes.\n");
   }

   char nume_campionat[50];

   int termeni = sscanf(comanda, "inregistrare utilizator in campionat:%49[^\n]", nume_campionat);

   if(termeni != 1)
   {
      corect = 0;
   }

   if(corect == 0)
   {
      strcpy(comanda, "Format incorect. Incercati din nou.");
      exit(EXIT_FAILURE);
   }

   sqlite3_stmt *stmt;
   sprintf(sql, "select nume, tip, nr_jucatori, structura, reguli, mod_extragere_partida, nr_participanti, participanti, data, ora from campionatele"); 
   sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

   while(sqlite3_step(stmt) != SQLITE_DONE)
   {
      nume = (const char *)sqlite3_column_text(stmt, 0);

      if(strstr(nume, nume_campionat) != 0)
      {
         nume_gasit = 1;
         tip = (const char *)sqlite3_column_text(stmt, 1);
         nr_jucatori = (const char *)sqlite3_column_text(stmt, 2);
         nr_participanti = (const char *)sqlite3_column_text(stmt, 6);
         participanti = (const char *)sqlite3_column_text(stmt, 7);

         int nr_jucatorii = atoi(nr_jucatori);
         int nr_participantii = atoi(nr_participanti);

         if(strcmp(tip, punct_slab) != 0 && strcmp(participanti, nume_utilizator) != 0 && nr_participantii < nr_jucatorii)
         {
            structura = (const char *)sqlite3_column_text(stmt, 3);
            reguli = (const char *)sqlite3_column_text(stmt, 4);
            mod_extragere_partida = (const char *)sqlite3_column_text(stmt, 5);
            data = (const char *)sqlite3_column_text(stmt, 8);
            ora = (const char *)sqlite3_column_text(stmt, 9);

            sprintf(sql, "UPDATE CAMPIONATELE SET NR_PARTICIPANTI = NR_PARTICIPANTI + 1 WHERE NUME = '%s';", nume);
            rc = sqlite3_exec(db, sql, 0, 0, &msg_err);

            if(rc != SQLITE_OK)
            {
               fprintf(stderr, "Nu se poate updata baza de date(1): %s.\n", sqlite3_errmsg(db));
               sqlite3_close(db);

               exit(EXIT_FAILURE);
            }
            else
            {
               fprintf(stdout, "Baza de date updatata cu succes(1).\n");
            }

            sprintf(sql, "UPDATE CAMPIONATELE SET PARTICIPANTI = '%s, %s' WHERE NUME = '%s';", participanti, nume_utilizator, nume);
            rc = sqlite3_exec(db, sql, 0, 0, &msg_err);

            if(rc != SQLITE_OK)
            {
               fprintf(stderr, "Nu se poate updata baza de date(2): %s.\n", sqlite3_errmsg(db));
               sqlite3_close(db);

               exit(EXIT_FAILURE);
            }
            else
            {
               fprintf(stdout, "Baza de date updatata cu succes(2).\n");
            }


            char info[2000];

            strcpy(comanda, "\n");
            strcat(comanda, "Ai fost acceptat in campionat.Vezi mai jos detalii despre Campionat.\n");
            sprintf(info, "Nume campionat: %s\n", nume_campionat);
            strcat(comanda, info);
            sprintf(info, "Tip: %s\n", tip);
            strcat(comanda, info);
            sprintf(info, "Structura: %s\n", structura);
            strcat(comanda, info);
            sprintf(info, "Reguli joc: %s\n", reguli);
            strcat(comanda, info);
            sprintf(info, "Mod extragere partida(deciderea partidelor): %s\n", mod_extragere_partida);
            strcat(comanda, info);
            sprintf(info, "Adversari: %s\n", participanti);
            strcat(comanda, info);
            sprintf(info, "Data urmatoarei partide: %s\n", data);
            strcat(comanda, info);
            sprintf(info, "Ora la care incepe aceasta partida: %s\n", ora);
            strcat(comanda, info);

            email(comanda);
         }
         else if(strcmp(tip, punct_slab) != 0 && strcmp(participanti, nume_utilizator) == 0 && nr_participantii < nr_jucatorii)
         {
            strcpy(comanda, "Esti deja inregistrat in acest campionat.");
         }
         else if(strcmp(tip, punct_slab) == 0)
         {
            strcpy(comanda, "Din pacate nu ai fost acceptat. Incerca un alt campionat in functie de punctele tale forte.");

            email(comanda);
         }
         else if(nr_participantii == nr_jucatorii)
         {
            strcpy(comanda, "Partida a atins deja numarul maxim de participanti.");
         }
      }
   }

   if(nume_gasit == 0)
   {
      strcpy(comanda, "Nu exista un campionat cu numele asta.");
   }

   sqlite3_finalize(stmt);
   sqlite3_close(db);
}

void reprogramare_sesiune_joc(char comanda[])
{
   char sql[5000];
   char nume[100];
   char data[15];
   char ora[6];
   int corect = 1;
   int nume_gasit = 0, participa = 0;

   rc = sqlite3_open("test.db", &db);

   if(rc != SQLITE_OK)
   {
      fprintf(stderr, "Nu se poate deschide baza de date: %s.\n", sqlite3_errmsg(db));
      sqlite3_close(db);

      exit(EXIT_FAILURE);
   }
   else
   {
      fprintf(stdout, "Baza de date deschisa cu succes.\n");
   }

   int termeni = sscanf(comanda, "reprogramare sesiune joc:%99[^:]:%14[^:]:%5[^\n]\n", nume, data, ora);

   if(termeni != 3)
   {
      corect = 0;
   }

   sqlite3_stmt *stmt;
   sqlite3_prepare_v2(db, "select nume from campionatele", -1, &stmt, 0);

   while(sqlite3_step(stmt) != SQLITE_DONE)
   {
       const char * nume_campionat = (const char *)sqlite3_column_text(stmt, 0);

       if(strstr(nume, nume_campionat) != 0)
       {
          nume_gasit = 1;

          sqlite3_stmt *stmmt;
          sprintf(sql, "select participanti from campionatele where nume = '%s'", nume);
          sqlite3_prepare_v2(db, sql, -1, &stmmt, 0);

          while(sqlite3_step(stmmt) != SQLITE_DONE)
          {
              const char *participanti = (const char *)sqlite3_column_text(stmmt, 0);

              if(strstr(participanti, nume_utilizator) != 0)
              {
                 participa = 1;
              }
          }

          sqlite3_finalize(stmmt);
       }
   }

   rc = sqlite3_finalize(stmt);

   if(corect == 1)
   {
      if(nume_gasit == 1 && participa == 1)
      {
         sprintf(sql, "UPDATE CAMPIONATELE SET DATA = '%s', ORA = '%s' WHERE NUME = '%s';", data, ora, nume);
         rc = sqlite3_exec(db, sql, 0, 0, &msg_err);

         if(rc != SQLITE_OK)
         {
            fprintf(stderr, "Nu se poate updata baza de date(3): %s.\n", sqlite3_errmsg(db));
            sqlite3_close(db);

            exit(EXIT_FAILURE);
         }
         else
         {
            fprintf(stdout, "Baza date updatata cu succes(3).\n");
         }

         strcpy(comanda, "Sesiune de joc reprogramata cu succes.");
      }
      else if(nume_gasit == 0)
      {
         strcpy(comanda, "Nu exista un campionat cu acest nume.");
      }
      else if(participa == 0)
      {
         strcpy(comanda, "Nu participati la acest campionat, deci nu puteti reprograma nicio sesiune.");
      }
   }
   else
   {
      strcpy(comanda, "Format incorect. Incercati din nou.");
   }

   sqlite3_close(db);
}

void edit_parola(char comanda[])
{
   char sql[5000];
   char parola[50];
   char nume_utiliz[15];
   int corect = 1;

   rc = sqlite3_open("test.db", &db);

   if(rc != SQLITE_OK)
   {
      fprintf(stderr, "Nu se poate deschide baza de date: %s.\n", sqlite3_errmsg(db));
      sqlite3_close(db);

      exit(EXIT_FAILURE);
   }
   else
   {
      fprintf(stdout, "Baza de date deschisa cu succes.\n");
   }

   int termeni = sscanf(comanda, "editare parola:%s %49[^\n]\n", nume_utiliz, parola);

   if(termeni != 2)
   {
      corect = 0;
   }

   sqlite3_stmt *stmt;
   sqlite3_prepare_v2(db, "select nume_utilizator from utilizatorii", -1, &stmt, 0);

   while(sqlite3_step(stmt) != SQLITE_DONE)
   {
       const char *nume = (const char *)sqlite3_column_text(stmt, 0);

       if(strcmp(nume, nume_utiliz) == 0)
       {
          sprintf(sql, "UPDATE UTILIZATORII SET PAROLA = '%s' WHERE NUME_UTILIZATOR = '%s';", parola, nume_utiliz);
          rc = sqlite3_exec(db, sql, 0, 0, &msg_err);

          if(rc != SQLITE_OK)
          {
             fprintf(stderr, "Nu se poate updata baza de date(4).\n");
             sqlite3_close(db);

             strcpy(comanda, "Parola nu aputut fi schimbata. Incearca din nou.");

             exit(EXIT_FAILURE);
          }
          else
          {
             fprintf(stdout, "Baza de date updatata cu succes.(4).\n");

             strcpy(comanda, "Parola schimbata cu succes.");
          }
       }
   }

   if(corect == 0)
   {
      strcpy(comanda, "Format incorect. Incearca din nou.");
   }

   sqlite3_finalize(stmt);
   sqlite3_close(db);
}

void cazuri_raspuns(int idThread, char comanda[])
{
   if(strstr(comanda, "conectare:") != 0 && conectat[idThread] == 0)
   {
      conectare_utilizator(idThread, comanda);
   }
   else if(strstr(comanda, "conectare:") != 0 && conectat[idThread] == 1)
   {
      strcpy(comanda, "Exista deja un utilizator conectat : ");
      strcat(comanda, nume_utilizator);
   }
   else if(strstr(comanda, "inregistrare utilizator:") != 0 && conectat[idThread] == 0)
   {
      inregistrare_utilizator(comanda);
   }
   else if(strstr(comanda, "inregistrare utilizator:") != 0 && conectat[idThread] == 1)
   {
      strcpy(comanda, "Deconectati-va mai intai pentru a inregistra un alt cont.");
   }
   else if(strstr(comanda, "inregistrare campionat:") != 0 && conectat[idThread] == 1 && admin[idThread] == 1)
   {
      inregistrare_campionat(comanda);
   }
   else if(strstr(comanda, "inregistrare campionat:") != 0 && conectat[idThread] == 0)
   {
      strcpy(comanda, "Conectati-va mai intai pentru a adauga un campionat in baza de date.");
   }
   else if(strstr(comanda, "inregistrare campionat:") != 0 && conectat[idThread] == 1 && admin[idThread] == 0)
   {
      strcpy(comanda, "Nu sunteti administrator, in consecinta nu puteti adauga un nou campionat in baza de date.");
   }
   else if(strstr(comanda, "informatii campionate") != 0 && strlen(comanda) == 21 && conectat[idThread] == 1)
   {
      informatii_campionate(comanda);
   }
   else if(strstr(comanda, "informatii campionate") != 0 && strlen(comanda) == 21 && conectat[idThread] == 0)
   {
      strcpy(comanda, "Conectati-va mai intai pentru a primi informatii despre campionate.");
   }
   else if(strstr(comanda,"istoric partide") != 0 && conectat[idThread] == 1 && admin[idThread] == 1 && strlen(comanda) == 15)
   {
      istoric_partide(comanda);
   }
   else if(strstr(comanda, "istoric partide") != 0 && conectat[idThread] == 0 && strlen(comanda) == 15)
   {
      strcpy(comanda, "Conectati-va mai intai pentru a primi informatii despre istoricul partidelor.");
   }
   else if(strstr(comanda, "istoric partide") != 0 && conectat[idThread] == 1 && admin[idThread] == 0 && strlen(comanda) == 15)
   {
      strcpy(comanda, "Nu sunteti administrator, in consecinta nu puteti vizualiza istoricul partidelor.");
   }
   else if(strstr(comanda, "deconectare") != 0 && strlen(comanda) == 11 && conectat[idThread] == 1)
   {
      conectat[idThread] = 0;
      strcpy(comanda, "Utilizator deconectat.");
   }
   else if(strstr(comanda, "deconectare") != 0 && strlen(comanda) == 11 && conectat[idThread] == 0)
   {
      strcpy(comanda, "Nu exista niciun utilizator conectat pentru a fi de deconectat.");
   }
   else if(strstr(comanda, "inregistrare utilizator in campionat:") != 0 && conectat[idThread] == 1 && admin[idThread] == 0)
   {
      inregistrare_utilizator_campionat(comanda);
   }
   else if(strstr(comanda, "inregistrare utilizator in campionat:") != 0 && conectat[idThread] == 0)
   {
      strcpy(comanda, "Conectati-va mai intai pentru a va inregistra intr-un campionat.");
   }
   else if(strstr(comanda, "inregistrare utilizator in campionat:") != 0 && conectat[idThread] == 1 && admin[idThread] == 1)
   {
      strcpy(comanda, "Sunteti administrator, deci nu va puteti inregistra intr-un campionat.");
   }
   else if(strstr(comanda, "reprogramare sesiune joc:") != 0 && conectat[idThread] == 1 && admin[idThread] == 0)
   {
      reprogramare_sesiune_joc(comanda);
   }
   else if(strstr(comanda, "reprogramare sesiune joc:") != 0 && conectat[idThread] == 0)
   {
      strcpy(comanda, "Conectati-va mai intai pentru a reprograma o seiune de joc.");
   }
   else if(strstr(comanda, "reprogramare sesiune joc:") != 0 && conectat[idThread] == 1 && admin[idThread] == 1)
   {
      strcpy(comanda, "Sunteti administrator, deci nu puteti reprograma o sesiune de joc.");
   }
   else if(strstr(comanda, "quit") != 0 && strlen(comanda) == 4)
   {
      exit(0);
   }
   else if(strstr(comanda, "editare parola:") != 0 && conectat[idThread] == 0)
   {
      edit_parola(comanda);
   }
   else if(strstr(comanda, "help") != 0 && strlen(comanda) == 4)
   {
      strcpy(comanda, "conectare:nume parola\n");
      strcat(comanda, "inregistrare utilizator:nume email parola tip punct_slab punct_tare\n");
      strcat(comanda, "inregistrare campionat:nume:tip:nr_jucatori:structura:mod_extragere_partida:nr_participanti:participanti:istoric:castigatori:scoruri:data:ora\n");
      strcat(comanda, "inregistrare utilizator in campionat:nume_campionat\n");
      strcat(comanda, "reprogramare sesiune joc:nume_campionat:data:ora\n");
      strcat(comanda, "informatii campionate\n");
      strcat(comanda, "istoric partide\n");
      strcat(comanda, "quit\n");
      strcat(comanda, "deconectare\n");
      strcat(comanda, "editare parola:nume_utiliz parola\n");
      strcat(comanda, "help\n");
   }
   else
   {
      strcpy(comanda, "Comanda necunoscuta.");
   }
}

int raspuns(void *arg)
{
   char comanda[1000] = "";
   struct thData tdL;
   tdL= *((struct thData*)arg);

   while(1)
   {
      if((read(tdL.cl, &comanda, sizeof(comanda))) == -1)
      {
         printf("[thread %d]\n", tdL.idThread);
         printf("Eroare la read() de la client.\n");
      }

      comanda[strlen(comanda) - 1]='\0';

      printf("[thread %d] Am primit comanda: %s\n",tdL.idThread, comanda);

      cazuri_raspuns(tdL.idThread, comanda);

      if((write(tdL.cl, &comanda, strlen(comanda))) == -1)
      {
         perror("Eroare la write() catre client.\n");
         exit(EXIT_FAILURE);
      }
   }
}

static void *treat(void *arg)
{
   struct thData tdL;
   tdL = *((struct thData*)arg);

   printf("[thread] - %d - Asteptam mesajul...\n", tdL.idThread);
   fflush(stdout);
   pthread_detach(pthread_self());
   raspuns((struct thData*)arg);

   close((intptr_t)arg);
   return(NULL);
}

int main()
{
   struct sockaddr_in server;
   struct sockaddr_in client;
   int sd;
   pthread_t th[100];
   int on = 1;
   int i = 0;

   if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {
      perror("[SERVER]Eroare la socket().\n");
      exit(EXIT_FAILURE);
   }

   setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

   bzero(&server, sizeof(server));
   bzero(&client, sizeof(client));

   server.sin_family = AF_INET;
   server.sin_addr.s_addr = htonl(INADDR_ANY);
   server.sin_port = htons(PORT);

   if((bind(sd, (struct sockaddr *) &server, sizeof(struct sockaddr))) == -1)
   {
      perror("[SERVER]Eroare la bind().\n");
      exit(EXIT_FAILURE);
   }

   if(listen(sd,2) == -1)
   {
     perror("[SERVER]Eroare la listen().\n");
     exit(EXIT_FAILURE);
   }

   while(1)
   {
     int cd;
     socklen_t length = sizeof(client);
     thData *td;

     printf("[SERVER]Asteptam la portul %d...\n", PORT);
     fflush(stdout);

     cd = accept(sd, (struct sockaddr *) &client, &length);
     if(cd < 0)
     {
       perror("[SERVER]Eroare la accept().\n");
       continue;
     }

     td = (struct thData*)malloc(sizeof(struct thData));
     td->idThread = i++;
     td->cl = cd;

     pthread_create(&th[i], NULL, &treat, td);
   }
}
