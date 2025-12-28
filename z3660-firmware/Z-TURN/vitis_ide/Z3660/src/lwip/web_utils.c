/*
 * web_utils.c
 *
 *  Created on: 21 Jan 2024
 *      Author: Efe Tunca
 */

#include "web_utils.h"

#include <string.h>
#include <stdio.h>

#include "xil_printf.h"

char *httpHeader =
   "<HTML>\n\
   \t<HEAD>\n\
   \t\t<TITLE>TFTP Server</TITLE>\n\
   \t</HEAD>\n\
   \t<BODY BGCOLOR=\"#110022\" TEXT=\"#FFBBAA\" STYLE=\"font-size: 16px; line-height: 1.5;\">\n\
   \t\t<p style=\"font-size: 24px;\"><b>TFTP Serve File Tree</b></p>\n\
   \t\t<ul>\n";

/*****************************************************************************/
/**
*
* This function prints the items of a given directory.
*
* @param path is a pointer to the path which wanted to be printed.
*
* @return   None.
*
* @note     The path should be given without an '/' at the end (e.g '0:').
*
******************************************************************************/
void listDirectory(const char *path)
{
   FRESULT res;
   DIR dir;
   FILINFO info;
   DIRSTACK stack[MAX_FOLDER_LEVEL];

   /*
    * To be able to print nested folder structures,
    * the root path should be saved into an array and then
    * subfolder paths should be given by concatenating.
    */
   char currentPath[MAX_PATH_LENGTH];
   int stackIndex = 0;
   int nFolder = 0, nFile = 0;
   strncpy(currentPath, path, MAX_PATH_LENGTH-1);

   xil_printf(".\r\n");

   res = f_opendir(&dir, path);
   if (res == FR_OK) {
      /*
       * The stackIndex variable holds the level of the folder.
       * It's dir object is determined with f_opendir function and
       * it's initialized with level 0.
       */
      stack[stackIndex].dir = dir;
      stack[stackIndex].level = 0;

      while (stackIndex >= 0) {
         DIR *currentDir  = &stack[stackIndex].dir;
         int currentLevel = stack[stackIndex].level;

         /*
          * In order for the f_readdir function to get the file names
          * without abbreviation, the 'use_lfn' value must be changed
          * in the BSP settings.
          */
         res = f_readdir(currentDir, &info);

         /*
          * Checking if the directory item is read successfully and
          * file name is not starting with 0.
          *
          * Here, 0 is not an character, it is an integer.
          * No item name starts with an integer 0.
          */
         if (res == FR_OK && info.fname[0] != 0) {

            /* Checking if the read item is a directory or not. */
            if (info.fattrib & AM_DIR) {
               if (strcmp(info.fname, ".") && strcmp(info.fname, "..")) {

                  /* If the item is a subfolder, then print space(s) for indent. */
                  for (int i = 0; i < currentLevel; i++)
                     xil_printf("|  ");
                  /*
                   * When indentation is done, print folder name
                   * with '|__' at the beginning.
                   */
                  xil_printf("|__%s\r\n", info.fname);

                  /*
                   * To be able to check the contents of the folder,
                   * it is required to concatenate the folder name to root path,
                   * in order to open the directory.
                   */
                  strncat(currentPath, "/", 2);
                  strncat(currentPath, info.fname, MAX_PATH_LENGTH-1);

                  /*
                   * The concatenated directory is opened and
                   * stackIndex number is incremented to indicate that
                   * this is a different directory.
                   *
                   * Then the opened directory is added to the stack.
                   */
                  res = f_opendir(&dir, currentPath);
                  if (res == FR_OK) {
                     stackIndex++;
                     stack[stackIndex].dir = dir;
                     stack[stackIndex].level = currentLevel + 1;
                  }

                  /*
                   * Incrementing nFolder variable only when currentLevel is 0
                   * to count folders located in the root directory.
                   */
                  if (!currentLevel)
                     nFolder++;
               }
            }
            /* If the read item is not a directory, then it is a file. */
            else {
               for (int i = 0; i < currentLevel; i++)
                  xil_printf("|  ");
               xil_printf("|__%s\r\n", info.fname);

               if (!currentLevel)
                  nFile++;
            }
         }
         /*
          * If there is no files or folders, then close the directory
          * and decrement the stackIndex to go back to the previous directory.
          */
         else {
            f_closedir(currentDir);
            stackIndex--;

            /*
             * Removing the last component from the path
             * to go back to the parent directory.
             */
            char *previousDir = strrchr(currentPath, '/');
            if (previousDir)
               *previousDir = '\0';
         }
      }
   }
   xil_printf("\r\n%d folder(s) and %d file(s) in the root folder.\r\n\n", nFolder, nFile);
   f_closedir(&dir);
}

/*****************************************************************************/
/**
*
* This function converts the given file size
* to Bytes, Kilobytes or Megabytes and saves them into a char array.
*
* @param convertedFileSize is a pointer to the char array that
*        the file size information will be saved into.
* @param info is a pointer to the file information structure.
*
* @return   None.
*
* @note     This function is not used currently.
*        However, it could be implemented into the project in the future.
*
******************************************************************************/
void convertFileSize(char *convertedFileSize, FILINFO *info)
{
   int buf;

   if (info->fsize < 1024)
      snprintf(convertedFileSize, 128, "%llu B", info->fsize);
   else if (info->fsize < 1048576) {
      buf = info->fsize / 1024;
      snprintf(convertedFileSize, 128, "%u KB", buf);
   }
   else {
      buf = info->fsize / 1048576;
      snprintf(convertedFileSize, 128, "%u MB", buf);
   }
}

/*****************************************************************************/
/**
*
* This function sets the time attribute of a file
*
* @param fname is a pointer to the file name
*        whose time attribute is to be changed
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void setTimestamp(char *fname)
{
#if 0
   FILINFO info;
   RTC_INFO rtc;

   /*
    * To avoid errors, I2CInit function should be called
    * each time before calling the GetCurrentTime function.
    */
   I2CInit(I2C_DEVICE_ID);
   GetCurrentTime(&rtc);

   info.fdate = (WORD)(((rtc.year + 20) << 9) | (rtc.month << 5) | rtc.day);
   info.ftime = (WORD)((rtc.hour << 11) | (rtc.minute << 5) | (rtc.second / 2U));

   /*
    * In order to be able to use the f_utime function,
    * the 'use_chmod' value must be set to 'true' in the BSP settings.
    */
   f_utime(fname, &info);
#else
   (void)fname;
#endif
}

/*****************************************************************************/
/**
*
* This function creates an index.html file and writes the contents of the
* given directory in these file as nested lists.
*
* @param path is a pointer to the path which wanted to be written to the file.
*
* @return   None.
*
* @note     The path should be given without an '/' at the end (e.g '0:').
*
******************************************************************************/
void createIndexFileTree(const char *path)
{
   FRESULT res;
   DIR dir;
   FILINFO info;
   DIRSTACK stack[MAX_FOLDER_LEVEL];
   FIL file;
   UINT numBytesWritten;
   char itemCountInfo[256];

   res = f_open(&file, "index.html", FA_CREATE_ALWAYS | FA_WRITE);
   res = f_write(&file, httpHeader, strlen(httpHeader), &numBytesWritten);

   /*
    * To be able to write nested folder structures,
    * the root path should be saved into an array and then
    * subfolder paths should be given by concatenating.
    */
   char currentPath[MAX_PATH_LENGTH];
   int stackIndex = 0;
   int nFolder = 0, nFile = 0;

   strncpy(currentPath, path, MAX_PATH_LENGTH-1);

   res = f_opendir(&dir, path);
   if (res == FR_OK) {
      /*
       * The stackIndex variable holds the level of the folder.
       * It's dir object is determined with f_opendir function and
       * it's initialized with level 0.
       */
      stack[stackIndex].dir = dir;
      stack[stackIndex].level = 0;

      while (stackIndex >= 0) {
         DIR *currentDir  = &stack[stackIndex].dir;
         int currentLevel = stack[stackIndex].level;

         /*
          * In order for the f_readdir function to get the file names
          * without abbreviation, the 'use_lfn' value must be changed
          * in the BSP settings.
          */
         res = f_readdir(currentDir, &info);

         /*
          * Checking if the directory item is read successfully and
          * file name is not starting with 0.
          *
          * Here, 0 is not an character, it is an integer.
          * No item name starts with an integer 0.
          */
         if (res == FR_OK && info.fname[0] != 0) {
            f_write(&file, "\t\t\t", strlen("\t\t\t"), &numBytesWritten);

            /* Checking if the read item is a directory or not. */
            if (info.fattrib & AM_DIR) {
               if (strcmp(info.fname, ".") && strcmp(info.fname, "..")) {
                  /*
                   * If the item is a subfolder, then write tab character(s)
                   * for the indentation of the nested list.
                   *
                   * This tab characters has no effect on the appearance
                   * of the index page, just to beautify the HTML file.
                   */
                  for (int i = 0; i < currentLevel; i++)
                     f_write(&file, "\t", strlen("\t"), &numBytesWritten);
                  /*
                   * When indentation is done, write folder name as list item,
                   * in a bold-italic style.
                   */
                  sprintf(stack[stackIndex].name, "<li><b><i>%s</i></b></li>\n", info.fname);
                  f_write(&file, stack[stackIndex].name, strlen(stack[stackIndex].name), &numBytesWritten);

                  /*
                   * To be able to check the contents of the folder,
                   * it is required to concatenate the folder name to root path,
                   * in order to open the directory.
                   */
                  strncat(currentPath, "/", 2);
                  strncat(currentPath, info.fname, MAX_PATH_LENGTH - 1);

                  /*
                   * The concatenated directory is opened and
                   * stackIndex number is incremented to indicate that
                   * this is a different directory.
                   *
                   * Then the opened directory is added to the stack.
                   */
                  res = f_opendir(&dir, currentPath);
                  if (res == FR_OK) {
                     stackIndex++;
                     stack[stackIndex].dir = dir;
                     stack[stackIndex].level = currentLevel + 1;

                     /*
                      * Again, tab characters for just beautifying the HTML file.
                      */
                     f_write(&file, "\t\t\t", strlen("\t\t\t"), &numBytesWritten);
                     for (int i = 0; i < currentLevel; i++)
                        f_write(&file, "\t", strlen("\t"), &numBytesWritten);
                     /*
                      * If there is a subfolder, then create a new list
                      * under the created list to nest the subfolder.
                      */
                     f_write(&file, "<ul>\n", strlen("<ul>\n"), &numBytesWritten);
                  }

                  /*
                   * Incrementing nFolder variable only when currentLevel is 0
                   * to count folders located in the root directory.
                   */
                  if (!currentLevel)
                     nFolder++;
               }
            }
            /* If the read item is not a directory, then it is a file. */
            else {
               for (int i = 0; i < currentLevel; i++)
                  f_write(&file, "\t", strlen("\t"), &numBytesWritten);

               sprintf(stack[stackIndex].name, "<li>%s</li>\n", info.fname);
               f_write(&file, stack[stackIndex].name, strlen(stack[stackIndex].name), &numBytesWritten);

               if (!currentLevel)
                  nFile++;
            }
         }
         /*
          * If there is no files or folders, then close the directory
          * and decrement the stackIndex to go back to the previous directory.
          */
         else {
            f_closedir(currentDir);
            stackIndex--;
            /*
             * Removing the last component from the path
             * to go back to the parent directory.
             */
            char *previousDir = strrchr(currentPath, '/');
            if (previousDir)
               *previousDir = '\0';
            /*
             * Closing the sublist of the subfolder.
             */
            f_write(&file, "\t\t\t</ul>\n", strlen("\t\t\t</ul>\n"), &numBytesWritten);
         }
      }
   }

   sprintf(itemCountInfo, "\t\t<p><i>%d folder(s) and %d file(s) in the root folder.</i></p>\n", nFolder, nFile);
   /*
    * Closing the previously opened tags.
    */
   f_write(&file, "\t\t</ul>\n", strlen("\t\t</ul>\n"), &numBytesWritten);
   f_write(&file, itemCountInfo, strlen(itemCountInfo), &numBytesWritten);
   f_write(&file, "\t</BODY>\n", strlen("\t</BODY>\n"), &numBytesWritten);
   f_write(&file, "</HTML>", strlen("</HTML>"), &numBytesWritten);

   /*
    * The previously opened file(s) and folder(s) must be closed.
    */
   f_close(&file);
   f_closedir(&dir);
}
#if 0
/*****************************************************************************/
/**
*
* This function checks if there is files named 'BOOT.BIN'
* and 'BOOT_old.BIN' in the directory.
*
* If there is a file named 'BOOT_old.BIN', this function deletes it.
*
* If there is a file named 'BOOT.BIN', this function renames it as 'BOOT_old.BIN'
* and sets its time attribute with setTimestamp function.
*
* @param None.
*
* @return   None.
*
* @note     The firmware directory must be selected before calling this function.
*
******************************************************************************/
void checkBootFile(void)
{
   FRESULT res;

   /* Checking if there is a file named 'BOOT_old.BIN' */
   res = f_stat(BOOT_FILE_NAME_OLD, NULL);
   if (res == FR_OK)
      /* Delete the file if there is */
      f_unlink(BOOT_FILE_NAME_OLD);

   /* Checking if there is a file named 'BOOT.BIN' */
   res = f_stat(BOOT_FILE_NAME, NULL);
   if (res == FR_OK) {
      /* Rename the file if there is and set its time attribute */
      f_rename(BOOT_FILE_NAME, BOOT_FILE_NAME_OLD);
      setTimestamp(BOOT_FILE_NAME_OLD);
   }

   /* Renaming the 'temp_BOOT.BIN' file */
   res = f_stat(BOOT_FILE_NAME_TEMP, NULL);
   if (res == FR_OK) {
      f_rename(BOOT_FILE_NAME_TEMP, BOOT_FILE_NAME);
      setTimestamp(BOOT_FILE_NAME);
   }
}
#endif
