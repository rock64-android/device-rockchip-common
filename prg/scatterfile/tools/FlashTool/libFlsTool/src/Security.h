/*
 *       Copyright (C) 2012-2014 Intel Mobile Communications GmbH
 *
 *            Sec Class: Intel Confidential (IC)
 *
 *
 * This document contains proprietary information belonging to IMC.
 * Denmark A/S. Passing on and copying of this document, use and communication
 * of its contents is not permitted without prior written authorisation.
 *
 * Description:
 *  Helper functions for mainly filesystem related tasks
 *
 * Revision Information:
 *  $File name:  /msw_tools/FlashTool/libFlsTool/src/Security.h $
 *   Resonsible:
 *   Comment:
 *
 */
#ifndef IFWD_secH
#define IFWD_secH

#include "OS.h" 
#include "header.h" /* defines "SecurityStructType" */

/****************************************************************************
 *                          FUNCTIONS TO EXPORT                             *
 ****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
* Function:... uint8_t DecryptSecPack(SecurityStructType *Security, char **status_text)
* Parameters:.
*     In/Out  :   SecurityStructType *Security       : pointer to sec pack for investigation/modification
*     In/Out  :   char **status_text             : On fail this is set to a description of why it failed
*
* Returns:.... The function returns:
*              TRUE on success, FALSE if there is an error
* Description:
*
* Created:....
* Modified:...
*              Modifications note.
******************************************************************************/
extern uint8_t DecryptSecPack(SecurityStructType *Security, char **status_text);

#ifdef __cplusplus
}
#endif

#endif  /* from #ifndef IFWD_secH */

