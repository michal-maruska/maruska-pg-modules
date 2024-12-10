/* #include <string.h> */
#include <postgres.h>
#include <varatt.h>
#include <fmgr.h>


#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif


/* prototypes ? */
void scale_to(int* x, int* y, int max);

PG_FUNCTION_INFO_V1(thumbsize);

/* MAX is maximum for both x and y! */
void
scale_to(int* width, int* height, int max)
{
        if ((*width == 0) || (*height == 0))
        {
                return;
        }
        else
        {
                /* one of the sizes will be max.
                 * Hence no need to use a temp variable. */

                if (*width > *height)
                {
                        *height = (*height * max) / *width;
                        *width = max;
                }
                else
                {
                        *width = (*width * max) / *height;
                        *height = max;
                }
        }
}


Datum thumbsize(PG_FUNCTION_ARGS);

Datum
thumbsize(PG_FUNCTION_ARGS)
{
  int height;
  int direction;
  int width;
  int ret = 0;

  /* fixme:      so we can be non-strict !!*/
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2)){
          PG_RETURN_NULL();
  };


  height = PG_GETARG_INT16(0);
  width  = PG_GETARG_INT16(1);
  direction = PG_GETARG_INT16(2);   /* fixme! boolean!  */

  scale_to(&width, &height, 100);

  /* so I have to call twice, to get the whole result? */
  ret = direction ? width : height;

  PG_RETURN_INT16( ret);
}




static
Datum
make_html_img(int numero, text* name, int width, int height, int maximum)
{
   /* fixme: I would like to accept the PG types! */


   char part1[30];
   char part2[50];
   int t1, t2, t;
   text* result_t;

   scale_to(&width, &height, maximum);

   sprintf(part1, "<img src=\"/f/%d/", numero);
   sprintf(part2, "/t/2\" width=\"%d\" height=\"%d\">",width, height);

   t1 = strlen(part1);
   t2 = strlen(part2);

   t = VARSIZE(name) + t1 + t2;

   result_t = palloc(t);
   SET_VARSIZE(result_t,t);


   /* concat the 3 strings: */
   memcpy((VARDATA(result_t)), part1, t1);
   memcpy((VARDATA(result_t)) + t1, VARDATA(name), VARSIZE(name) - VARHDRSZ);
   memcpy((VARDATA(result_t)) + t1 + VARSIZE(name) - VARHDRSZ,
          part2, t2);


   /* strcpy((VARDATA(result_t)), result); */

   PG_RETURN_TEXT_P(result_t);
}




PG_FUNCTION_INFO_V1(foto_img2);

/* This is a hack! In gauche I won't use it! */
Datum foto_img2(PG_FUNCTION_ARGS);

Datum
foto_img2(PG_FUNCTION_ARGS)
{
  int numero;
  text	*name;
  int height;
  int width;

  /* fixme:      so we can be non-strict !!*/
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2)){
    PG_RETURN_NULL();
  };


  numero = PG_GETARG_INT32(0);
  name = PG_GETARG_TEXT_P(1);

  height = PG_GETARG_INT16(2);
  width = PG_GETARG_INT16(3);

  return make_html_img(numero, name, width, height, 200);
}



PG_FUNCTION_INFO_V1(foto_img);

/* This is a hack! In gauche I won't use it! */
Datum foto_img(PG_FUNCTION_ARGS);


Datum
foto_img(PG_FUNCTION_ARGS)
{
  int numero;
  text	*name;
  int height;
  int width;


  /* fixme:      so we can be non-strict !!*/
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2)){
    PG_RETURN_NULL();
  };


  numero = PG_GETARG_INT32(0);
  name = PG_GETARG_TEXT_P(1);

  height = PG_GETARG_INT16(2);
  width = PG_GETARG_INT16(3);

  return make_html_img(numero, name, width, height, 100);
}

