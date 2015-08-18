/**
 *  Copyright (C) 2011,2013 Intel Mobile Communications GmbH
 *
 *       Sec Class: Intel Confidential (IC)
 *
 *
 *  This document contains proprietary information belonging to IMC.
 *  Design A/S. Passing on and copying of this document, use and communication
 *  of its contents is not permitted without prior written authorisation.
 *
 *  Revision Information:
 *    $$File name:  /mhw_drv_src/nv_memory_driver/inc/ftle.h $
 *  Responsible: gjoerup
 *   Comment:
 *     Check In Performed by OptiCM
 */

#ifdef FTLE_PRESENT

/*! \file */

#ifndef FTLE_H
#define FTLE_H

#include "bastypes.h"

/**
@addtogroup ISPEC_FTLE_H
This part of the ftle module specifies the read, write and erase interface towards a logical partition in the embedded MMC or embedded SD card.
*/
/*@{*/

/**
  @brief Interface sector size.

  When the FTLe interface specifies a sector, then it is a data unit of this many bytes
*/
#define FTLE_DATA_SIZE_IN_BYTES        512

/**
  @brief Return value type for FTLE interface.

  This enumeration is used as return value type for the FTLE interfaces. Besides it is used
  when changeing a partition from RO to RW or vice versa.
*/
typedef enum
{
  ENUM_FTLE_OK,                      /**< Operation succeed. */
  ENUM_FTLE_SYSTEM_ERROR,            /**< Unspecified error occurred during operation. */
  ENUM_FTLE_INVALID_PARAMETERS,      /**< The input parameters are not legal. */
  ENUM_FTLE_DATA_INVALID,            /**< The data to access are invalid. */
  ENUM_FTLE_NO_MEM,                  /**< Not enough memory to perform operation. */
  ENUM_FTLE_RO,                      /**< Partition is (or set partition to) Read Only. */
  ENUM_FTLE_RW                       /**< Partition is (or set partition to) Read Only. */
} T_ENUM_FTLE_STATUS_TYPE;

/**
 @brief Interface for getting the access rights for a given partition.

 This function will find and return the access rights for a given partition.

 @param id [in] Id for the partition.

 @return ENUM_FTLE_RO                  If the partition is a read only partition.
 @return ENUM_FTLE_RW                  If the partition is a read write partition.
 @return ENUM_FTLE_INVALID_PARAMETERS  If the partition does not exist.
 @return ENUM_FTLE_SYSTEM_ERROR        If system is not initialized.
 */
T_ENUM_FTLE_STATUS_TYPE ftle_get_partition_access_rights(U32 id);

/**
 @brief Interface for setting access rights for a specific partition.

 This function will change the access rights for a specified partition.

 @param ro_access [in]  Access type requested: FTLE_RW if partition should be read/write, FTLE_RO if partition should be read only.
 @param id [in]     Id for the partition.

 @return ENUM_FTLE_OK                 If access is set correctly.
 @return ENUM_FTLE_INVALID_PARAMETERS If the partition does not exist.
 @return ENUM_FTLE_SYSTEM_ERROR       If the partition could not be started up, with the requested access rights.
 @return ENUM_FTLE_SYSTEM_ERROR       If the system is not initialized.
 */
T_ENUM_FTLE_STATUS_TYPE ftle_set_partition_access_rights(T_ENUM_FTLE_STATUS_TYPE ro_access, U32 id);

/**
 @brief Reads from flash, given a logical address and a number of bytes.

 This function reads data in a specific partition into the provided destination buffer.
 Please ensure that the buffer is 32 bit aligned for best performance.

 @param logical_addr [in] Start address to read from, given as the number of bytes from partition start.
 @param number [in]       Number of bytes to read.
 @param p_dist_addr [out]   Pointer to location where data should be stored.
 @param id [in]       Id for the partition where the sector is to be read from.

 @return ENUM_FTLE_OK      If data is read correctly.
 @return ENUM_FTLE_SYSTEM_ERROR       If ftle have not been initialized or partition has not been started.
 @return ENUM_FTLE_INVALID_PARAMETERS If the address or id is invalid.
 @return ENUM_FTLE_DATA_INVALID       If the data could not be read from device.
 */
T_ENUM_FTLE_STATUS_TYPE ftle_read_addr(U32 logical_addr, U32 number, U8 * p_dist_addr, U32 id);

/**
 @brief Reads data from flash, starting from a logical sector address in a partition.

 This function reads data in a specific partition into the provided destination buffer.
 Please ensure that the buffer is 32 bit aligned for best performance.

 @param sector_nr [in]  Sector to read.
 @param byte_nr [in]    Offset in the sector that have to be read from.
 @param number [in]     Number of bytes to read.
 @param p_dist_addr [out] Pointer to location where data should be stored.
 @param id [in]         Id for the partition where the sector is to be read from.

 @return ENUM_FTLE_OK      If data is read correctly.
 @return ENUM_FTLE_SYSTEM_ERROR       If ftle have not been initialized or partition has not been started.
 @return ENUM_FTLE_INVALID_PARAMETERS If the address or id is invalid.
 @return ENUM_FTLE_DATA_INVALID       If the data could not be read from device.
 */
T_ENUM_FTLE_STATUS_TYPE ftle_read_pageaddr(U32 sector_nr, U32 byte_nr, U32 number, U8 *p_dist_addr, U32 id);

/**
 @brief Reads information from eMMCSD to determine the value of erased memory content.

 This function reads the value of byte 181 of extended CSD register of eMMC, and always return 1 for eSD.

 @return 1      If erased content of the memory is 1.
 @return 0      If erased content of the memory is 0.
 */
U8 ftle_get_erased_mem_cont(void);

/**
 @brief Writes data in a partition, starting from a given logical address.

 This function writes data to a specific partition from the source address provided in the interface.
 Please ensure that the buffer is 32 bit aligned for best performance.

 @param logical_addr [in] Start address to write from, given as the number of bytes from partition start.
 @param number [in]       Number of bytes to write.
 @param p_src_addr [in]     Pointer to location where data that is to be written is situated.
 @param id [in]           Id for the partition where data is to be written.

 @return ENUM_FTLE_OK      If data is written correctly.
 @return ENUM_FTLE_SYSTEM_ERROR       If ftle have not been initialized or partition has not been started.
 @return ENUM_FTLE_SYSTEM_ERROR       If ftle did not succees writing the data to the device.
 @return ENUM_FTLE_DATA_INVALID       If the modificed data could not be read.
 @return ENUM_FTLE_RO                 If data cannot be written because the partition given is a read only partition.
 @return ENUM_FTLE_INVALID_PARAMETERS If the address or id is invalid.
 @return ENUM_FTLE_NO_MEM             If there was not enough memory to write the data.
 */
T_ENUM_FTLE_STATUS_TYPE ftle_write_addr(U32 logical_addr, U32 number, U8 * p_src_addr, U32 id);

/**
 @brief Writes data to flash, to a logical sector address.

 This function writes data to a specific partition from the source address provided in the interface.
 Please ensure that the buffer is 32 bit aligned for best performance.

 @param sector_nr [in] Sector to write to.
 @param byte_nr [in]   Offset in the sector that have to be written to.
 @param number [in]    Number of bytes to write.
 @param p_src_addr [in]  Pointer to location where data to be written are stored.
 @param id [in]        Id for the partition where the data is to be written.

 @return ENUM_FTLE_OK      If data is written correctly.
 @return ENUM_FTLE_SYSTEM_ERROR       If ftle have not been initialized or partition has not been started.
 @return ENUM_FTLE_SYSTEM_ERROR       If ftle did not succees writing the data to the device.
 @return ENUM_FTLE_DATA_INVALID       If the modificed data could not be read.
 @return ENUM_FTLE_RO                 If data cannot be written because the partition given is a read only partition.
 @return ENUM_FTLE_INVALID_PARAMETERS If the address or id is invalid.
 @return ENUM_FTLE_NO_MEM             If there was not enough memory to write the data.
 */
T_ENUM_FTLE_STATUS_TYPE ftle_write_pageaddr(U32 sector_nr, U32 byte_nr, U32 number, U8 *p_src_addr, U32 id);

/**
 @brief Deletes any number of bytes in a partition.

 This function will delete any number of bytes in a partition, given the logical address, as a byte offset from partition start and the number of bytes to delete.

 @param logical_addr [in] Start Address to delete from, given as the number of bytes from partition start.
 @param number [in]       Number of bytes to delete.
 @param id [in]           Id for the partition where data is to be deleted.

 @return ENUM_FTLE_OK                 If data is deleted.
 @return ENUM_FTLE_SYSTEM_ERROR       If ftle have not been initialized or partition has not been started.
 @return ENUM_FTLE_SYSTEM_ERROR       If ftle did not succeed to delete the data.
 @return ENUM_FTLE_DATA_INVALID       If the data to partial delete does not exists.
 @return ENUM_FTLE_RO                 If data cannot be deleted because the partition given is a read only partition.
 @return ENUM_FTLE_INVALID_PARAMETERS If the address or id is invalid.
 */
T_ENUM_FTLE_STATUS_TYPE ftle_delete_addr(U32 logical_addr, U32 number, U32 id);

/**
 @brief Deletes any number of bytes in a partition.

 This function will delete any number of bytes in a partition, given the first sector that is to be deleted, and the number of bytes to delete.\n
 It is possible to use the input byte_nr to delete from an offset in the first sector that is to be deleted, eg. erasing the last 200 bytes in a
 sector by setting byte_nr to 312

 @param sector_nr [in] First sector number that have to be erased.
 @param byte_nr [in]   Denotes the first byte in the first sector that is to be deleted.
 @param number [in]    Number of bytes to delete
 @param id [in]        Id for the partition where data is to be deleted

 @return ENUM_FTLE_OK                 If data is deleted.
 @return ENUM_FTLE_SYSTEM_ERROR       If ftle have not been initialized or partition has not been started.
 @return ENUM_FTLE_SYSTEM_ERROR       If ftle did not succeed to delete the data.
 @return ENUM_FTLE_DATA_INVALID       If the data to partial delete does not exists.
 @return ENUM_FTLE_RO                 If data cannot be deleted because the partition given is a read only partition.
 @return ENUM_FTLE_INVALID_PARAMETERS If the address or id is invalid.
 */
T_ENUM_FTLE_STATUS_TYPE ftle_delete_pageaddr(U32 sector_nr, U32 byte_nr, U32 number, U32 id);

/**
 @brief Retrives the size of a given partition.

 This function will return the partition size of the partition given as input, in bytes.

 @param p_bytes [out] Pointer to a variable (U32) where the size of the partition is to be put.
 @param id [in]     Id for the partition where the data is to be written.

 @return ENUM_FTLE_OK                 Partition size found and returned.
 @return ENUM_FTLE_SYSTEM_ERROR       If ftle is not initialized.
 @return ENUM_FTLE_INVALID_PARAMETERS If the partition does not exists.
 */
T_ENUM_FTLE_STATUS_TYPE ftle_get_partition_size(U32 * p_bytes, U32 id);

/**
 @brief Interface for reading out the partition table.

 This function is used to read out the partition table (1536 bytes).

 @param p_data [out] Pointer to memory where to put the partition table.

 @return ENUM_FTLE_OK           Data found and returned.
 @return ENUM_FTLE_SYSTEM_ERROR An error occured when trying to retrieve data.
 */
T_ENUM_FTLE_STATUS_TYPE ftle_get_partition_info(U32 * p_data);

/**
  @}
*/

#endif //FTLE_H

#endif //FTLE_PRESENT
