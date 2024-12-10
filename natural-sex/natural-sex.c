#include <postgres.h>
#include <varatt.h>
#include <catalog/pg_type.h>

/* what? */
#include <fmgr.h>
/* #include "mb/pg_wchar.h" */
#define DEBUG 0


#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif


/* really 2 macros? */
PG_FUNCTION_INFO_V1(nsex);
Datum nsex(PG_FUNCTION_ARGS);


/* simply map: */
Datum
nsex(PG_FUNCTION_ARGS)
{
        BpChar *bpchar_in;
        BpChar *result;

        /*  so we can be non-strict !!*/
        if (PG_ARGISNULL(0)) {
                PG_RETURN_NULL();
        };

#if DEBUG
        elog(WARNING, "%s: reading arg", __FUNCTION__);
#endif

        /* sex is 1 character! */
        bpchar_in = PG_GETARG_BPCHAR_P(0);

        /* result is 1 char again: */
        result = palloc(VARHDRSZ + 1);
        SET_VARSIZE(result, VARHDRSZ + 1);

        /* DatumGetChar(PG_GETARG_DATUM(n)) -> ((char) GET_1_BYTE(X))*/
#if DEBUG
        char char_in = VARDATA(bpchar_in)[0];
        elog(WARNING, "%s: looking at the char %c", __FUNCTION__, char_in);
#endif


        if (strchr("muas", VARDATA(bpchar_in)[0]))
        {
                *(VARDATA(result)) = 'm';
        }
        else
        {
                *(VARDATA(result)) = 'f';
        }

#if DEBUG
        elog(WARNING, "%s: returning %c", __FUNCTION__, *(VARDATA(result)));
#endif
        PG_RETURN_BPCHAR_P(result);

        /* PG_RETURN_CHAR -> CharGetDatum(x) ->  ((Datum) SET_1_BYTE(X))
           -> (((Datum) (value)) & 0x000000ff) */
}
