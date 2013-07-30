/* I keep some static variables (to speed up?)
 * which are shared/accessed/processed by the functions.
 */





/* old version: */

#if        0


static char *result;            /* |XXXXX------| */
static int res_len=0;           /*             ^ */
static int res_offset=0;        /*       ^       */


/* i dynamically allocate the result string.  Here we add a character: */
static int
append (char c)
{
   if (res_offset==res_len){
      // Should be (lenght of rest + reserve)
      res_len+=10;
      result=repalloc (result,res_len);
      // if result==NULL;
   }
   result[res_offset++]=c;
}


/* add to result  STRING */
static int
append_string (char* string)
{
  int len=strlen (string);
  
  if ((res_offset+len)>=res_len){
    // Should be (lenght of rest + reserve)
    res_len=res_offset+len+10;
    result=repalloc (result,res_len);
    // if result==NULL;
  }
  memcpy (result+res_offset,string, len);
  res_offset+=len;
}

#endif










#if  0
/* old versions: */
text* 
mule_tex (text* text_in)
/* translate the pg TEXT_IN as mule-encoded string to  TeX encoded string (pg text) */
{

   

   unsigned char* string; // ="ŒäŒÔŒÛŒĞŒÒŒäŒĞ‚¹é‚øí‚¹é‚øé";

   int len;

   // important
   if (text_in==NULL) return NULL;


   len=text_in->vl_len-sizeof (int32);
   string=text_in->vl_dat;
#if 0
   elog(NOTICE, "mule->tex: %d",len);
#endif

  
   text_out=palloc (res_offset + sizeof (int32));
   text_out->vl_len=res_offset+sizeof (int32);
   memcpy (text_out->vl_dat,result,res_offset);

   //  pfree (result);
   // res_len=0;
   res_offset=0;
   return text_out;
}


#endif

