#include <postgres.h>
#include <fmgr.h>


#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif
PG_FUNCTION_INFO_V1(max_of_2);
Datum max_of_2(PG_FUNCTION_ARGS);



Datum
max_of_2(PG_FUNCTION_ARGS)
{
   /* uintmax_t */
   int32 value1, value2;

   /* DatumGetInt64(PG_GETARG_DATUM(n)) */


   /* fixme:      so we can be non-strict !!*/
   if (PG_ARGISNULL(0) || PG_ARGISNULL(1)){
      PG_RETURN_NULL();
   }
   
   value1 = PG_GETARG_INT32(0);
   value2 = PG_GETARG_INT32(1);

   if (value1 > value2) {
      PG_RETURN_INT32(value1);
   }
   else {
      PG_RETURN_INT32(value2);
   }
}


PG_FUNCTION_INFO_V1(min_of_2);
Datum min_of_2(PG_FUNCTION_ARGS);

Datum
min_of_2(PG_FUNCTION_ARGS)
{
   int32 value1, value2;

   /* DatumGetInt64(PG_GETARG_DATUM(n)) */


   /* fixme:      so we can be non-strict !!*/
   if (PG_ARGISNULL(0) || PG_ARGISNULL(1)){
      PG_RETURN_NULL();
   }
   
   value1 = PG_GETARG_INT32(0);
   value2 = PG_GETARG_INT32(1);

   if (value1 < value2) {
      PG_RETURN_INT32(value1);
   }
   else {
      PG_RETURN_INT32(value2);
   }
}






/* sizeof(Datum)*/

PG_FUNCTION_INFO_V1(max_of_2big);
Datum max_of_2big(PG_FUNCTION_ARGS);


Datum
max_of_2big(PG_FUNCTION_ARGS)
{
   /* uintmax_t */
   int64 value1, value2;

   /* DatumGetInt64(PG_GETARG_DATUM(n)) */


   /* fixme:      so we can be non-strict !!*/
   if (PG_ARGISNULL(0) || PG_ARGISNULL(1)){
      PG_RETURN_NULL();
   }
   
   value1 = PG_GETARG_INT64(0);
   value2 = PG_GETARG_INT64(1);

   if (value1 > value2)
      PG_RETURN_INT64(value1);
   else PG_RETURN_INT64(value2);
}





/* sizeof(Datum)*/

PG_FUNCTION_INFO_V1(min_of_2big);
Datum min_of_2big(PG_FUNCTION_ARGS);

Datum
min_of_2big(PG_FUNCTION_ARGS)
{
   /* uintmax_t */
   int64 value1, value2;

   /* DatumGetInt64(PG_GETARG_DATUM(n)) */


   /* fixme:      so we can be non-strict !!*/
   if (PG_ARGISNULL(0) || PG_ARGISNULL(1)){
      PG_RETURN_NULL();
   }
   
   value1 = PG_GETARG_INT64(0);
   value2 = PG_GETARG_INT64(1);

   if (value1 < value2)
      PG_RETURN_INT64(value1);
   else PG_RETURN_INT64(value2);
}

