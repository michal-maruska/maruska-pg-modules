
#include <string.h>


#include <postgres.h>
#include <catalog/pg_type.h>
// postgresql/9.5/server/
#include <catalog/pg_collation.h>
#include <fmgr.h>
#include <regex/regex.h>
#include <executor/spi.h>


/* #include "mb/pg_wchar.h" */


#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

PG_FUNCTION_INFO_V1(sset);
Datum sset(PG_FUNCTION_ARGS);


/* Look at Multi-Byte string TEXT.
 * Skip over N chars, and find the byte-lenght of the M chars.
 * Returns the start and in @len
 * the byte-lenght of M chars, starting from the N+1 th one. */
static char*
skip_n_take_m(char* text, int n, int m, int* len)
{
    /* skip */
    int skipped = 0;
    while (skipped < n) {
        int i = pg_mblen(text);
        text += i;
        skipped ++;
    }

    /* take */
    {
        int taken = 0;
        int l = 0;
        char* t = text;             /* text stops here */

        while(taken < m) {
            int i = pg_mblen(t);
            t += i;
            l += i;
            taken ++;
        }
        *len = l;
    }
    return text;
}


/* Given a C @string, of lenght @len,
 * return the text type value */
static inline text*
make_text(char* string, size_t len)
{
  text* text_out = palloc(len + VARHDRSZ);
  SET_VARSIZE(text_out, len + VARHDRSZ);
  memcpy((void *) VARDATA(text_out), string, len);
  return text_out;
}


#define USE_REGEXP 1
#define DEBUG 0

#define BUFSIZE 150

/* numero, relname, string
 * We assume, that `relname' contains an attribute named same as relname!
 *
 * Ex.  hobby (numero, hobby, string)
 */
Datum
sset(PG_FUNCTION_ARGS)
{

    text* table_t;
    int   numero;
    text* sadd_text_t;

    char* table;
    char* sadd_text;
    pg_wchar* wtext;
    int sadd_len;


    char* query /* [BUFSIZE]; */ = palloc(BUFSIZE);
    void  *plan;
    char* nulls= " ";

#if USE_REGEXP
    regex_t re;
#endif

    /*  so we can be non-strict !!*/
    if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2)){
        PG_RETURN_NULL();
    };


    /* char */
    table_t = PG_GETARG_TEXT_P(0); /* PG_GETARG_CSTRING(1); */
    table =  strndup((char*) VARDATA(table_t),
                     VARSIZE(table_t) - VARHDRSZ); /* fixme!*/

#if DEBUG
    elog(INFO, "table= %s(%d), numero = %d", table,
             VARATT_SIZEP(table_t) - VARHDRSZ, numero);
#endif

    numero = PG_GETARG_INT32(1);

    /* Convert the MB argument to pg_wchar*  */
    sadd_text_t = PG_GETARG_TEXT_P(2);
    {
        int   sadd_len1 =   VARSIZE(sadd_text_t) - VARHDRSZ;
        sadd_text =   strndup(VARDATA(sadd_text_t),sadd_len1);
        /* VARDATA(sadd_text_t);*/

        sadd_len1 = pg_mbstrlen(sadd_text);

        wtext = (pg_wchar *) palloc((sadd_len1 + 1) * sizeof(pg_wchar));
        sadd_len    = pg_mb2wchar(sadd_text, wtext);
#if DEBUG
        elog(INFO, "MB string is %d C long, %d chars in coding, %d Wchar long",
             VARSIZE(sadd_text_t) - VARHDRSZ, sadd_len1, sadd_len);
#endif
    }

    if (SPI_connect() != SPI_OK_CONNECT)
        elog(ERROR, "could not connect to SPI manager");


    /* fixme: This could be a simple  `strcat' */
    snprintf(query, BUFSIZE, "delete from %s where numero=%d;", table, numero);

#if DEBUG
    elog(INFO, "exec: %s", query);
#endif
    SPI_execute(query,false,0);

    snprintf(query, BUFSIZE, "insert into %s (numero, %s) VALUES (%d, $1);",
                 table, table, numero);

    {
        Oid argtypes[1] = {TEXTOID };
        plan = SPI_prepare(query, 1, argtypes); /* fixme! F_OID_TEXT */
        /* UTF_U2E(argv[1]), nargs, qdesc->argtypes);*/
        /* fixme:  if failure? */
#if DEBUG
        elog(INFO, "Plan |%s| prepared!", query);
#endif
    }

#if USE_REGEXP
    /* cached_re_str re_temp; */
    {
        /* Create the regexp pattern:
           We write it in char*
           but PG wants it in pg_wchar:  */
        char *pat = "^ *([^ ]+)";     /* hungry RE! */
        int len = strlen(pat);
        /* mmc: is there a macro *alloc(len, type) ? */
        pg_wchar* pattern = (pg_wchar *) palloc((len + 1) * sizeof(pg_wchar));
        int pattern_len = pg_mb2wchar_with_len(pat, pattern, len);

        if (0 != pg_regcomp(&re, pattern, pattern_len, REG_EXTENDED, DEFAULT_COLLATION_OID))
            /* REG_NOSUB */
            elog(ERROR, "regexp compiling failed");
    }

    {
        /* bool read_only= FALSE; */
        long count = 10;
        Datum data[1];
        regmatch_t match[2];


        while ((sadd_len > 0) &&
               (0 == pg_regexec(&re, wtext,
                                sadd_len, /* lenght */
                                0,    /* start */
                                NULL, /* no details?? */

                                2,    /* REGEXP_REPLACE_BACKREF_CNT */
                                match, /* NULL */
                                0)))   /* flags */
            {
                text* t;
                /* lenght of complete match! */
                int mlen = match[0].rm_eo - match[0].rm_so;
                int submlen = match[1].rm_eo - match[1].rm_so;

                if (match[0].rm_so == -1)
                    elog(ERROR, "did not match!");

#if 0
                elog(INFO, "match: %d %d - %d %d !", (int)match[0].rm_so, mlen,
                         (int)match[1].rm_so, submlen); /* fixme! conversion! */
#endif

                /* We have to skip over N chars, and take M! */
                {
                    int len;
                    sadd_text = skip_n_take_m(sadd_text, match[1].rm_so,
                                                  submlen, &len);

                    /* fixme:  recycle! */
                    // t = make_text(sadd_text, len);
                    t = (text*) palloc(VARHDRSZ + len);
                    SET_VARSIZE(t, VARHDRSZ + submlen); /* 2 + VARHDRSZ; */

                    /* this is nonsense!  sadd_text is pg_wchar! */
                    memcpy((void *) VARDATA(t),
                           /* wtext */
                           sadd_text, /* bug: sadd_text + match[1].rm_so, */
                           len);
                    /* VARATT_DATA(t)[submlen] = 0; */
                    sadd_text += len;
                }

#if 0
                elog(INFO, "match: %d %d (%s)!", match[0].rm_so, mlen,
                     VARATT_DATA(t));
#endif
                data[0] = (Datum) t;

                {
                    int result;
                    if ((result = /* SPI_execute_plan(plan, data, nulls, read_only, count))*/
                         SPI_execp(plan, data, nulls, count))
                        != SPI_OK_INSERT) /* <0 */
                    {
                        elog(ERROR, "execute plan failed %d! %s",
                             result,
                             (result ==SPI_ERROR_ARGUMENT)?"SPI_ERROR_ARGUMENT":
                             ((result == SPI_ERROR_PARAM )?"SPI_ERROR_PARAM":
                              "dunno"));
                    };
                }
                /* step ahead: */
                wtext += mlen;
                sadd_len -= mlen;
            }
#if 0
        elog(INFO, "while finished!! remains %s", sadd_text);
#endif


#else  /* USE_REGEXP */
        /* fixme: i don't need Regexps!  */
        /* skip over spaces. skip until spaces. push. */

        while (1) {
            while (sadd_text[i] && (sadd_text[i] == ' ')) /* ASCII ?? */
                i++;
            /* still something remains? */
            if ()
                break;
            /* Dig it all */
            /* Do the insert */
        }
        /* while (len > 2, & [0] != ' ' &&  [1] != ' ' && ([2] == '0' ||
         * [2] == ' ')) */
#endif
    }

    /* SPI_freeplan(plan); */
    if (SPI_finish() != SPI_OK_FINISH)
        elog(ERROR, "SPI_finish() failed");

    /* result is the # of matches? */
    PG_RETURN_INT32(1);
}
