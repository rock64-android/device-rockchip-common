/*
 * Copyright (C) 2015 Intel Mobile Communications GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Notes - Intel Mobile Communications:
 * Variant specific updater binary functions to include inside an OTA update
 * Most logic in init_sys_paths() copied from services/jni/com_android_server_BatteryService.cpp
 */


#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <sys/statfs.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <sys/utsname.h>

#include "edify/expr.h"
#include "mincrypt/sha.h"
#include "mkbootimg/bootimg.h"
#include "bootloader.h"
#include "updater/install.h"
#include <cutils/android_reboot.h> 

/***************************************************************************
 * 
 ***************************************************************************/

#define DEFAULT_BLOCK_SIZE    512 // 1meg
#define RK_BCB_OFFSET			32

#define OTA_OK                              0
#define OTA_FAIL                            1
#define BCB_DEVICE 							"/dev/block/platform/soc0/e0000000.noc/by-name/ImcPartID074"
#define PRINT_ERROR_LOCATION printf("Error %s:%s():%d\n", __FILE__, __FUNCTION__, __LINE__)

typedef unsigned long long u64; 
static int read_bootloader_message(struct bootloader_message *out);
static int write_bootloader_message(struct bootloader_message *in);
static int read_bootloader_message(struct bootloader_message *out)
{
	FILE* f=fopen(BCB_DEVICE, "rb");
	if(f == NULL){
		printf("Can't open BCB device\n");
		return -1;
	}
	struct bootloader_message temp;

	/*RK offset*/
	fseek(f,(RK_BCB_OFFSET*DEFAULT_BLOCK_SIZE),SEEK_SET); 
	
	int count = fread(&temp, sizeof(temp), 1, f);
	if (count != 1) {
		printf("Failed reading the BCB device\n");
		PRINT_ERROR_LOCATION;
        return -1;
    }
    if (fclose(f) != 0) {
        printf("Failed closing he BCB device\n");
		PRINT_ERROR_LOCATION;
        return -1;
    }
    memcpy(out, &temp, sizeof(temp));
    return 0;
}

static int write_bootloader_message(struct bootloader_message *in)
{
	FILE* f=fopen(BCB_DEVICE, "wb");
	if(f == NULL){
		printf("Can't open BCB device\n");
		return -1;
	}
	struct bootloader_message temp;

	/*RK offset*/
	fseek(f,(RK_BCB_OFFSET*DEFAULT_BLOCK_SIZE),SEEK_SET); 
	
	int count = fwrite(in, sizeof(*in), 1, f);
	if (count != 1) {
		printf("Failed writing the BCB device\n");
		PRINT_ERROR_LOCATION;
        return -1;
    }
    if (fclose(f) != 0) {
        printf("Failed closing he BCB device\n");
		PRINT_ERROR_LOCATION;
        return -1;
    }
    return 0;
}
static int PackageExtractFile(char* name, char* path)
{
	FILE *fd_src, *fd_dest;
	char* buf[DEFAULT_BLOCK_SIZE];
	int rc = 0;
	if(NULL == (fd_src = fopen(name, "rb")))
	{
		PRINT_ERROR_LOCATION;
		return -1;
	}
	if(NULL == (fd_dest = fopen(path, "wb")))
	{
		PRINT_ERROR_LOCATION;
		fclose(fd_src);
		return -1;
	}
	memset(buf, 0, DEFAULT_BLOCK_SIZE);
	while((rc = fread(buf, BLOCK_SIZE, 1 , fd_src)) != 0)
	{
		fwrite(buf, rc, 1, fd_dest);
		memset(buf, 0 , rc);
	}
	fclose(fd_dest);
	fclose(fd_src);
	return 1;
}

Value* maybe_install_firmware_update(const char* name, State* state, int argc, Expr* argv[]) 
{
	char* fwu_part;
    char* image_name;
	struct bootloader_message boot;
	if(ReadArgs(state, argv, 2, &image_name, &fwu_part) < 0)
    {
        return NULL;
    }
	memset(&boot, 0, sizeof(boot));
    read_bootloader_message(&boot);
	strlcpy(boot.command, "update-radio/hboot", sizeof(boot.command));
	write_bootloader_message(&boot);
	sync();  // For good measure.
	//android_reboot(ANDROID_RB_RESTART, 0, 0);
	return (Value*) StringValue(strdup(""));
}


void Register_librecovery_sofia3gr_intel()
{
    fprintf(stderr, "Registering device extensions\n");
	RegisterFunction("intel_install_firmware_update", maybe_install_firmware_update);
}
