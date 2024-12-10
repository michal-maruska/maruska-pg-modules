#include <postgres.h>
#include <varatt.h>
#include <string.h>
#include <fmgr.h>


#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif


PG_FUNCTION_INFO_V1(char_in_text);

#define USE_BPCHAR 1

Datum char_in_text(PG_FUNCTION_ARGS);

Datum
char_in_text(PG_FUNCTION_ARGS)
{
        text* text_in;
        size_t size;

#if USE_BPCHAR
        BpChar *bpchar_in;
#else
        unsigned char char_in;
#endif
        /* fixme:      so we can be non-strict !!*/
        if (PG_ARGISNULL(0) || (PG_ARGISNULL(1))){
                /* not ! */
                PG_RETURN_NULL();
        };



#if USE_BPCHAR
        bpchar_in = PG_GETARG_BPCHAR_P(0);
#else
        char_in = PG_GETARG_CHAR(0);
#endif


        text_in = PG_GETARG_TEXT_P(1);

#if 0
        if (char_in == 'b')
                PG_RETURN_BOOL(false);
#endif
        /* DatumGetChar */
        size = VARSIZE(text_in)-VARHDRSZ;
        if (size > 100)
                PG_RETURN_NULL();

        {
                void* found = memchr(VARDATA(text_in),
#if USE_BPCHAR
                                     VARDATA(bpchar_in)[0],
#else
                                     char_in,
#endif
                                     size);

                /* PG_RETURN_BOOL((VARSIZE(text_in) > 10)?false:true); */
                PG_RETURN_BOOL((found == NULL)?false:true);
        }
}


