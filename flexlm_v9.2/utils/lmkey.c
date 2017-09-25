#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lmclient.h"
#include "lm_attr.h"

VENDORCODE m_VendorCode;
unsigned long m_VendorCode5;
int update =0;

struct
{
   char   value[80];
   char   key[80];
   short  demo;
   short  purchase;
}file_info[8]={
      "ENCRYPTION_SEED1","               ",0,1,
      "ENCRYPTION_SEED2","               ",0,1,
      "VENDOR_KEY1",     "               ",1,1,
      "VENDOR_KEY2",     "               ",1,1,
      "VENDOR_KEY3",     "               ",1,1,
      "VENDOR_KEY4",     "               ",1,1,
      "VENDOR_KEY5",     "               ",1,1,
      "VENDOR_NAME",     "               ",0,1};


void  romptforInput_(int i);
int findToken(char *str);
int initialize( const char *ENC1,  const char *ENC2,  const char *vk1,
                    const char *vk2, const char *vk3, const char *vk4,
		                                   const char *vk5, char *VN);

main( int argc, char *argv[])
{
  FILE   *inpf;
  FILE   *outf;
  char   lineinfo[256];
  char   *retval;
  char   *def;
  char   *firsttok;
  char   installtype[80]; 
  char   copystr[256];
  char   tmpstr[256];
  const  char *newlmcode="machind/lm_code.h";
  const  char *oldlmcode="machind/lm_code.bak";
  int    i=0,n=0 ;
  int    foundone = 0;
  int    goodkeys = 0;

  
  if(argc == 1) 
  {
     printf("Is this a DEMO install? (y) or (n), (q)uit  ");
     gets(installtype); 

     while((installtype[0] == '\0') ||
             ((installtype[0] != 'q') && (installtype[0] != 'Q') 
	             && (installtype[0] != 'n') && (installtype[0] != 'N')
		         && (installtype[0] != 'Y')  && (installtype[0] !='y')))
     {
        printf("Invalid Input\n");
        printf("Is this a demo install? (y) or (n), (q)uit  ");
        gets(installtype); 
     }

     if((installtype[0] == 'q') || (installtype[0] == 'Q'))
     {
        printf("Exiting, lm_code.h will not be updated!\n");
        exit(0);
     }

     if((*installtype == 'y') || (*installtype == 'Y'))
     {
        strcpy(file_info[0].key,"0x12345678");
        strcpy(file_info[1].key,"0x87654321");
        strcpy(file_info[7].key,"demo");
     }

     while(!goodkeys)
     {
         for(i = 0;i < 8; i++)
         {
            if(((*installtype == 'y') || (*installtype == 'Y')) 
	                                   && (file_info[i].demo ))
            {
               PromptForInput_(i);
	    }
	    else
	    if((*installtype == 'n') || (*installtype == 'N'))
	    {
	        PromptForInput_(i);
	    }
         }	
     goodkeys = initialize(file_info[0].key, file_info[1].key, file_info[2].key,
                     file_info[3].key, file_info[4].key, file_info[5].key,
                                         file_info[6].key, file_info[7].key);
     }
  }
   else
   {
     if((argc > 1) && ((strcmp(argv[1], "YES") == 0) ||(strcmp(argv[1], "NO") == 0))) 
     {
         strcpy(file_info[0].key,"0x12345678");
         strcpy(file_info[1].key,"0x87654321");
         strcpy(file_info[7].key,"demo");
     }
     else
     {
        printf("incorrect usage: update [YES] [NO] \n");
        exit(0);
     }

     while(!goodkeys)
     {
         for(i = 0;i < 8; i++)
         {
            if((strcmp(argv[1], "YES") ==  0) && (file_info[i].demo )) 
            {
               PromptForInput_(i); 
            }
            else
            if(strcmp(argv[1], "NO") == 0) 
            {
               PromptForInput_(i); 
            }
         }
         goodkeys = initialize(file_info[0].key, file_info[1].key,
	             file_info[2].key, file_info[3].key, file_info[4].key,
	                 file_info[5].key, file_info[6].key, file_info[7].key);
      }
    }
   inpf= fopen("machind/lm_code.h","r");
   if(inpf == NULL)
   {
      printf("Unable to open lm_code.h\n");
      exit(0);
   }

  outf= fopen("machind/lm_code.bak","w");
  if(outf == NULL)
  {
     printf("Unable to open lm_code.bak\n");
     exit(0);
  }

   while((retval=fgets(lineinfo,255,inpf))!= NULL)
   {
      strcpy(copystr,lineinfo);
      if((lineinfo[0] == '#') &&
        ((firsttok=strtok(lineinfo," ")) != NULL) &&
	((def=strtok(NULL," ")) != NULL))
      {
	     
	 if((foundone=findToken(def)) != -1)
	 {
	      fputs(lineinfo,outf);
	      fputs(" ",outf);
	      fputs(file_info[foundone].value,outf);
	      fputs(" ",outf);
	      if(foundone == 7)
	      {
	        fputs("\"",outf);
	        fputs(file_info[foundone].key,outf);
	        fputs("\"\n",outf);
	      }
	      else
	      {
	        fputs(file_info[foundone].key,outf);
	        fputs("\n",outf);
	      }
	 }
	 else
	 {
           	fputs(copystr,outf); 
	 }
      }
      else 
      {
        fputs(copystr,outf); 
      }

  }

     if(update)
       printf("lm_code.h updated successfully!\n");

     fclose(outf);
     fclose(inpf); 
     remove(newlmcode);
     rename(oldlmcode, newlmcode);
     return(0);
 }

int PromptForInput_(int i)
{
  char  input[80];
  char  *retval;
  int   numargs=0;
  int   strsize;
  int   k;

  if((strcmp(file_info[i].value,"ENCRYPTION_SEED1") == 0) ||
       (strcmp(file_info[i].value,"ENCRYPTION_SEED2") == 0))
  {
     printf("Enter %s or (q)uit\t", file_info[i].value);
     gets(input);
     while(*input == '\0')
     {
        printf("\nEnter %s or (q)uit\t", file_info[i].value);
        gets(input);
     }
  }
  else
  {
     printf("Enter %s or (q)uit\t\t", file_info[i].value);
     gets(input);
     while(*input == '\0')
     {
        printf("Enter %s or (q)uit\t\t", file_info[i].value);
        gets(input);
     }
  }
  if((input[0] == 'Q') || (input[0] == 'q'))
  {
     printf(" Exiting, lm_code.h will not be updated!\n");
     exit(0);
  }
 
  if(strcmp(file_info[i].value,"VENDOR_NAME") == 0)
  {
     int VN_Size = strlen(input);

     for(k=0; k <= (VN_Size - 1); k++)
     {
       if(!islower(input[k]))
       {
          input[k]=tolower(input[k]);
       }
     }
  }
  strcpy(file_info[i].key,input);
  return(0);
}


int initialize(const char *ENC1, const char *ENC2,  const char *VK1, 
                   const char *VK2,  const char *VK3,  const char *VK4,
                        		      const  char *VK5, char *VN)
{
   int intRet; 
   char cont[80]; 

   LM_HANDLE *m_Job;
   char m_VendorName[MAX_VENDOR_NAME]; 

   m_VendorCode.data[0]= (unsigned long) strtoul(ENC1,(char**)NULL,0); 
   m_VendorCode.data[1]= (unsigned long) strtoul(ENC2,(char**)NULL,0); 
   m_VendorCode.keys[0]= (unsigned long) strtoul(VK1,(char**)NULL,0); 
   m_VendorCode.keys[1]= (unsigned long) strtoul(VK2,(char**)NULL,0); 
   m_VendorCode.keys[2]= (unsigned long) strtoul(VK3,(char**)NULL,0);
   m_VendorCode.keys[3]= (unsigned long) strtoul(VK4,(char**)NULL,0);
   m_VendorCode5 = (unsigned long) strtoul(VK5,(char**)NULL,0);
   strcpy(m_VendorName,VN);

   m_VendorCode.type = VENDORCODE_5;
   m_VendorCode.data[0] ^= m_VendorCode5;
   m_VendorCode.data[1] ^= m_VendorCode5;
   m_VendorCode.flexlm_version = FLEXLM_VERSION;
   m_VendorCode.flexlm_revision = FLEXLM_REVISION;

   intRet = lc_init(
          		      NULL,
       		      m_VendorName,
		      &m_VendorCode,
		      &m_Job );
									    
									    
  if(intRet)
  {
    printf("The keys you entered are Invalid!\n");
    printf("Would you like to re-enter them or exit, (y)es (q)uit\t");
    gets(cont);


    while((cont[0] == '\0') ||
             ((cont[0] != 'q') && (cont[0] != 'Q') 
		         && (cont[0] != 'Y')  && (cont[0] !='y')))
    {
       printf("Invalid Input\n");
       printf("Would you like to re-enter them or exit, (y)es (q)uit\t");
       gets(cont); 
    }

    if((*cont == 'y') || (*cont == 'Y'))
     return(0);
    else
    if((*cont == 'q') || (*cont == 'Q'))
    {
      printf("Exiting, lm_code.h will not be updated!\n");
      exit(0);
    }
  }
  else
  { 
    update =1;
    return(1);
  }
										}
		


int findToken(char *str)
{
int n;
	if(str != NULL)
	{
	 for(n=0;n < 8;n++)
	 {
            if(strcmp(str,file_info[n].value) == 0) 
	    {
		return(n);
	    }
	  }
	}else
	{
		return -1;
	}

return -1;
}
