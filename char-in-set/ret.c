/*
* FUNCTION: input text/cstring, return char.
*/
#include "postgres.h"
#include "fmgr.h"

PG_FUNCTION_INFO_V1(retchar);
PG_FUNCTION_INFO_V1(retchar1);

/*
* Fetch first character of text.
* Returns char
*/
Datum
retchar( PG_FUNCTION_ARGS )
{
        text *val = (text *) PG_GETARG_TEXT_P(0);
        char retdata = *(VARDATA(val)) ;
        PG_RETURN_CHAR( retdata );
}

/* Verbatim from utils/adt/char.c; changed name of function only; */
Datum
retchar1(PG_FUNCTION_ARGS)
{
        text       *arg1 = PG_GETARG_TEXT_P(0);
        char            result;
        /*
         * An empty input string is converted to \0 (for consistency with
         * charin). If the input is longer than one character, the excess data
         * is silently discarded.
         */
        if (VARSIZE(arg1) > VARHDRSZ)
                result = *(VARDATA(arg1));
        else
                result = '\0';

        PG_RETURN_CHAR(result);
}
