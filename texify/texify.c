#include "postgres.h"
#include <varatt.h>
#include <string.h>
#include "fmgr.h"


/* Quote certain char*/


#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

char chars_to_quote[]="$_";


static inline text*
make_text(char* string, size_t len)
{
        text* text_out = palloc(len + VARHDRSZ);
        SET_VARSIZE(text_out, len + VARHDRSZ);
        memcpy((void *) VARDATA(text_out), string, len);
        return text_out;
}


PG_FUNCTION_INFO_V1(texify);
Datum texify(PG_FUNCTION_ARGS);


Datum
texify(PG_FUNCTION_ARGS)
{
        text *text_in;
        char *in;
        char *out, *o;

        /* remove  */
        if (PG_ARGISNULL(0)){
                PG_RETURN_NULL();
        }
        text_in = PG_GETARG_TEXT_P(0);
        in = VARDATA(text_in);


        out = o = alloca(2 * VARSIZE(text_in));
        {
                int j = 0;                /* number of quotatins! */
                int i;

                for(i=0;i<VARSIZE(text_in) - VARHDRSZ; i++)
                {
                        if (strchr(chars_to_quote,in[i]))
                        {
                                j++;
                                *o++ = '\\';
                        }
                        *o++ = in[i];
                }

                /* close the string?? */
                *o = 0;

                {
                        int t = VARSIZE(text_in) + j;
                        text* text_out = make_text(out, t - VARHDRSZ);
                        PG_RETURN_TEXT_P(text_out);
                }
        }
}
