#ifndef UTILS_H
#define UTILS_H

double getTime_usec() {
   struct timeval tp;
   gettimeofday(&tp, NULL);
   return (double) tp.tv_sec * 1E6 + (double)tp.tv_usec;
}
unsigned long string_to_bytes(char* bstring){
   int last = strlen(bstring) - 1;
   char end;
   long scalar = 1;
   end = bstring[last];
   printf("data = %s %c\n", bstring, end);
   switch (end){
      case 'B':
         bstring[last] = '\0';
         scalar = atoi(bstring);
         break;
      case 'K':
         bstring[last] = '\0';
         scalar = atoi(bstring) << 10;
         break;
      case 'M':
         bstring[last] = '\0';
         scalar = atoi(bstring) << 20;
         break;
      case 'G':
         bstring[last] = '\0';
         scalar = atoi(bstring) << 30;
         break;
      default:
         scalar = atoi(bstring);
   }
   return scalar;
}

char* bytes_to_string (double b){
   double rate = b;
   int pow = 0;
   while (rate > 1024){
      rate /= 1024.0;
      pow++;
   }
   char *buf = (char*) malloc(sizeof(char)*30);
   char blue;
   switch (pow){
      case 0:
         blue = '\0';
         break;
      case 1:
         blue = 'K';
         break;
      case 2:
         blue = 'M';
         break;
      case 3:
         blue = 'G';
         break;
   }
   sprintf(buf, "%f %c", rate, blue);
   return buf;
}

#endif
