



char*
cyrillic_to_utf8(const char* string, size_t len, size_t* destination_len)
{

    return mule_page_to_utf8(string, len, destination_len, 140, "cyrillic")


#undef ONE_PASS
#define ONE_PASS

#ifdef ONE_PASS

   /* copy stripping the \0214 chars:   (so get the pure cyrillic-iso )
    *
    * */


   /*
    *  string is not 0-terminated !!  clone IS.
    *  */
   
   unsigned char* clone = alloca(strlen (string) + 1); /* malloc */
   const unsigned char *s = string;
   unsigned char* d = clone;


#if 0   
   while (*s)
      {
         /*  '\\0214'   */
         if (*s == 140)
            s++;
         else
            *d++ = *s++;
      };
   *d = 0;
#else



   int i;
   for(i=0; i< len ;i++)
      {
         /*  '\\0214'   */
         if (*s == 140)
            s++;
         else
            *d++ = *s++;
      };
   *d = 0;
   
#endif
   string = clone;

   len = d - clone;         /*  - 1     len w/o the terminating 0 */


#endif


   
   {
      /* string + len ->
       * */
      const char* s = string;
      
      iconv_t ico =  iconv_open("utf8", "cyrillic");
   
      size_t left = len;


      size_t outbytesleft = 2 * strlen (clone) + 1;
      *destination_len = outbytesleft;
      
      char* utf = malloc( outbytesleft);
      char* utf_p = utf;

#if 0
      while ((left > 0) && (outbytesleft > 0))
         {
            /* i misunderstood it: */
#endif            
            int res = iconv(ico,
                  ((char**) &s),  &left,
                  &utf_p, &outbytesleft);

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
                     elog(ERROR, "An incomplete multibyte sequence has been encountered in the input.");
                  default:
                     elog(ERROR, "iconv failed: unspecified error");
                     break;
                  }
               };
#if 0
         };
#endif      

      
      *utf_p = 0;
      iconv_close(ico);

#ifdef ONE_PASS

#if 0      
      free(clone);   /* i use alloca*/
#endif      
#endif
      *destination_len -= outbytesleft;
      return utf;
   }
}
