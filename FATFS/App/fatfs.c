/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file   fatfs.c
  * @brief  Code for fatfs applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
#include "fatfs.h"
#include <stdio.h>

uint8_t retUSER;    /* Return value for USER */
char USERPath[4] = "0:"; /* USER logical drive path */
FATFS USERFatFS;    /* File system object for USER logical drive */
FIL USERFile;       /* File object for USER */

/* Additional drive paths */
uint8_t retUSER1;    /* Return value for USER1 */
char USER1Path[4] = "1:"; /* USER1 logical drive path */
FATFS USER1FatFS;    /* File system object for USER1 logical drive */
FIL USER1File;       /* File object for USER1 */

uint8_t retUSER2;    /* Return value for USER2 */
char USER2Path[4] = "2:"; /* USER2 logical drive path */
FATFS USER2FatFS;    /* File system object for USER2 logical drive */
FIL USER2File;       /* File object for USER2 */

/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

void MX_FATFS_Init(void)
{
  /*## FatFS: Link the USER driver ###########################*/
  retUSER = FATFS_LinkDriver(&USER_Driver, USERPath);

  /* USER CODE BEGIN Init */
  /* Link additional drivers for multi-path support */
  retUSER1 = FATFS_LinkDriver(&USER_Driver, USER1Path);
  retUSER2 = FATFS_LinkDriver(&USER_Driver, USER2Path);

  printf("FatFS drivers linked:\r\n");
  printf("- Drive %s: %s\r\n", USERPath, retUSER == 0 ? "OK" : "FAILED");
  printf("- Drive %s: %s\r\n", USER1Path, retUSER1 == 0 ? "OK" : "FAILED");
  printf("- Drive %s: %s\r\n", USER2Path, retUSER2 == 0 ? "OK" : "FAILED");
  /* USER CODE END Init */
}

/**
  * @brief  Gets Time from RTC
  * @param  None
  * @retval Time in DWORD
  */
DWORD get_fattime(void)
{
  /* USER CODE BEGIN get_fattime */
  return 0;
  /* USER CODE END get_fattime */
}

/* USER CODE BEGIN Application */

/* USER CODE END Application */
