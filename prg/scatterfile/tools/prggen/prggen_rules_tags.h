// BEGIN CCCT SECTION
/* -------------------------------------------------------------------------
 Copyright (C) 2013 Intel Mobile Communications GmbH
 
      Sec Class: Intel Confidential (IC)
 ----------------------------------------------------------------------
 Revision Information:
    $File name:  /mhw_drv_src/memory_management/mem/scatterfile/tools/prggen/prggen_rules_tags.h $
 ----------------------------------------------------------------------
 by CCCT (0.12e)
 ---------------------------------------------------------------------- */
// END CCCT SECTION

// prggen_rules_tags.h
// tags rules file

  #define FMT_HEX2FLS_TOOL  "%s --replace "
  #define FMT_PRGFILE       "--prg %s/%s.prg "
  #define FMT_SIGN_SCRIPT   "--script %s/%s "
  #define FMT_OUTPUT        "--output %s/%s.fls "
  #define FMT_INJECT_EBL    "--ebl %s/ebl.fls "
  #define FMT_INJECT_PSIR   "--psi %s/psi_ram.fls "
#if defined(INI_FILE)
  #define FMT_INI_FILE      "--zip %s/%s "
#else
  #define FMT_INI_FILE      "%s"
#endif  /* INI_FILE */
  #define FMT_TAG           "--tag %s "
  #define FMT_INPUT_P       "%s/%s"
  #define FMT_INPUT         "%s "
  #define FMT_VERSION       "--meta %s/%s "

  #define FMT_STANDARD      FMT_HEX2FLS_TOOL FMT_PRGFILE FMT_INJECT_EBL FMT_INJECT_PSIR FMT_INI_FILE FMT_TAG
  #define FMT_NO_INJECT     FMT_HEX2FLS_TOOL FMT_PRGFILE FMT_TAG
  #define FMT_STDIO         FMT_OUTPUT FMT_INPUT

  #define V_HEX2FLS_TOOL    g_p_hex2fls_tool
  #define V_PRGFILE(X)      p_input_path, X
  #define V_SIGN_SCRIPT(X)  g_p_sign_script, X
  #define V_OUTPUT(X)       p_output_path, strtok(X, ":")
  #define V_OUTPUT_BOOT(X)  p_input_path, X  
  #define V_OUTPUT_ID(ID)   V_OUTPUT(g_partition_info[PARTITION_ID_##ID].d.output)
  #define V_OUTPUT_BOOTID(ID) V_OUTPUT_BOOT(g_partition_info[PARTITION_ID_##ID].d.output)
  #define V_INJECT_EBL      p_input_path
  #define V_INJECT_PSIR     p_input_path
#if defined(INI_FILE)
  #define V_INI_FILE        g_p_sign_script, g_ini_file
#else
  #define V_INI_FILE        ""
#endif  /* INI_FILE */

  #define V_TAG(X)          X
  #define V_TAG_AUTO(ID)    V_TAG(g_partition_info[PARTITION_ID_##ID].d.fls_tag)
  #define V_INPUT(X)        X
  #define V_INPUT_BOOT(X)   g_p_sign_script, X
  #define V_VERSION(P,F)    P, F
  #define V_VERSION_BOOT(F) V_VERSION(g_p_sign_script, F)

#define V_STANDARD(ID)     V_HEX2FLS_TOOL, V_PRGFILE("modem_sw"), V_INJECT_EBL, V_INJECT_PSIR, V_INI_FILE, V_TAG_AUTO(ID)
#define V_NO_INJECT(ID)    V_HEX2FLS_TOOL, V_PRGFILE("modem_sw"), V_TAG_AUTO(ID)
#define V_STDIO(ID)        V_OUTPUT(g_partition_info[PARTITION_ID_##ID].d.output), g_p_hex2fls_input_files

/* hex2fls [prg] -s [sign script] --output [out] --ebl [ebl.fls] --psi [psi_ram.fls] --zip [ini] --tag [tag] file-list */
// "%s --prg %s/modem_sw.prg -s %s/psi_flash.xor_script.txt --output %s/%s.fls --ebl %s/ebl.fls --psi %s/psi_ram.fls --zip %s/%s --tag %s %s %s/slb.hex %s/psi_flash.hex\n",
