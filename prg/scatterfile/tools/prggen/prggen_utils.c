// BEGIN CCCT SECTION
/* -------------------------------------------------------------------------
 Copyright (C) 2013-2014 Intel Mobile Communications GmbH
 
      Sec Class: Intel Confidential (IC)
 ----------------------------------------------------------------------
 Revision Information:
    $File name:  /mhw_drv_src/memory_management/mem/scatterfile/tools/prggen/prggen_utils.c $
 ----------------------------------------------------------------------
 by CCCT (0.12e)
 ---------------------------------------------------------------------- */
// END CCCT SECTION

// prggen_utils.c

static void string_lower(char *dst, const char *src)
{
  while (1)
  {
    if (*src >= 'A' && *src <= 'Z')
      *dst = *src - 'A' + 'a';
    else
      *dst = *src;

    if (*src == 0)
      break;

    ++dst; ++src;
  }
}

static void get_hex_details(char *filename,  U32 *s_addr,  U32 *length)
{
  char line[50];
  char line_prev[50];
  int no_sect=0;
  char g_hex_file[300];
  U32 offset=0;
  FILE *fp_hex;

  snprintf(g_hex_file,sizeof(g_hex_file), "%s/%s", g_boot_file_loc, filename);
  fp_hex = fopen(g_hex_file,"r");
  if (fp_hex != NULL)
  {
    while (fgets(line, sizeof(line), fp_hex) != NULL )
    {
      if((line[7] == '0') && (line[8] == '5'))
      {
        if(no_sect == 0)
        {
          sscanf(&line[9], "%8x", s_addr);
        }
        no_sect++;
      }
      else if((line[7] == '0') && (line[8] == '1'))
      {
        sscanf(&line_prev[3], "%4x", &offset);
        sscanf(&line_prev[1], "%2x", length);
      }
      strncpy(line_prev,line,sizeof(line));
    }
    *length += offset + ((no_sect-1)*0x10000);
    *length = ALIGN_2K(*length);
    fclose(fp_hex);
  }
  else
  {
    printf("Could not extract details from the hex file %s\n", filename);
    exit(3);
  }
}
