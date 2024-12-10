
/* generalization of translate  */

#include <postgres.h>
#include <varatt.h>
#include <string.h>
#include <fmgr.h>

#include <mb/pg_wchar.h>

#define DEBUG 0

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

Datum f5(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(f5);

const struct
{
#if 0
        /* could precompute the lenght in bytes. */
        int flen;
        int tlen;
#endif

        char* from;
        char* to;
}

        translation_table[] =
                {
#if 0
                        {"а", "a"},
                        {"б", "b"},
                        {"ш", "sch"}
#endif
                        /* note: these are 4 bytes */
                        {"щ","stch"},
                        {"Щ","Stch"},
                        {"Щ","STCH"},
                        {"ш","sch"},
                        {"ч","tch"},
                        /* {"х","kch"}, */
                        {"Ш","Sch"},
                        {"Ш","SCH"},
                        {"Ч","Tch"},
                        {"Ч","TCH"},
                        /*{"Х","Kch"},*/
                        {"ж","zh"},
                        {"Ж","Zh"},
                        {"Ж","ZH"},
                        {"ж","zh"},
                        {"Ж","Zh"},
                        /*{"ц","tc"},
                          #{"ц","ts"},
                          #{"Ц","Tc"},
                          #{"Ц","Ts"}, */
                        {"Я","Ja"},
                        {"Я","Ya"},
#if 0
                        {"Е","Je"},
                        {"Е","Ye"},
#endif
                        {"Ю","Ju"},
                        {"Ю","Yu"},
                        {"Ё","Jo"},
                        {"Ё","Yo"},
                        {"Х","Ch"},
                        {"я","ja"},
                        {"я","ya"},
#if 0
                        {"е","je"},
                        {"е","ye"},
#endif
                        {"ю","ju"},
                        {"ю","yu"},
                        {"ё","jo"},
                        {"ё","yo"},
                        {"х","ch"},
                        {"а","a"},
                        {"б","b"},
                        {"ц","c"},
                        {"д","d"},
                        {"е","e"},
                        {"ф","f"},
                        {"г","g"},
                        {"х","h"},
                        {"и","i"},
                        {"й","j"},
                        {"к","k"},
                        {"л","l"},
                        {"м","m"},
                        {"н","n"},
                        {"о","o"},
                        {"п","p"},
                        {"р","r"},
                        {"с","s"},
                        {"т","t"},
                        {"у","u"},
                        {"в","v"},
                        {"в","w"},
                        /*{"Я","x"}, */
                        {"ы","y"},
                        {"з","z"},
                        {"А","A"},
                        {"Б","B"},
                        {"Ц","C"},
                        {"Д","D"},
                        {"Е","E"},
                        {"Ф","F"},
                        {"Г","G"},
                        {"Х","H"},
                        {"И","I"},
                        {"Й","J"},
                        {"К","K"},
                        {"Л","L"},
                        {"М","M"},
                        {"Н","N"},
                        {"О","O"},
                        {"П","P"},
                        {"Р","R"},
                        {"С","S"},
                        {"Т","T"},
                        {"У","U"},
                        {"В","V"},
                        {"В","W"},
                        /*{"о","X"}, */
                        {"Ы","Y"},
                        {"З","Z"},
                        {"ь",""},
                        {"Э","E"},
                        {"э","e"},
                        /* {"je","e"}, */
                        {NULL,NULL}
                };

int translation_table_len = sizeof(translation_table)
        / sizeof(translation_table[0]);


/* UTF8 here? or load it from a table? */
static void
start()
{
        /* Get the # of entries in the array: */
        int i = 0;
#if DEBUG
        elog(WARNING, "%s: translation_table is long %d",
             __FUNCTION__, translation_table_len);
#endif

        while (translation_table[i].from)
                i++;
        translation_table_len = i;

#if DEBUG
        elog(WARNING, "%s: translation_table is long %d",
             __FUNCTION__, translation_table_len);
#endif
}


/********************************************************************
 *
 * translate
 *
 * Syntax:
 *
 *	 text translate(text string, text from, text to)
 *
 * Purpose:
 *
 *	 Returns string after replacing all occurrences of characters in from
 *	 with the corresponding character in to.  If from is longer than to,
 *	 occurrences of the extra characters in from are deleted.
 *	 Improved by Edwin Ramirez <ramirez@doc.mssm.edu>.
 *
 ********************************************************************/

Datum
f5(PG_FUNCTION_ARGS)
{
	text *string = PG_GETARG_TEXT_P(0);
	text *result;
	char *source, *target;
	int  m;
#if 0
	char *from_ptr, *to_ptr;
        int fromlen, tolen;
#endif
        int retlen, i;
	int str_len;
	int estimate_len;
	int len;
	int source_len;
	int from_index;

	if ((m = VARSIZE(string) - VARHDRSZ) <= 0)
                /* is this correct? it's aliasing, so double-freed? */
		PG_RETURN_TEXT_P(string);
#if 0
	fromlen = VARSIZE(from) - VARHDRSZ;
	from_ptr = VARDATA(from);
	tolen = VARSIZE(to) - VARHDRSZ;
	to_ptr = VARDATA(to);
#endif


        /* fixme!  */
        start();

	str_len = VARSIZE(string) - VARHDRSZ;
        if (str_len == 0)
                elog(WARNING, "%s: empty-string as input!", __FUNCTION__);

        /* Allocate the result Text  */
	estimate_len = str_len * 3; /* (tolen * 1.0 / fromlen + 0.5) * str_len; */
        /* min() */
	// estimate_len = (estimate_len > str_len) ? estimate_len : str_len;
#if DEBUG
        elog(WARNING, "%s: result estimated at lenght %d, input %d",
             __FUNCTION__, estimate_len, str_len);
#endif
	result = (text *) palloc(VARHDRSZ + estimate_len);

	source = VARDATA(string);
	target = VARDATA(result);
	retlen = VARHDRSZ;

        /* available memory: */
	while (m > 0) {
                /* Get next character: */
                source_len = pg_mblen(source);
                /* mmc: so this uses the DB coding? */
                from_index = 0;

                if (! source_len)
                        elog(WARNING, "%s: empty-string as input!", __FUNCTION__);

                /* Find in the Alist */
                for (i = 0; i < translation_table_len; i++)
                {

                        /* pg_mblen(&from_ptr[i]) */
                        len = pg_mblen(translation_table[i].from); /* flen fixme! */

#if 0
                        elog(WARNING, "%s: translation_table pattern %d is long %d",
                             __FUNCTION__, i, len);
#endif
                        if (len == source_len && /*  &from_ptr[i] */
                            memcmp(source, translation_table[i].from, len) == 0)
                                break;
                        /* from_index++; */
                }

                /* Found -> rewrite */
                if (i < translation_table_len) {
#if 0
                        /* substitute */
                        char	   *p = to_ptr;

                        for (i = 0; i < from_index; i++)
                        {
                                p += pg_mblen(p);
                                if (p >= (to_ptr + tolen))
                                        break;
                        }
                        if (p < (to_ptr + tolen))
                        {
                                len = pg_mblen(p);
                                memcpy(target, p, len);
                                target += len;
                                retlen += len;
                        }
#else
                        char *p = translation_table[i].to;

                        len = strlen(p); /* pg_mblen(p);*/
                        memcpy(target, p, len);
                        target += len;
                        retlen += len;
#endif
                } else {
                        /* no match, so copy */
#if DEBUG
                        elog(WARNING, "%s: no match, so copying %d",
                             __FUNCTION__, source_len);
#endif
                        memcpy(target, source, source_len);
                        target += source_len;
                        retlen += source_len;
                }


                source += source_len;
                m -= source_len;
        }

#if DEBUG
        elog(WARNING, "%s: returning. total len %d",
             __FUNCTION__, retlen);
#endif

        SET_VARSIZE(result, retlen);
	// VARATT_SIZEP(result) = retlen + VARHDRSZ;
	/*
	 * There may be some wasted space in the result if deletions occurred, but
	 * it's not worth reallocating it; the function result probably won't live
	 * long anyway.
	 */
	PG_RETURN_TEXT_P(result);
}
