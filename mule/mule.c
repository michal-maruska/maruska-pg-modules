#include <postgres.h>
#include <varatt.h>
#include <string.h>
#include <fmgr.h>

#include <iconv.h>

#define DEBUG 0

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif


/* given a string in MULE internal encoding (given as C string),
 * produce a C string, w/ a representation in Tex (w/ my custom font-changing macros
 * */



/* mule encodes in 2 bytes non-asci code. the 1st byte selects a code page.
 * */

/* a table mapping the 1st byte/code -> TeX macro */
#define change_cmd_limit 15

static
const char* change_cmd[change_cmd_limit]={
  // 0
  "",
  "\\IT ",
  "\\CZ ",
  //3
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  //10
  "",
  "",
  "\\RUS ",
  "",
  ""
};

static
const char* iconv_encoding[change_cmd_limit]={
   // 0
   "",
   "LATIN1",
   "LATIN2",
   //3
   "",
   "",
   "",
   "",
   "",
   "",
   "",
   //10
   "",
   "",
   "CYRILLIC ",
   "",
   ""
};


/* mule 'limit': here start the 1st bytes of 2-byte characters */
#define MULE_BOUNDARY 127
#define CHAR_IS_MULE_MARKER(x) ((unsigned char)x >= MULE_BOUNDARY)


/*  Append to a byte string, possibly re-allocating it.
 *   |XXXXXXXXXX---|
 *              |xxxxxxx|
 */

static char*
append_mem(char* store, size_t* store_end, size_t* store_capacity,
           const char* item, size_t len)
{
        char* new_store = store;

        if ((*store_end + len)>= *store_capacity){

                /* Should be (lenght of rest + reserve) */
                *store_capacity += len + 10;

                new_store = repalloc (store, *store_capacity);
                /* fixme: if new_store==NULL;  ... */
                if (new_store == NULL) {
                        elog(ERROR, "%s: couldn't allocate mem", __FUNCTION__);
                }
        }

#if DEBUG
        elog(WARNING, "%s: appending %d", __FUNCTION__, len);
#endif
        memcpy (new_store + *store_end, item, len);
        *store_end += len;
        return new_store;
};



/* convert a memory block (not C string) long LEN
   Return the here-allocated memory w/ result, and its lenght in destination_len */
static char*
mule_to_tex(const char* source_string, int len, int* destination_len)
{
        /*    |------------------------|
         *       ^
         *       c/offset
         *
         *       current   TeX 'charset page'
         *
         *  first_p
         */

        /* create a `store': */
        size_t store_end=0;
        size_t store_capacity=10;
        char*  store= palloc(store_capacity);

        unsigned char c;
        unsigned char current = MULE_BOUNDARY; /* current TeX encoding `Page' */
        int first_p = 1;

#if DEBUG
        elog(WARNING, "%s: input string %d long", __FUNCTION__, len);
#endif


        /* must not contain C ??*/
#define APPEND_CHAR(c)   {char C=c;  store = append_mem(store, &store_end, &store_capacity, &C, 1);}

        /* for (offset= ....) */
        int offset=0;
        while (offset <len) {
                c= source_string [offset++];
                if (c > MULE_BOUNDARY) {
                        if (c != current) {
                                /* the coding page has changed.
                                 *  Possibly close the previous group: */
                                if (first_p == 0) {
                                        APPEND_CHAR('}');
                                }
                                else {
                                        first_p = 0;
                                }

                                /* Enter a TeX 'group' and issue the TeX macro associated w/ the charpage */
                                APPEND_CHAR('{');

                                /* store = append_mem(store, &store_end, &store_capacity, "}", 1); */
                                /* append ('{'); */
                                if (c - MULE_BOUNDARY -1 < change_cmd_limit) {
                                        int index=c-MULE_BOUNDARY-1;
                                        /* append_string(); */
                                        store = append_mem(store, &store_end, &store_capacity,
                                                           change_cmd[index] ,
                                                           strlen (change_cmd[index]));
                                }
                                /* else  error !!! */
                                current = c;
                        }

                        /* append the char itself ?? but it is a 201 code ???  */
                        /* fixme:  */


                        /* append (c); */
                        /* APPEND_CHAR(c); */
                        /*store = append_mem(store, &store_end, &store_capacity, "}", 1); */

                        if (offset < len) {
                                /*Sembra, che ogni tanto c'e' un carattere alla fine. E senza questo, riceviamo ...*/

                                /*  append the 2nd byte */
                                /* i thought i could append in the next cycle  :
                                 * mmc: false; the chars itself are beyond the BOUNDARY,*/
                                APPEND_CHAR(source_string[ offset++ ]);
                        }
                } else {
                        /* simply push the ascii char: */
                        APPEND_CHAR(c);
                }
        }

        /* close the current group if necessary: */
        if (first_p == 0)
                APPEND_CHAR ('}');

        /* return the data: */
        *destination_len = store_end;

#if DEBUG
        elog(WARNING, "%s: output string %d long", __FUNCTION__, *destination_len);
#endif
        return store;
 }



static inline text*
make_text(char *string, size_t len)
{
#if DEBUG
        elog(WARNING, "%s: creating text of %d", __FUNCTION__, len + VARHDRSZ);
#endif
        text *text_out = palloc (len + VARHDRSZ);
        SET_VARSIZE(text_out, len + VARHDRSZ);
#if DEBUG
        elog(WARNING, "%s: copying over %d", __FUNCTION__, len);
#endif
        memcpy((void *) VARDATA(text_out), string, len);
        return text_out;
}


PG_FUNCTION_INFO_V1(mule_tex);

Datum mule_tex(PG_FUNCTION_ARGS);

Datum
mule_tex(FunctionCallInfo fcinfo)
{
   text* text_in;
   text* text_out;
   int len;
   char* s;

   /* mmc: so we can be non-strict. */
   if (PG_ARGISNULL(0)) {
           PG_RETURN_NULL();
   };

   // text_in = (text *) fcinfo->arg[0];
   text_in = PG_GETARG_TEXT_P(0);
   s = mule_to_tex((char*) VARDATA(text_in),
                      VARSIZE(text_in)- VARHDRSZ,
                      &len);
#if DEBUG
   elog(WARNING, "%s: creating text of %d", __FUNCTION__, len);
#endif
   text_out = make_text(s, len);
   //   free(s);

#if DEBUG
   elog(WARNING, "%s: done", __FUNCTION__);
#endif

   PG_RETURN_TEXT_P(text_out);
}



/* wrapper around  `iconv'.
   returns the lenght of the converted string (result in utf8).
   Possibly calls PG specific Log functions in case of problems. */

/* destination _must_ be long enough (twice...)    we accept only  latinX as FROM */
/* string + len ->  */
static inline int
do_convert_to_utf8 (const char* from_coding, char* source, size_t len,
                    char* destination, int dest_len)
{


#if debug
  elog(WARNING, "converting %s: %lu chars", from_coding, len);
#endif

  iconv_t ico =  iconv_open("utf8", from_coding);
  size_t left = dest_len;

  int res = iconv(ico,
                  ((char**) &source),  &len,
                  &destination,  &left);

  if (res == -1)
    {
      switch errno {
          /* error!! */
        case EILSEQ:
          elog(ERROR, "iconv failed: invalid multibyte sequence on input (cyrillic-iso)");
          break;
        case E2BIG:
          elog(ERROR, "iconv failed: the output too big (insufficient space malloc-ed");
          break;
        case EINVAL:
          elog(ERROR, "iconv: An incomplete multibyte sequence has been encountered in the input.");
        default:
          elog(ERROR, "iconv failed: unspecified error");
          break;
        }
    };
  iconv_close(ico);
  return (dest_len - left);
}



/* Convert (old) emacs mule encoding into UTF8.
 * string contains \201 ->
 * string contains \202 ->
 * string contains \214 ->
 *
 *  If it's not MULE (all >128 prefixed by one of these TAGS) then we treat is as UTF-8!
 *
 *  returns palloc-ed byte sequence (not 0-terminated)!
 */

static char*
mule_2_utf8(const char* source, size_t len, size_t* result_len)
{
  int i_offset = 0;                /* in the `source' */
  /*  */
  char* store;
  int store_capacity;
  /*  */
  char* translated;
  int translated_size;

  int d_offset = 0;            /* in the `translated' ! */
  unsigned char last_coding = MULE_BOUNDARY + 2;

  if (len == 0) {
    *result_len = 0;
    return NULL;
  };

  /* Mule is verbose: 2 bytes per glyph.
     Here we keep the 1byte-per-glyph: */
  store_capacity = (len / 2) + 1;
  store = alloca(store_capacity);

  /* fixme: if the source is in native encoding (we know which one), we need 2 *  */
  translated_size = 2 * len;   /* mule string is not shorter than utf8 ! */
  translated = palloc (translated_size); /* utf 8 uses max. 2 bytes. */

  /* Walk the source string: */

  while (i_offset < len) {

    /* skip over & copy ascii chars */
    while ((i_offset < len) &&
           !CHAR_IS_MULE_MARKER(source[i_offset])) {
      translated[d_offset++] = source[i_offset++];
    }

    if (i_offset < len) {
      /* we touched MULE: but not necessary!!
         Maybe it's non-ascii, like latin-2 diacritics! */
      size_t store_len = 0;
      unsigned char coding = source[i_offset];
      const char* coding_name;

#if debug
      elog(WARNING, "found next boundary, %d", coding);
#endif

      if (coding > change_cmd_limit + MULE_BOUNDARY) {

        /* take it as tagged by the previous one !    hmmm ?  if none ? latin2 ?*/
        store[store_len++] = source[++i_offset];

        {
          /* report */
          char* copy_string = alloca(len + 1);
          strncpy(copy_string, source, len);
          copy_string[len] = 0;
          elog(WARNING, "invalid MULE tag! %d -> using %d (%s)", coding,
               last_coding, copy_string);
        }

        // coding = last_coding;

        /* [15 apr 06]
           In this case it probably is already an UTF string!
           I suggest exiting w/ plain copy of source string! */
        {
          memcpy(translated, source, len);
          *result_len = len;
          return translated;
        }
      }

      last_coding = coding;

      /* skip over MULE chars w/ the same TAG: */
      while ((store_len < store_capacity) /* fixme: useless test! */
             && (i_offset < len)
             && (source[i_offset] == coding))
        {
          store[store_len++] = source[++i_offset];

          if (i_offset < len)
            i_offset++;
          else
            elog(ERROR, "the string ends after a MULE tag!");
        }

      coding_name = iconv_encoding[coding - MULE_BOUNDARY - 1];
#if debug
      elog(WARNING, "converting %s (%d): %d chars", coding_name, coding - MULE_BOUNDARY -1, len);
#endif

      /* now, iconv the store of store_len */
      d_offset += do_convert_to_utf8(coding_name,
                                     store, store_len,
                                     translated + d_offset, translated_size - d_offset);
    };
  };

  *result_len = d_offset;
  return translated;
}






PG_FUNCTION_INFO_V1(mule_utf8);
Datum mule_utf8(PG_FUNCTION_ARGS);

Datum
mule_utf8(PG_FUNCTION_ARGS)
{
  text* text_in;
  text* text_out;
  size_t len;
  char* s;

  /* fixme:      so we can be non-strict !!*/
  if (PG_ARGISNULL(0)){
    PG_RETURN_NULL();
  }
  text_in = PG_GETARG_TEXT_P(0);

  s = mule_2_utf8( (char*) text_in + VARHDRSZ,
                      VARSIZE(text_in)- VARHDRSZ,
                      &len);
  text_out = make_text(s, len);
  PG_RETURN_TEXT_P(text_out);
};





/* mmc: Ah nice trick: We don't care if it's MULE or cyrillic-ISO, since we can map
   both to the latter by simply removing the Mule tag */

/* mule_to_utf:  emacs-mule  cyrillic only string (remove 0214 and get cyrillic-iso text) -> utf8  */


static char*
mule_page_to_utf8(const unsigned char* s, size_t len,
                  size_t* destination_len, unsigned char mule_tag,
                  const char* coding)
{
  /* copy stripping the MULE selector (ex. \0214) (so we  get the pure latinN/cyrillic-iso text) */
  /* Note: text is not 0-terminated !!
     clone IS.  */

  char *clone = alloca(len + 1);
  char *d = clone;

  int i;
  for(i = 0; i< len ;i++) {
    /*  '\\0214'   */
    if ((*s == mule_tag) ||
        (*s == 129))     /* fixme:  I add this...   */
      s++;
    else
      *d++ = *((char*) s++);
  };

  *d = 0;
  len = d - clone;
  {
    size_t outbytesleft = 2 * len + 1;

    /* fixme: Should be allocate on the stack! */
    char* utf = malloc(outbytesleft);
    *destination_len = do_convert_to_utf8(coding, clone, len, utf, outbytesleft);
    return utf;
  }
}



PG_FUNCTION_INFO_V1(cyrillic_utf8);
Datum cyrillic_utf8(PG_FUNCTION_ARGS);

Datum
cyrillic_utf8(PG_FUNCTION_ARGS)
{
  text* text_in;
  text* text_out;
  size_t len;
  char *s;

  /* fixme:      so we can be non-strict !!*/
  if (PG_ARGISNULL(0)){
    PG_RETURN_NULL();
  }

  text_in = PG_GETARG_TEXT_P(0);
  s = mule_page_to_utf8((unsigned char*) text_in + VARHDRSZ,
                           VARSIZE(text_in)- VARHDRSZ,
                           &len,
                           140, "cyrillic");
  /* compose the result: */
  text_out = make_text(s, len);
  free(s);
  PG_RETURN_TEXT_P(text_out);
}


PG_FUNCTION_INFO_V1(latin2_utf8);
Datum latin2_utf8(PG_FUNCTION_ARGS);

Datum
latin2_utf8(PG_FUNCTION_ARGS)
{
        text* text_in;
        text* text_out;
        size_t len;
        char* s;

        /* fixme:      so we can be non-strict !!*/
        if (PG_ARGISNULL(0)){
                PG_RETURN_NULL();
        }
        text_in = PG_GETARG_TEXT_P(0);
        s = mule_page_to_utf8((unsigned char*) text_in + VARHDRSZ,
                                         VARSIZE(text_in)- VARHDRSZ,
                                         &len,
                                         130, "latin2");

        text_out = make_text(s, len);
        free(s);
        PG_RETURN_TEXT_P( text_out);
}



PG_FUNCTION_INFO_V1(latin1_utf8);
Datum latin1_utf8(PG_FUNCTION_ARGS);

Datum
latin1_utf8(PG_FUNCTION_ARGS)
{
  text* text_in;
  text* text_out;
  size_t len;
  char* s;

  /* fixme:      so we can be non-strict !!*/
  if (PG_ARGISNULL(0)){
    PG_RETURN_NULL();
  }

  text_in = PG_GETARG_TEXT_P(0);
  s = mule_page_to_utf8((unsigned char*) text_in + VARHDRSZ,
                           VARSIZE(text_in)- VARHDRSZ,
                           &len,
                           129, "latin1");

  text_out = make_text(s, len);
  free(s);
  PG_RETURN_TEXT_P(text_out);
}





/* fixme: this should return bytea  not text (in DB's native encoding) */
PG_FUNCTION_INFO_V1(utf8_cyrillic);
Datum utf8_cyrillic(PG_FUNCTION_ARGS);

Datum
utf8_cyrillic(PG_FUNCTION_ARGS)
{
  text* text_in;
  char* source;
  size_t len;
  char* destination_p;
  char* destination;
  int dest_len;
  int res;
  size_t left;
  const char* to_coding = "8859_5"; /* latin5 cyrillic*/

  /* fixme:      so we can be non-strict !!*/
  if (PG_ARGISNULL(0)){
    PG_RETURN_NULL();
  }

  text_in = PG_GETARG_TEXT_P(0);
  source = (char*) text_in + VARHDRSZ; /* unsigned */
  len = VARSIZE(text_in)- VARHDRSZ;

  dest_len = len + 1;
  destination  = (char*) alloca(dest_len); /* alloca */
  destination_p = destination;             /* this can be moved! */
  memset(destination, 0, dest_len);


  /* fixme:  take the coding as argument! */
  {
    iconv_t ico =  iconv_open(to_coding, "utf8");
    left = dest_len;
    if (! ico)
      elog(WARNING, "iconv: couldn't open the iconv object");

#if debug
    elog(WARNING, "iconv: source len: %d  left: %d", len, left);
    elog(WARNING, "iconv: source: >%5.10s<", source);
#endif
    res = iconv(ico,
                ((char**) &source),  &len,
                &destination_p, &left);
#if debug
    elog(WARNING, "iconv: res: %d destination len: %d  >%5.10s< %d %d %d",
         res, dest_len - left, destination,
         (int) destination[0],
         (int) destination[1],
         (int) destination[2]);
#endif

#if debug
    /* dump: */
    {
      int i;
      for (i = 0; i < dest_len - left; i++)
        elog(WARNING, "%d %x",(int) destination[i], (int) destination[i]);
    }
#endif


    if (res == -1) {
      switch errno {
          /* error!! */
        case EILSEQ:
          elog(ERROR, "iconv failed: invalid multibyte sequence on input utf8");
          break;
        case E2BIG:
          elog(ERROR, "iconv failed: the output too big (insufficient space malloc-ed");
          break;
        case EINVAL:
          elog(ERROR, "iconv: An incomplete multibyte sequence has been encountered in the input.");
        default:
          elog(ERROR, "iconv failed: unspecified error");
          break;
        }
    };
    iconv_close(ico);
  }
  {
    text* text_out = make_text(destination, (dest_len - left));
    /* free(destination); ... alloca!*/
    PG_RETURN_TEXT_P(text_out);
  }
};

extern PGDLLIMPORT const Pg_finfo_record * pg_finfo_demule(void);

const Pg_finfo_record *
pg_finfo_demule (void)
{
  static const Pg_finfo_record my_finfo = { 1 };
  return &my_finfo;
}
extern int no_such_variable;

// PG_FUNCTION_INFO_V1(demule);
Datum demule(PG_FUNCTION_ARGS);

/* Remove tags (bytes) used as code-page switchers by Mule encoding.
 * Why? */
Datum
demule(PG_FUNCTION_ARGS)
{
  char skip_chars[] = "\201\214\202\212";
  text* text_in;
  char* in;
  char* o;
  char* out;
  int j = 0;
  int i;
  int len;


  /* remove  */
  if (PG_ARGISNULL(0)){
    PG_RETURN_NULL();
  }

  text_in = PG_GETARG_TEXT_P(0);
  in = VARDATA(text_in);

  len = VARSIZE(text_in)- VARHDRSZ;
  out = alloca(len);
  o = out;


  /* copy: */
  for(i=0;i<len; i++) {
    if (NULL == strchr(skip_chars,in[i]))
      {
#if 1
        *o++ = in[i];
        j++;
#else
        out[j++] = in[i];
#endif
      }
  }
  {
    text* text_out =  make_text (out, j);
    PG_RETURN_TEXT_P(text_out);
  }
};




// standalone
#if STANDALONE

main ()
{
#define  test_len  10
  text* test_text;
  text* test_result;

  test_text=malloc (test_len+sizeof (int32));
  test_text->vl_len=test_len;
  memcpy (test_text->vl_dat,
	  'å‰å‘å€å–å“å‰å–ÇπÅÈÇ¯ÅÌÇπÅÈÇ¯ÅÈ',test_len);
  test_result=mule_tex (test_text);
}
#endif /* STANDALONE */
