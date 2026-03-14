unsigned int hash(const char* str, unsigned int len)
{
   const unsigned int fnv_prime = 0x811C9DC5;
   unsigned int hash = 0;
   unsigned int i    = 0;

   for (i = 0; i < len; i++)
   {
      hash *= fnv_prime;
      hash ^= str[i];
   }

   return hash;
}

unsigned int merge_hash(unsigned int left, unsigned int right)
{
    return (left * 0x811C9DC5) ^ right;
}
